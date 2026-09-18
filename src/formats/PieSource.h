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
#ifndef PIESOURCE_HPP
#define PIESOURCE_HPP

#include <istream>
#include <map>
#include <string>
#include <vector>

#define PIE_MODEL_COMMENT_CHAR '#'

/// Line ending used when writing a model that did not come from a file.
#define PIE_MODEL_DEF_LINE_ENDING PieLineEnding::LF

enum class PieLineEnding
{
	LF,
	CRLF
};

const char* getPieLineEndingChars(PieLineEnding ending);

std::string applyPieLineEnding(const std::string& text, PieLineEnding ending);

/**
  * Names a line by its directive and by how many lines with that directive came
  * before it, ex. "TEXTURE#1" or "LEVEL#2". Data lines have no name, since
  * nothing about them survives being loaded.
  *
  * @param counts running tally of the directives seen so far, updated in place
  */
std::string pieAnchorName(const std::string& line, std::map<std::string, unsigned>& counts);

enum class PieCommentPlacement
{
	Leading,	///< On its own line, above the line it belongs to
	Trailing,	///< At the end of the line it belongs to
	InsideBlock,	///< Was among the data lines of a block
	EndOfFile
};

struct PieComment
{
	std::string anchor;
	PieCommentPlacement placement;
	std::string indent;
	std::string text;
};

/**
  * The comments of a model file, each tied to a directive line rather than to a
  * line number, so that they can be put back after the model has been written
  * out afresh.
  *
  * A comment among the data lines of a block is tied to the line that opens the
  * block, because the vertices it sat between do not survive being loaded.
  */
class PieCommentLedger
{
public:
	void add(const PieComment& comment) {m_comments.push_back(comment);}
	void clear() {m_comments.clear();}
	bool empty() const {return m_comments.empty();}
	size_t size() const {return m_comments.size();}

	/**
	  * Puts the comments back into a freshly written model file.
	  *
	  * @param dropped receives the number of comments whose directive is no
	  *	longer in the file, ex. after saving a PIE 4 model as PIE 3
	  */
	std::string apply(const std::string& text, size_t* dropped = nullptr) const;

private:
	std::vector<PieComment> m_comments;
};

/**
  * A PIE file with its comments removed, ready for the model readers.
  *
  * The readers extract whitespace separated tokens and rewind the stream to
  * back out of optional directives, so they cannot see comments themselves.
  * Comments are stripped up front instead, using the same rules as the game:
  * - a '#' that is the first non-whitespace character discards the whole line
  * - a '#' after other content discards the rest of the line
  *
  * Discarded lines are kept as empty lines so that line numbers still match
  * the original file.
  */
class PieSource
{
public:
	PieSource(): m_lineEnding(PIE_MODEL_DEF_LINE_ENDING) {}

	/// False if nothing could be read.
	bool load(std::istream& in);

	const std::string& text() const {return m_text;}
	PieLineEnding lineEnding() const {return m_lineEnding;}
	const PieCommentLedger& comments() const {return m_comments;}

private:
	std::string m_text;
	PieLineEnding m_lineEnding;
	PieCommentLedger m_comments;
};

#endif // PIESOURCE_HPP
