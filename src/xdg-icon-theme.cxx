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

#include <xdg-desktop-file.hxx>
#include "xdg-icon-theme.hxx"

#include "xdg-utils.hxx"

#include <cmath>
#include <cstdlib>
#include <cstdint>

extern "C" {
#include <unistd.h>
}

namespace xdg {

using namespace std;

struct s_subdir_rule {
	int scale;
	int size;

	union {
		struct {
			int min_size;
			int max_size;
		};
		int threshold;
	};

	struct s_vtable {
		bool (s_subdir_rule::*match) (int, int) const;
		int  (s_subdir_rule::*distance) (int, int) const;
	};

	enum : char {
		TYPE_UNKNOWN   = 0,
		TYPE_THREHOLD  = 1,
		TYPE_FIXED     = 2,
		TYPE_SCALABLE  = 3,
		TYPE_MAX       = 4
	} type;

	static s_vtable const _api[TYPE_MAX];


	bool match_unknown(int size, int scale) const
	{
		return false;
	}

	bool match_fixed(int size, int scale) const
	{
		if (scale != this->scale)
			return false;
		return this->size == size;
	}

	bool match_scaled(int size, int scale) const
	{
		if (scale != this->scale)
			return false;
		return min_size <= size and size <= max_size;
	}

	bool match_threshold(int size, int scale) const
	{
		if (scale != this->scale)
			return false;
		return std::abs(size-this->size) <= threshold;
	}

	int dist_unknown(int size, int scale) const
	{
		return std::numeric_limits<int>::max();
	}

	int dist_fixed(int size, int scale) const
	{
		return std::abs(this->size*this->scale-size*scale);
	}

	int dist_scaled(int size, int scale) const
	{
		if (size*scale < this->min_size*this->scale)
			return this->min_size*this->scale - size*scale;
		if (size*scale > this->max_size*this->scale)
			return size*scale - this->max_size*this->scale;
		return 0;
	}


	int dist_threshold(int size, int scale) const
	{
		if (size*scale < (this->size-this->threshold)*this->scale)
			return (this->size-this->threshold)*this->scale - size*scale;
		if (size*scale > (this->size+this->threshold)*this->scale)
			return size*scale - (this->size+this->threshold)*this->scale;
		return 0;
	}

	s_subdir_rule & operator=(s_subdir_rule const &) = default;
	s_subdir_rule(s_subdir_rule const &) = default;
	s_subdir_rule() {
		type = TYPE_UNKNOWN;
	}

	s_subdir_rule(group const & subdir)
	{
		size  = subdir.getattr<int>("Size");
		scale = subdir.getattr<int>("Scale", 1);

		string stype = subdir.getattr<string>("Type", "Threshold");

		if (stype == "Threshold") {
			threshold = subdir.getattr<int>("Threshold", 2);
			type = TYPE_THREHOLD;
		} else if (stype == "Scalable") {
			min_size = subdir.getattr<int>("MinSize", size);
			max_size = subdir.getattr<int>("MaxSize", size);
			type = TYPE_SCALABLE;
		} else if (stype == "Fixed") {
			type = TYPE_FIXED;
		} else {
			// Unknown type
			cout << "WARNING: unexpected subdir type" << endl;
			type = TYPE_UNKNOWN;
		}
	}

	bool match(int size, int scale) const
	{
		return (this->*_api[type].match)(size, scale);
	}

	int dist(int size, int scale) const
	{
		return (this->*_api[type].distance)(size, scale);
	}


};

s_subdir_rule::s_vtable const s_subdir_rule::_api[TYPE_MAX]  = {
		{&s_subdir_rule::match_unknown, &s_subdir_rule::dist_unknown},
		{&s_subdir_rule::match_threshold, &s_subdir_rule::dist_threshold},
		{&s_subdir_rule::match_fixed, &s_subdir_rule::dist_fixed},
		{&s_subdir_rule::match_scaled, &s_subdir_rule::dist_scaled}
};

struct theme_index {

	friend struct theme;

	struct invalid_theme_index_file : public std::exception {
		virtual char const * what() const noexcept override {
			return "invalid_theme_index_file";
		}
	};

	string identifier;
	vector<theme_index const *> inherits;
	unordered_map<string, s_subdir_rule> subdir_rules;

