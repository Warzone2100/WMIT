/*
	Copyright 2026 Warzone 2100 Project

	This file is part of WMIT.

	WMIT is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	WMIT is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with WMIT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PieSource.h"

#include <set>
#include <sstream>

const char* getPieLineEndingChars(PieLineEnding ending)
{
	switch (ending)
	{
	case PieLineEnding::CRLF: return "\r\n";
	default: return "\n";
	}
}

std::string applyPieLineEnding(const std::string& text, PieLineEnding ending)
{
	if (ending == PieLineEnding::LF)
	{
		return text;
	}

	const std::string newline(getPieLineEndingChars(ending));
	std::string out;

	out.reserve(text.size() + text.size() / 16);
	for (std::string::size_type i = 0; i < text.size(); ++i)
	{
		if (text[i] == '\n')
			out += newline;
		else
			out += text[i];
	}

	return out;
}

static bool isPieLineEnd(char c)
{
	return c == '\n' || c == '\r';
}

static bool isPieSpace(char c)
{
	return c == ' ' || c == '\t';
}

std::string pieAnchorName(const std::string& line, std::map<std::string, unsigned>& counts)
{
	static const std::set<std::string> directives = {
		"PIE", "TYPE", "INTERPOLATE",
		"TEXTURE", "TCMASK", "NORMALMAP", "SPECULARMAP",
		"EVENT", "LEVELS", "LEVEL",
		"MATERIALS", "SHADERS",
		"POINTS", "NORMALS", "POLYGONS", "CONNECTORS", "ANIMOBJECT",
		"SHADOWPOINTS", "SHADOWPOLYGONS"
	};

	std::istringstream ss(line);
	std::string token;

	if (!(ss >> token) || directives.find(token) == directives.end())
	{
		return std::string();
	}

	return token + "#" + std::to_string(++counts[token]);
}

static void splitLineComment(const std::string& line, std::string& content,
			     std::string& indent, std::string& comment)
{
	content.clear();
	indent.clear();
	comment.clear();

	std::string::size_type hash = std::string::npos;
	bool foundContent = false;

	for (std::string::size_type i = 0; i < line.size(); ++i)
	{
		if (line[i] == PIE_MODEL_COMMENT_CHAR)
		{
			hash = i;
			break;
		}

		if (!isPieSpace(line[i]))
			foundContent = true;
	}

	if (hash == std::string::npos)
	{
		content = line;
		return;
	}

	comment = line.substr(hash);

	// Leading whitespace does not count as content, so a '#' after it still
	// comments out the whole line.
	if (!foundContent)
	{
		indent = line.substr(0, hash);
		return;
	}

	std::string::size_type cut = hash;
	while (cut > 0 && isPieSpace(line[cut - 1]))
		--cut;

	content = line.substr(0, cut);
	indent = line.substr(cut, hash - cut);
}

bool PieSource::load(std::istream& in)
{
	std::ostringstream raw;

	raw << in.rdbuf();
	if (in.bad())
		return false;

	const std::string contents = raw.str();
	if (contents.empty())
		return false;

	m_lineEnding = contents.find("\r\n") != std::string::npos ?
				PieLineEnding::CRLF : PieLineEnding::LF;

	m_text.clear();
	m_text.reserve(contents.size());
	m_comments.clear();

	std::map<std::string, unsigned> counts;
	std::vector<PieComment> pending;
	std::string lastBlock;

	std::string::size_type pos = 0;
	while (pos <= contents.size())
	{
		std::string::size_type end = pos;
		while (end < contents.size() && !isPieLineEnd(contents[end]))
			++end;

		std::string content, indent, comment;
		splitLineComment(contents.substr(pos, end - pos), content, indent, comment);

		if (!content.empty())
		{
			const std::string anchor = pieAnchorName(content, counts);

			if (!anchor.empty())
			{
				lastBlock = anchor;
			}

			// A comment above a data line belongs to the block it sat in.
			const std::string target = anchor.empty() ? lastBlock : anchor;
			const PieCommentPlacement placement = anchor.empty() ?
					PieCommentPlacement::InsideBlock : PieCommentPlacement::Leading;

			for (PieComment& held : pending)
			{
				held.anchor = target;
				held.placement = placement;
				m_comments.add(held);
			}
			pending.clear();

			if (!comment.empty() && !target.empty())
			{
				m_comments.add({target, PieCommentPlacement::Trailing, indent, comment});
			}
		}
		else if (!comment.empty())
		{
			pending.push_back({std::string(), PieCommentPlacement::Leading, indent, comment});
		}

		m_text += content;
		m_text += '\n';

		if (end >= contents.size())
			break;

		if (contents[end] == '\r' && end + 1 < contents.size() && contents[end + 1] == '\n')
			++end;
		pos = end + 1;
	}

	// Whatever is left sat below the last directive of the file.
	for (PieComment& held : pending)
	{
		held.placement = PieCommentPlacement::EndOfFile;
		m_comments.add(held);
	}

	return true;
}

std::string PieCommentLedger::apply(const std::string& text, size_t* dropped) const
{
	if (m_comments.empty())
	{
		if (dropped)
			*dropped = 0;
		return text;
	}

	std::map<std::string, unsigned> counts;
	std::vector<bool> used(m_comments.size(), false);
	std::ostringstream out;
	std::istringstream in(text);
	std::string line;

	while (std::getline(in, line))
	{
		const std::string anchor = pieAnchorName(line, counts);

		if (anchor.empty())
		{
			out << line << '\n';
			continue;
		}

		for (size_t i = 0; i < m_comments.size(); ++i)
		{
			const PieComment& comment = m_comments[i];
			if (comment.anchor != anchor || comment.placement != PieCommentPlacement::Leading)
				continue;
			out << comment.indent << comment.text << '\n';
			used[i] = true;
		}

		out << line;
		for (size_t i = 0; i < m_comments.size(); ++i)
		{
			const PieComment& comment = m_comments[i];
			if (comment.anchor != anchor || comment.placement != PieCommentPlacement::Trailing)
				continue;
			out << comment.indent << comment.text;
			used[i] = true;
		}
		out << '\n';

		for (size_t i = 0; i < m_comments.size(); ++i)
		{
			const PieComment& comment = m_comments[i];
			if (comment.anchor != anchor || comment.placement != PieCommentPlacement::InsideBlock)
				continue;
			out << comment.indent << comment.text << '\n';
			used[i] = true;
		}
	}

	for (size_t i = 0; i < m_comments.size(); ++i)
	{
		if (m_comments[i].placement != PieCommentPlacement::EndOfFile)
			continue;
		out << m_comments[i].indent << m_comments[i].text << '\n';
		used[i] = true;
	}

	if (dropped)
	{
		*dropped = 0;
		for (size_t i = 0; i < used.size(); ++i)
		{
			if (!used[i])
				++*dropped;
		}
	}

	return out.str();
}
