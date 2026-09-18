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

#include <sstream>

const char* getPieLineEndingChars(PieLineEnding ending)
{
	switch (ending)
	{
	case PieLineEnding::CRLF: return "\r\n";
	default: return "\n";
	}
}

static bool isPieLineEnd(char c)
{
	return c == '\n' || c == '\r';
}

static bool isPieSpace(char c)
{
	return c == ' ' || c == '\t';
}

/// Cuts a comment off one line, returning the content that survives.
static std::string stripLineComment(const std::string& line)
{
	bool foundContent = false;

	for (std::string::size_type i = 0; i < line.size(); ++i)
	{
		const char c = line[i];

		if (c == PIE_MODEL_COMMENT_CHAR)
		{
			// Leading whitespace does not count as content, so a '#' after it
			// still comments out the whole line.
			if (!foundContent)
				return std::string();
			break;
		}

		if (!isPieSpace(c))
			foundContent = true;
	}

	if (!foundContent)
		return std::string();

	std::string::size_type cut = line.find(PIE_MODEL_COMMENT_CHAR);
	if (cut == std::string::npos)
		cut = line.size();

	while (cut > 0 && isPieSpace(line[cut - 1]))
		--cut;

	return line.substr(0, cut);
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

	std::string::size_type pos = 0;
	while (pos <= contents.size())
	{
		std::string::size_type end = pos;
		while (end < contents.size() && !isPieLineEnd(contents[end]))
			++end;

		m_text += stripLineComment(contents.substr(pos, end - pos));
		m_text += '\n';

		if (end >= contents.size())
			break;

		if (contents[end] == '\r' && end + 1 < contents.size() && contents[end + 1] == '\n')
			++end;
		pos = end + 1;
	}

	return true;
}
