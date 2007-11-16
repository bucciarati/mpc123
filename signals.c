/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2006-2007 Daniele Sempione <scrows at oziosi.org>
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
#include <stdio.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <execinfo.h>

#include "mpc123.h"

struct sigaction mpc123_signals;
sigset_t mpc123_sigset;
struct timeval tlast, tnew;

int handled_signals[]={
  SIGSEGV,    /* mem problems */
  SIGFPE,     /* math problems */
  SIGILL,     /* function pointer problems */
  SIGINT,     /* play next song */
  0           /* terminator */
};

int mpc123_setsigh(int currsig) {
sigintr:
	if(sigaction(currsig, &mpc123_signals, NULL) == -1) {
		if(errno == EINTR)
			goto sigintr;
		else
			return 1;
	}
	return 0;
}

void mpc123_initsigh(void) {
	int i;
  gettimeofday(&tlast, NULL);
	mpc123_signals.sa_handler = signal_handler;
	mpc123_signals.sa_flags = 0;
	sigemptyset(&mpc123_signals.sa_mask);

  sigemptyset(&mpc123_sigset);
  sigaddset(&mpc123_sigset, SIGINT);

	for(i=0; handled_signals[i]; i++)
		if(mpc123_setsigh(handled_signals[i]))
			sayf(0, "Can't handle %s. errno [%d]\n", strsignal(handled_signals[i]), errno);
}

/* who needs more backtrace symbols ? */
#ifdef DEBUG
#  define MPC123_BT_SYMBOLS 50
#else
#  define MPC123_BT_SYMBOLS 10
#endif
void signal_handler(int signo){
  void * ary[MPC123_BT_SYMBOLS];
  char ** strings;
  int i=0, size=0;

  switch(signo){
    case SIGINT:
      sigprocmask(SIG_BLOCK, &mpc123_sigset, NULL);
      gettimeofday(&tnew, NULL);
      if(((tnew.tv_sec - tlast.tv_sec) * 1000000 +
          (tnew.tv_usec - tlast.tv_usec)) < 700000)
        mpc123_flag_set(MPC123_FL_STOP);
      else
        mpc123_flag_set(MPC123_FL_PLAYNEXT);
      tlast.tv_sec = tnew.tv_sec;
      tlast.tv_usec = tnew.tv_usec;
      sigprocmask(SIG_UNBLOCK, &mpc123_sigset, NULL);
      break;
    case SIGSEGV:
      say(0, "Segmentation Fault. Backtrace follows.\n"
             "NOTE: if you didn't compile with ``make DEBUG=1'', then\n"
             "  *   this info is probably mostly useless;\n"
             "  *   recompile with debug activated and try to crash\n"
             "  *   mpc123 again :)\n\n");
      size=backtrace(ary, MPC123_BT_SYMBOLS);
      strings=backtrace_symbols(ary, size);
      if(!strings)        /* unlikely */
        return;
      for(i=0; i < size; i++){
        sayf(0, "%2d: %s\n", i, strings[i]);
      }
      free(strings);
      fprintf(stderr, "Aborting\n");
      abort();
      break;            /* useless, anyway */
    /* XXX: case SIGFPE SIGILL ?? */
    default:
      /* .. */
      break;
  }
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
