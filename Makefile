#!/usr/bin/make -f

# mpc123 - Musepack Console audio player
# Copyright (C) 2005-2006 Fernando Vezzosi <fvezzosi at linuxvar.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

CC := $(shell which colorgcc || which cc)

TAGSPRG := ctags

CFLAGS += -Wall
LDFLAGS += -lao -lmpcdec

# in case of DEBUG, we shall add these flags
ifneq ($(DEBUG), )
	CPPFLAGS += -DDEBUG=$(DEBUG)
	CFLAGS += -ggdb
	LDFLAGS += -rdynamic
#	LDFLAGS += -lefence
#	LDFLAGS += -lmudflap
else
	CFLAGS += -O2
endif

TARGET := mpc123
MAJOR  := 0
MINOR  := 2

POS := $(wildcard LOCALES/*/LC_MESSAGES/*.po)
MOS := $(POS:%.po=%.mo)

# very rudimentary dependency checking
DEPS := $(patsubst %.c, %.o, $(filter-out $(TARGET).c, $(wildcard *.c)))

$(TARGET): $(DEPS) $(TARGET).c $(MOS)
	@echo Building mpc123 version $(MAJOR).$(MINOR) ...
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(DEPS) $(TARGET).c

.PHONY: clean tarball

clean:
	$(RM) tags $(DEPS) $(TARGET) $(MOS)

tags: $(wildcard *.[ch])
	$(TAGSPRG) -R --exclude=.hg ./

tarball: clean
	hg archive -t tbz2 ../mpc123-$$(date "+%Y%m%d").tar.bz2

$(MOS): $(POS)
	$(foreach po, $(POS), msgfmt $(po) -o $(po:%.po=%.mo);)
