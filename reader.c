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

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "mpc123.h"

/*
 * select which mpc_reader to use, based on filename
 * currently it knows about:
 * - plain file
 */
int reader_choose_and_prepare(const char * fname, mpc_reader ** ret,
                              reader_data * data){
  /* default to plain file reader */
  debugf("choosing plain file reader for \"%s\"", fname);
  data->fd=open(fname, O_RDONLY);
  if(data->fd == -1){
    dief("Couldn't open file \"%s\": [%d] %s\n",
         fname, errno, (errno == 0 ? "Unknown reason" : strerror(errno) ));
  }
  *ret=&mpc123_file_reader;
  (*ret)->data=data;
  return 0;
}

/*
 * cleans up things needed for each reader 
 * knows about:
 * - plain files
 */
void do_cleanup_stream(mpc_reader * the_reader, reader_data * the_data){
  /*
   * plain file reader,
   * nothing special
   */
  if(the_reader == &mpc123_file_reader){
    close(the_data->fd);
    the_data->fd = -1;
  }
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
