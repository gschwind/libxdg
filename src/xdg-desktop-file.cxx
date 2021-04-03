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

#include "xdg-utils.hxx"

#include <regex>
#include <fstream>
#include "xdg-desktop-file.hxx"

extern "C" {
#include <sys/types.h>
#include <dirent.h>
}

namespace xdg {

using namespace std;

ostream & operator<<(ostream & out, desktop_file const & data)
{
       for (auto & g : data) {
               out << "[" << g.first << "]" << endl;
               for (auto & e: g.second) {
                       out << e.first << "=" << e.second.data << endl;
               }
       }
       return out;
}

desktop_file::desktop_file(string const & filename, string const & lang) :
	filename{filename}
{
	ifstream fin(filename);

	static regex const comment("^(#[^\\n]*\\n?)|(\\s+)$");
	static regex const group("^\\[([^[:cntrl:]]+)\\]\\n?$");
	static regex const entry("^([a-zA-Z0-9-]+)(\\[([a-zA-Z]+)(_[a-zA-Z-]+)?(\\.[a-zA-Z-]+)?(@[a-zA-Z]+)?\\])?\\s?=\\s?([^\\n]*)\\n?$");
	static regex const locale("^([a-zA-Z]+)(_[a-zA-Z]+)?(\\.[a-zA-Z]+)?(@[a-zA-Z]+)?$");

	string language  = "";
	string territory = "";
	string codeset   = "";
	string modifier  = "";

	{
		smatch m;
		if (regex_match(lang, m, locale)) {
			language    = m[1];
			territory   = m[2];
			codeset     = m[3];
			modifier    = m[4];
		}
	}

	xdg::group * cur_group = nullptr;
	vector<char> buffer;
	while(not fin.eof()) {
		buffer.push_back(fin.get());
		if (buffer.back() == '\n') {
			// TODO:processe line

			string s{buffer.begin(), buffer.end()};
			smatch m;

			if (regex_match(s, comment)) {
//				cout << "COMMENT:" << s;
			} else

			if (regex_match(s, m, group)) {
//				cout << "GROUP:" << m[1] << endl;
				cur_group = &this->operator [](m[1]);
			} else


			if (regex_match(s, m, entry)) {
//				cout << "ENTRY:" << s;

				if (cur_group == nullptr) {
					// TODO: Error invalid file.
					buffer.clear();
					continue;
				}

				string key = m[1];
				entry_data tmp;
				string tmp_language  = m[3];
				string tmp_territory = m[4];
				string tmp_codeset   = m[5];
				string tmp_modifier  = m[6];

				tmp.data = m[7];

				// If no language is defined, score 0 but mach any LANG.
				if (m[2] == "") {
					tmp.score = 0;
				} else {
					tmp.score = 0;
					if (language != "") {
						if (language == tmp_language) {
							tmp.score += 8;
						} else {
							tmp.score -= 16;
						}
					}

					if (territory != "") {
						if (territory == tmp_territory) {
							tmp.score += 4;
						} else {
							tmp.score -= 16;
						}
					}

					if (modifier != "") {
						if (modifier == tmp_modifier) {
							tmp.score += 2;
						} else {
							tmp.score -= 16;
						}
					}
				}

				if (tmp.score >= 0) {
					auto f = cur_group->find(key);
					if (f == cur_group->end()) {
						(*cur_group)[key] = tmp;
					} else if (f->second.score < tmp.score) {
						f->second = tmp;
					}
				}

			} else {
				cout << "INVALID:" << s;
			}
			buffer.clear();
		}
	}

}


vector<desktop_file> desktop_file::list_all_applications(string const & lang)
{
	static regex const desktop_file_patern{"^.+\\.desktop$"};

	vector<desktop_file> list;
	stack<string> pending_directories;

	char const * XDG_DATA_DIRS = std::getenv("XDG_DATA_DIRS");

	if (XDG_DATA_DIRS) {
		for (auto & p: split(XDG_DATA_DIRS, ':')) {
			pending_directories.push(p+"/applications");
		}
	}

	while(not pending_directories.empty()) {
		string const & curdir = pending_directories.top();

		DIR * dir = opendir(curdir.c_str());
		if (dir == nullptr) {
			pending_directories.pop();
			continue;
		}

		struct dirent * p;
		while((p = readdir(dir)) != nullptr) {
			// skip hidden file and "." and "..", and avoid infinite loop
			// we guess that a filename cannot be empty
			if (p->d_name[0] == '.')
				continue;
			if (p->d_type == DT_DIR or p->d_type == DT_UNKNOWN)
				pending_directories.push(curdir+"/"+p->d_name);
			if (p->d_type == DT_REG or p->d_type == DT_UNKNOWN) {
				if (regex_match(p->d_name, desktop_file_patern)) {
					cout << p->d_name << endl;
					list.emplace_back(desktop_file(curdir+"/"+p->d_name, lang));
				}
			}
		}
		closedir(dir);
		pending_directories.pop();
	}

	return list;
}

} // namespace xdg
