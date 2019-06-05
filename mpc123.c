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

/* is this portable ? */
#define _GNU_SOURCE

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <locale.h>
#include <libintl.h>

#include "mpc123.h"

#ifdef DEBUG
#  include <mcheck.h>
#endif

#define ARG_YES 1
#define ARG_NO 0
#define ARG_MAYBE 2

/* (hopefully) sane defaults */
opts_t options={
  .shuffle=0,
  .random=0,
  .verbosity=0,
  .foutput=NULL,
  .playlist=NULL,
  .ao_driver=NULL,
  .ao_dev=NULL,
  .volume=1.0
};

static void version(){
  fprintf(stderr, _("mpc123 version %d.%d.%d\n"),
          VERS_MAJOR, VERS_MINOR, VERS_REV);
  fprintf(stderr, "Copyright %s\n", COPYRIGHT);
}

static void usage(const char *name){
  version();
  sayf(0, "\nUsage: %s [options] [file1 [file2 [..]]]\n\n"
          "Options supported:\n"
          "   --verbose or -v          Increase verbosity\n"
          "   --quiet or -q            Quiet mode (no title or boilerplate)\n"
          "   --gain N or -g N         Set gain (audio volume) to N (0-100)\n"
          "   -o dt                    Set output devicetype to dt\n"
          "   --audiodevice N or -a N  Use N for audio-out\n"
          "   --au N or -u N           Use au file N for output\n"
          "   --cdr N or -c N          Use raw file N for output\n"
          "   --wav N or -w N          Use wave file N for output\n"
          "   --list N or -@ N         Use playlist N as list of Musepack files\n"
          "   --random or -Z           Play files randomly until interrupted\n"
          "   --shuffle or -z          Shuffle list of files before playing\n"
          "   --help or -h             Print this help screen\n"
          "   --version or -V          Print mpc123 version string\n",
	name);
}

int main(int argc, char ** argv){
  int c=0;
  int ret=0;
  playlist_t pl;

  /*  mpc_bool_t mpc_ret=FALSE;*/
  struct option myOpt[] = {
    {"verbose", ARG_NO, 0, 'v'},
    {"quiet", ARG_NO, 0, 'q'},
    {"gain", ARG_YES, 0, 'g'},
    {"audiodevice", ARG_YES, 0, 'a'},
    {"au", ARG_YES, 0, 'u'},
    {"cdr", ARG_YES, 0, 'c'},
    {"wav", ARG_YES, 0, 'w'},
    {"list", ARG_YES, 0, '@'},
    {"random", ARG_NO, 0, 'Z'},
    {"shuffle", ARG_NO, 0, 'z'},
    {"help", ARG_NO, 0, 'h'},
    {"version", ARG_NO, 0, 'V'},
    {NULL, 0, 0, '\0'}
  };

  setlocale(LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);

#ifdef DEBUG
  /* see glibc-doc, chapter "Debugging Memory Allocation"
   * for the use of mcheck.h, mtrace(), MALLOC_TRACE and mtrace(1);
   */
  mtrace();
  debugf("mpc123 compiled with DEBUG=%d enabled", DEBUG);
  if(!getenv("MALLOC_TRACE")){
    debug("WARNING: you have not set a MALLOC_TRACE destination file");
  }else{
    debug("* * malloc_trace activated; to check mpc123 for memory leaks,");
    debugf("* * type the following command: \"mtrace %s %s\"",
           argv[0], getenv("MALLOC_TRACE"));
  }
#endif

  while ((c=getopt_long(argc, argv, _GETOPT_FLAGS, myOpt, NULL)) != -1){
    switch(c){
      case 'z':			/* shuffle the pl once */
        options.shuffle=1;
        break;
      case 'Z':			/* choose a random entry from pl each time */
        options.random=1;
        break;
      case 'v':			/* be logorroical */
        options.verbosity++;
        break;
      case 'q':			/* be asocial */
        options.verbosity=0;
        break;
      case '@':     /* playlist */
        options.playlist=optarg;
        break;
      case 'o':     /* output driver */
        options.ao_driver=optarg;
        break;
      case 'a':     /* libao device to use */
        options.ao_dev=optarg;
        break;
      case 'w':     /* output to .wav */
        options.ao_driver="wav";
        options.foutput=optarg;
        break;
      case 'u':     /* output to .au */
        options.ao_driver="au";
        options.foutput=optarg;
        break;
      case 'c':     /* output to raw pcm */
        options.ao_driver="raw";
        options.foutput=optarg;
      case 'g':     /* set gain */
        options.volume=atof(optarg)/100.0;
        if(options.volume<0.0 || options.volume>1.0){
          dief("Value %s out of range, try values between 0 and 100\n", optarg);
        }
        break;
      case 'V':
        version();
        exit(0);
        break;
      case 'h':			/* need help */
      default:
        usage(argv[0]);
        exit(1);
    }
  }

  /* intialize singals' hendler */
  mpc123_initsigh();

  /* set default ao_driver and ao_device if none were
   * specified on the command line
   */
  if( !options.ao_driver ){
    options.ao_driver="oss";
  }
  if( !options.ao_dev ){
    /* if the driver is set, but not the output device, use
     * the default devices */
    mpc123_choose_default_dev_by_driver();
  }

  /* no playlist? and no files on the command line either? too bad! */
  if( !(argv[optind] || options.playlist) ){
    usage(argv[0]);
    exit(1);
  }

  say(1, COPYRIGHT_NOTICE "\n");

  /* dump options */
  debugf("opts.shuffle=%d\n"
         "  opts.random=%d\n"
         "  opts.verbosity=%d\n"
         "  opts.foutput=\"%s\"\n"
         "  opts.playlist=\"%s\"\n"
         "  opts.ao_driver=\"%s\"\n"
         "  opts.ao_dev=\"%s\"\n"
         "  opts.volume=%f",
         options.shuffle, options.random, options.verbosity,
         options.foutput, options.playlist, options.ao_driver,
         options.ao_dev, options.volume);

  /* optind? :) */
  debugf("argv[optind (%d)]=\"%s\"", optind, argv[optind]);

  /* prepare the playlist */
  if(options.playlist){
    if((ret=populate_playlist_from_file(&pl, options.playlist)))
      dief("Error populating playlist from file \"%s\": error %d\n",
           options.playlist, ret);
  }else{
    if((ret=populate_playlist_from_argv(&pl, argv, optind, argc-optind)))
      dief("Error populating playlist from command line: error %d\n",
           ret);
  }

  do_play_playlist(&pl);
  free_playlist(&pl);
  return 0;
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
