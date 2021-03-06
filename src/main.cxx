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
#include <iostream>

#include "xdg-utils.hxx"
#include "xdg-icon-theme.hxx"

using namespace std;

int main(int argc, char ** argv)
{
	for (auto & f: xdg::desktop_file::list_all_applications(xdg::getenv_lang())) {
		cout << f;
	}


	xdg::theme theme("Adwaita");

	cout << theme.find_icon("system-file-manager", 64, 1) << endl;
	cout << theme.find_icon("network-wired", 32, 1) << endl;

	return 0;
}

