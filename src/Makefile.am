bin_PROGRAMS = test-xdg

lib_LTLIBRARIES = libxdg.la

include_HEADERS = \
	xdg-utils.hxx \
	xdg-desktop-file.hxx \
	xdg-icon-theme.hxx

libxdg_la_SOURCES = \
	xdg-utils.hxx \
	xdg-desktop-file.hxx \
	xdg-desktop-file.cxx \
	xdg-icon-theme.hxx \
	xdg-icon-theme.cxx

test_xdg_LDADD = \
	libxdg.la
test_xdg_SOURCES = \
	main.cxx