	bool subdir_match(string const & subdir, int size, int scale) const
	{
		auto rule = subdir_rules.find(subdir);
		if (rule == subdir_rules.end())
			return false;
		return rule->second.match(size, scale);
	}

};


static array<string const, 3> const extensions{{".png", ".svg", ".xpm"}};
static string const undef{"undef"};

theme::~theme()
{

}

theme::theme(string const & identifier) : identifier{identifier}
{

	// Setup theme index searsh directories
	auto HOME = std::getenv("HOME");
	if (HOME) {
		string dir = string{HOME}+"/.icons";
		if (access(dir.c_str(), R_OK) == 0)
			search_directories.push_back(dir);
	}

	auto XDG_DATA_DIRS = std::getenv("XDG_DATA_DIRS");
	if (XDG_DATA_DIRS) {
		for (auto & p: split(XDG_DATA_DIRS, ':')) {
			string dir = p+"/icons";
			if (access(dir.c_str(), R_OK) == 0)
				search_directories.push_back(p+"/icons");
		}
	}

	if (access("/usr/share/pixmap", R_OK) == 0)
		search_directories.push_back("/usr/share/pixmap");

	lookup_list.push_back(_get_theme_index(identifier));
	_build_lookup_list();

	auto _hicolor = _get_theme_index("hicolor");
	if (find(lookup_list.begin(), lookup_list.end(), _hicolor) == lookup_list.end()) {
		lookup_list.push_back(_hicolor);
		_build_lookup_list();
	}

}

// Implement XDG lookup
string theme::_lookup_for_theme_index_file(string const & identifier) const
{
	for (auto & p: search_directories) {
		string f = p + "/" + identifier + "/index.theme";
		cout << "lookup: " << f << endl;
		if (access(f.c_str(), R_OK) == 0) {
			return f;
		}
	}

	return undef;

}

theme_index const * theme::_get_theme_index(string const & identifier)
{
	auto xtheme = theme_index_cache.find(identifier);
	if (xtheme != theme_index_cache.end())
		return xtheme->second.get();

	string filename = _lookup_for_theme_index_file(identifier);
	if (filename == undef)
		return nullptr;

	auto & t = theme_index_cache[identifier] = unique_ptr<theme_index>{new theme_index()};
	t->identifier = identifier;
	cout << "Load " << identifier << endl;

	desktop_file const data{filename, getenv_lang()};

	auto icon_theme = data.find("Icon Theme");
	if (icon_theme == data.end()) {
		cerr << "ERROR: Invalid icon theme" << endl;
		return nullptr;
	}
	auto directories = icon_theme->second.find("Directories");
	if (directories == icon_theme->second.end()) {
		cerr << "ERROR: Missing mandatory Directories entry" << endl;
		return nullptr;
	}
	for (auto const & s: split(directories->second.data, ',')) {
		auto subdir = data.find(s);
		if (subdir == data.end()) {
			cerr << "ERROR: Missing mandatory subdir group `" << s << "'" << endl;
			continue;
		}
		t->subdir_rules[s] = s_subdir_rule{subdir->second};
	}

	auto inherits_iter = icon_theme->second.find("Inherits");
	if (inherits_iter != icon_theme->second.end()) {
		for(auto & parent_theme: split(inherits_iter->second.data, ',')) {
			auto p = _get_theme_index(parent_theme);
			if (p) {
				t->inherits.push_back(p);
			}
		}
	}
	return t.get();
}

string theme::find_icon(string const & name, int size, int scale) const
{
	string ret;
	for (auto theme: lookup_list) {
		ret = _lookup_icon_in_theme(*theme, name, size, scale);
		if (ret != undef) {
			return ret;
		}
	}
	return _lookup_fallback(name);
}

string theme::_lookup_fallback(string const & name) const
{
	for (auto & basedir: search_directories) {
		for (auto & e: extensions) {
			string p = basedir+"/"+name+e;
			if (access(p.c_str(), R_OK) == 0)
				return p;
		}
	}
	return undef;
}

auto theme::_build_lookup_list() -> void
{
	auto theme = lookup_list.back();
	for (auto & p: theme->inherits) {
		bool visited = false;
		// CHeck if we already visited it
		for (auto &v: lookup_list) {
			if (v == p) { visited = true; break; }
		}
		if (visited)
			continue;
		lookup_list.push_back(p);
		_build_lookup_list();
	}
}


// Implement XDG lookup
string theme::_lookup_icon_in_theme(theme_index const & theme, string const & name, int size, int scale) const
{
	// First pass exact match
	for (auto & subdir: theme.subdir_rules) {
		if (subdir.second.match(size, scale)) {
			for (auto & basedir: search_directories) {
				for (auto & ext: extensions) {
					string p = basedir+"/"+theme.identifier+"/"+subdir.first+"/"+name+ext;
//					cout << p << endl;
					if (access(p.c_str(), R_OK) == 0)
						return p;
				}
			}
		}
	}

	// Second pass best match
	int subdir_distance = std::numeric_limits<int>::max();
	string match;
	for (auto & subdir: theme.subdir_rules) {
		if (subdir.second.match(size, scale)) {
			int d = subdir.second.dist(size, scale);
			if (d > subdir_distance)
				continue;
			for (auto & basedir: search_directories) {
				for (auto & ext: extensions) {
					string p = basedir+"/"+theme.identifier+"/"+subdir.first+"/"+name+ext;
					if (access(p.c_str(), R_OK) == 0) {
						subdir_distance = d;
						match = p;
					}
				}
			}
		}
	}

	if (match != undef)
		return match;
	return undef;
}

} // namespace xdg
