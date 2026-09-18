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
#include <string>

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

private:
	std::string m_text;
	PieLineEnding m_lineEnding;
};

#endif // PIESOURCE_HPP
