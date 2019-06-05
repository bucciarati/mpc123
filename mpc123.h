/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005-2008 Fernando Vezzosi <github-mpc123 at repnz.net>
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

#ifndef _MPC123_H
#  define _MPC123_H 1

#  include <stdio.h>
#  include <string.h>
#  include <wchar.h>
#  include <alloca.h>

#  include <stdint.h>

# include <libintl.h>

#  include <mpc/mpcdec.h>
#  include <mpc/streaminfo.h>
#  include <mpc/mpc_types.h>

#  ifndef LOCALEDIR
#    define LOCALEDIR "/usr/share/locale" /* this is the Debian default */
#  endif

#  define PACKAGE "mpc123"
#  define VERS_NAME	"mpc123"
#  define COPYRIGHT	"(C) 2005-2008 Fernando Vezzosi <github-mpc123 at repnz.net>"
/* don't touch these fields, as they are automatically
 * changed by the Makefile building process
 */
#  define VERS_MAJOR	0
#  define VERS_MINOR	2
#  define VERS_REV	5

#  define COPYRIGHT_NOTICE VERS_NAME " Copyright " COPYRIGHT "\n" \
	"This is free software, meaning it comes under the terms of the\n" \
	"GNU General Public License (GNU GPL) version 2 or above, see\n" \
	"http://www.gnu.org/ and http://mpc123.sourceforge.net/ for \n" \
	"+ info about the lack of any kind of warranty,\n" \
	"+ source code, and\n" \
	"+ license details\n"

#  define _GETOPT_FLAGS "zZvhqV@:o:a:g:w:u:c:"

#  define _(x) gettext(x)
/* #  define _(x) (x) */


/* macros for debugging output.
 * they add the trailing newline.
 */
#  ifdef DEBUG
#    define debug(x) printf("  " x "\n")
#    define debugf(x, ...) printf("  " x "\n", __VA_ARGS__)
#  else
#    define debug(x) do { } while (0)
#    define debugf(x, ...) do { } while (0)
#  endif

/* macros for normal output.
 * use these instead of printf()s
 */
#  define say(n, txt) do { \
  if(options.verbosity >= n){ \
    fprintf(stdout, _(txt)); \
  } \
} while (0)

#  define sayf(n, txt, ...) do { \
  if(options.verbosity >= n){ \
    fprintf(stdout, _(txt), __VA_ARGS__); \
  } \
} while (0)

/* macros for necrophiles. they add the trailing newline.
 * die, die with format
 */
#  define die(x) do { \
  fprintf(stderr, _(x)); \
  return 1; \
} while (0)

#  define dief(x, ...) do { \
  fprintf(stderr, _(x), __VA_ARGS__); \
  return 1; \
} while (0)

typedef struct opts_t {
  int shuffle;		/* produces a randomly-sorted playlist */
  int random;		/* playlist entries are randomly chosen */
  unsigned int verbosity;	/* program verbosity */
  char * foutput; 	/* target file for file output (wav, au...)*/
  char * playlist;      /* list of files to play */
  char * ao_driver;     /* libao driver name eg "oss" "alsa09" "esd" */
  char * ao_dev;        /* libao output device eg "/dev/dsp" "default" */
  float volume;         /* volume multiplier. the output gets multiplied by
                         * this, so it is a float going from 0 (mute volume)
                         * to 1 (100% volume)
                         */
  /* runtime flags */
  uint32_t flags;            /* 32 bit space for flags. see MPC123_FL_* */
} opts_t;

#  define MPC123_FL_PLAYNEXT    (1 << 0)
#  define MPC123_FL_STOP        (1 << 1)

#  define mpc123_flag_set(flag)         do { \
  options.flags |= (flag); \
  debugf("flags now look like 0x%08x", options.flags); \
} while (0)
#  define mpc123_flag_unset(flag)         do { \
  options.flags &= ~(flag); \
  debugf("flags now look like 0x%08x", options.flags); \
} while (0)
#  define mpc123_flag_isset(flag)       ((options.flags & (flag)) ? 1 : 0)

typedef struct playlist {
  char **files;		/* *files[] */
  /* the file list terminates when (pl->files[i] == NULL) */
  int n_elems;
} playlist_t;

/* this is passed between reader functions */
typedef struct reader_data {
  int fd;
  unsigned int length;
  off_t curr_pos;
} reader_data;

/* global data */
mpc_reader mpc123_file_reader;

opts_t options;

/* function declaration */
  /* reader.c */
int reader_choose_and_prepare(const char *, mpc_reader **, reader_data *);
void do_cleanup_stream(mpc_reader *, reader_data *);
  /* player.c */
int do_play_stream(mpc_reader *, reader_data *);
  /* playlist.c */
int populate_playlist_from_file(playlist_t *, const char *);
int populate_playlist_from_argv(playlist_t *, char **, int, int);
int do_play_playlist(playlist_t *);
int free_playlist(playlist_t *);
  /* shuffle.c */
int shuffle(char **, int, char **);
  /* ao.c */
void mpc123_choose_default_dev_by_driver(void);
int mpc123_ao_init(void **, mpc_streaminfo *);
int mpc123_ao_play(void *, MPC_SAMPLE_FORMAT *, unsigned);
int mpc123_ao_done(void *);
  /* signals.c */
void mpc123_initsigh(void);
void signal_handler(int);

#endif

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
