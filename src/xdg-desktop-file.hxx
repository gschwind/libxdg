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

#ifndef SRC_XDG_DESKTOP_FILE_HXX_
#define SRC_XDG_DESKTOP_FILE_HXX_

#include <unordered_map>
#include <string>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <vector>

namespace xdg {

struct entry_data {
	int32_t score; //< keep language score.
	std::string data;
};

struct group : public std::unordered_map<std::string, entry_data>
{
	template<typename T>
	static T convert(std::string const & s);

	template<typename T>
	T getattr(std::string const & key) const
	{
		auto x = this->find(key);
		if (x == this->end()) {
			throw std::runtime_error("Key not available");
		} else {
			T ret;
			std::istringstream(x->second.data) >> ret;
			return ret;
		}
	}


	template<typename T>
	T getattr(std::string const & key, T const & default_value) const
	{
		auto x = this->find(key);
		if (x == this->end()) {
			return default_value;
		} else {
			T ret;
			std::istringstream(x->second.data) >> ret;
			return ret;
		}
	}
};

struct desktop_file : public std::unordered_map<std::string, group>
{
	std::string filename;


	friend std::ostream & operator<<(std::ostream & out, desktop_file const & file);

	desktop_file(std::string const & filename, std::string const & lang);

};

struct application : public desktop_file {
	std::string id;

	application(std::string const & filename, std::string const & id, std::string const & lang);

	static std::vector<application> list_all_applications(std::string const & lang);

};

} // namespace xdg

#endif /* SRC_XDG_DESKTOP_FILE_HXX_ */
