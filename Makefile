#!/usr/bin/make -f

# mpc123 - Musepack Console audio player
# Copyright (C) 2005, 2006 Fernando Vezzosi <fvezzosi at masobit.net>
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

#POS := $(wildcard LOCALES/*/LC_MESSAGES/*.po)
#MOS := $(POS:%.po=%.mo)

# very rudimentary dependency checking
DEPS := $(patsubst %.c, %.o, $(filter-out $(TARGET).c, $(wildcard *.c)))

# exclude gnuarch files, precious files and .dotfiles
TAR_EXCLUDED += \\{arch} .arch-ids $(wildcard ++*) $(wildcard .?*)

$(TARGET): $(DEPS) $(TARGET).c
	@echo Building mpc123 version $(MAJOR).$(MINOR) ...
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $(DEPS) $(TARGET).c

.PHONY: clean tarball

clean:
	$(RM) tags $(DEPS) $(TARGET) $(MOS)

tags: $(wildcard *.[ch])
	$(TAGSPRG) -R --exclude=\{arch} ./

tarball: clean
	cd ../ &&\
	tar zcf ./mpc123-$$(date "+%Y%m%d").tar.gz $(foreach f, ${TAR_EXCLUDED}, --exclude=${f}) $${OLDPWD##*/}

#$(MOS): $(POS)
#	$(foreach po, $(POS), msgfmt $(po) -o $(po:%.po=%.mo);)
