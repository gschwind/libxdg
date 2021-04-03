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

#ifndef SRC_XDG_ICON_THEME_HXX_
#define SRC_XDG_ICON_THEME_HXX_

#include <vector>
#include <memory>
#include <array>
#include <unordered_map>
#include <string>

namespace xdg {

struct theme_index;

class theme {
	std::string identifier;
	std::vector<std::string> search_directories;
	std::unordered_map<std::string, std::unique_ptr<theme_index>> theme_index_cache;
	std::vector<theme_index const *> lookup_list;

	auto _get_theme_index(std::string const & identifier) -> theme_index const *;
	auto _lookup_for_theme_index_file(std::string const & identifier) const -> std::string;
	auto _build_lookup_list() -> void;
	auto _lookup_icon_in_theme(theme_index const & theme, std::string const & name, int size, int scale) const -> std::string;
	auto _lookup_fallback(std::string const & name) const -> std::string;

public:
	~theme();
	theme(std::string const & identifier);
	// find request icon within the theme, return undef on fail.
	auto find_icon(std::string const & name, int size, int scale) const -> std::string;

};

} // namespace xdg

#endif /* SRC_XDG_ICON_THEME_HXX_ */
