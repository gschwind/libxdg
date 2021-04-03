/*

Copyright (2021) Benoit Gschwind <gschwind@gnu-log.net>

This file is part of libxdg.

libxdg is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libxdg is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libxdg.  If not, see <https://www.gnu.org/licenses/>.

*/

#ifndef SRC_XDG_UTILS_HXX_
#define SRC_XDG_UTILS_HXX_

#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>

namespace xdg {

inline std::string getenv_lang()
{
	// Extract the default;
	std::string slocale = "";
	char const * lc_messages = getenv("LC_MESSAGES");
	if (lc_messages) {
		slocale = lc_messages;
	} else {
		char const * lc_all = getenv("LC_ALL");
		if (lc_all) {
			slocale = lc_messages;
		} else {
			char const * lang = getenv("LANG");
			if (lang)
				slocale = lang;
		}
	}

	return slocale;
}

inline std::vector<std::string> split(std::string const & in, char c)
{
	std::vector<std::string> ret;
	auto cur = in.begin();
	for (;;) {
		auto next = std::find(cur, in.end(), c);
		ret.emplace_back(cur, next);
		if (next == in.end())
			break;
		cur = next+1;
	}
	return ret;
}



} // namespace xdg

#endif /* SRC_XDG_UTILS_HXX_ */
