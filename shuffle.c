/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005-2008 Daniele Sempione <scrows at oziosi.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "mpc123.h"

int shuffle(char **files, int n, char **argv) {
  char *shuffletable;
  unsigned int i, sl, shuffind;

  srand(time(NULL));
  shuffletable = (char *) calloc(n, sizeof(char));

  for(i=0; i<n; i++) {
    debug("element ");

    shuffind = rand() % n;

    while(shuffletable[shuffind]) {
      shuffind++;
      if(shuffind == 0xfffffffc)  /* against integer overflow */
        shuffind = 0;
      shuffind %= n;
    }
    shuffletable[shuffind] = 1;
    sl = strlen(argv[shuffind]);
    files[i] = (char *) malloc(sl+1);
    strncpy(files[i], argv[shuffind], sl);
    debugf("\"%s\"", files[i]);
  }

  free(shuffletable);
  return i;
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
