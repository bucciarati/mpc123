/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005-2007 Fernando Vezzosi <fvezzosi at masobit.net>
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

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#include "mpc123.h"

/* populate playlist from file */
int populate_playlist_from_file(playlist_t *pl, const char *fname){
  FILE *fd  =  fopen(fname, "r");
  char *strFile;
  size_t strSize = 128;
  int sl,i=0;

  if(!fd)
    dief("Could not open playlist \"%s\": %s\n", fname, strerror(errno));

  /* allocating initial buffer for getline*/
  strFile = (char *)malloc(strSize);
  /* allocating initial playlist */
  pl->files = (char **)malloc(0);

  while(getline(&strFile, &strSize, fd) != -1){  /* get a playlist element */
    sl=strlen(strFile);
    strFile[sl-1] = '\0';  /* remove \n */
    if(sl==1)  /* empty line*/
      continue;

    debugf("Read string: \"%s\"", strFile);

    /* allocate and strcpy each element */
    pl->files = realloc(pl->files, (i+1)*sizeof(char*));
    pl->files[i]=(char*)malloc(sl+1);
    strncpy(pl->files[i], strFile, sl);
    debugf(" \"%s\"", pl->files[i]);
    i++;
  }

  pl->files[i]=NULL;

  /* insert info about playlist length */
  pl->n_elems=i;
  if(options.verbosity)
    printf("Playlist has %d elements\n", pl->n_elems);

  fclose(fd);
  free(strFile);
  return 0;
}

/* populate array from argv[optind], n elements */
int populate_playlist_from_argv(playlist_t * pl, char ** argv,
                                int optind, int n){
  int i=0;
  int sl=0;

  debugf("before black magic: argv[0]=\"%s\", optind=%d, n=%d",
        argv[0], optind, n);
  /* black magic */
  argv=&argv[optind];
  debugf("after black magic: argv[0]=\"%s\", optind=%d, n=%d",
        argv[0], optind, n);

  /* allocate main array (n elements + NULL) */
  pl->files=(char **)malloc(sizeof(char*) * n+1);

  /* scramble playlist */
  if(options.shuffle && !options.random){
    i = shuffle(pl->files, n, argv);
    debug("shuffle done");
  } else {
  /* otherwise just allocate and strcpy each element */
    for(i=0; i<n; i++){
      debugf("element \"%s\" ", argv[i]);
      sl=strlen(argv[i]);
      pl->files[i]=(char*)malloc(sl+1);
      strncpy(pl->files[i], argv[i], sl);
      debugf("        \"%s\"", pl->files[i]);
    }
  }
  pl->files[i]=NULL;

  /* insert info about playlist length */
  pl->n_elems=i;
  sayf(2, "Playlist has %d elements\n", pl->n_elems);
  return 0;
}

/* do the real job:
 * - select next file to play
 * - consider options
 */
int do_play_playlist(playlist_t *pl){
  reader_data data;
  mpc_reader * mpc123_general_reader;
  int ret=0;

  int current=0;	/* we play pl->files[current] */

  if(options.random){
    srand(time(NULL));                  /* in case we want to use rand() */
    current=rand() % pl->n_elems;       /* first item to be played */
  }

  /* main playing loop */
  while(1){
    if(!pl->files[current])
      return 1;
    sayf(1, "Playing Musepack audio stream from \"%s\"...\n",
         pl->files[current]);

    data.length=0;
    data.curr_pos=0;

    if( (reader_choose_and_prepare(pl->files[current],
                                   &mpc123_general_reader, &data)) == 0){
      /* play it ;) */
      ret=do_play_stream(mpc123_general_reader, &data);
      debugf("Stream play returned [%d] %s", ret, ret ? " !!" : "");

      /* clean up (close fds, etc..) */
      do_cleanup_stream(mpc123_general_reader, &data);
    }else{
      sayf(0, "Error choosing method for stream: \"%s\"\n",
           pl->files[current]);
    }

    /* couldn't play the stream; it's likely
     * we won't be able to play the next either
     */
    if(ret)
      break;

    /* no more stuff to play? break; */
    debugf("random=%d, pl->files[%d+1]=\"%s\", willstop=%d",
            options.random, current, pl->files[current+1],
            mpc123_flag_isset(MPC123_FL_STOP));
    if((!options.random && !pl->files[current+1])
       || mpc123_flag_isset(MPC123_FL_STOP) )
      break;

    /* select next playlist element */
    if(options.random){
      current=rand() % pl->n_elems;
    }else{
      current++;
    }
  }
  return 0;
}

/* free playlist mallocated memory  */
int free_playlist(playlist_t *pl){
  int  i;
  for(i=pl->n_elems;i!=0;i--){
    debugf("Freeing Memory: %s", pl->files[i-1]);
	  free(pl->files[i-1]); /* free each single elements  */
  }
  free(pl->files); /* free main array */
  return 0;
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
