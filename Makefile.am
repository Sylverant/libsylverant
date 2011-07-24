#  
#   This file is part of Sylverant PSO Server.
#  
#   Copyright (C) 2009 Lawrence Sebald
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Affero General Public License version 3
#   as published by the Free Software Foundation.
#  
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Affero General Public License for more details.
#  
#   You should have received a copy of the GNU Affero General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

ACLOCAL_AMFLAGS = -I m4
SUBDIRS = include src

lib_LTLIBRARIES = libsylverant.la
libsylverant_la_SOURCES =
nodist_EXTRA_libsylverant_la_SOURCES = dummy.c

libsylverant_la_LIBADD = src/database/libdatabase.la \
                         src/utils/libutils.la \
                         src/encryption/libencryption.la \
                         src/polarssl/libpolarssl.la

datarootdir = @datarootdir@ 
