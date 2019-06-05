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

#include <unistd.h>
#include <fcntl.h>

#include "mpc123.h"

/* read wrapper */
static mpc_int32_t mpc123_file_read(mpc_reader *t, void *ptr, mpc_int32_t size){
  reader_data *data=(reader_data*) t->data;
  return read(data->fd, ptr, size);
}

/* seek wrapper */
static mpc_bool_t mpc123_file_seek(mpc_reader *t, mpc_int32_t offset){
  reader_data *data=(reader_data*) t->data;
  lseek(data->fd, offset, SEEK_SET);
  return MPC_TRUE;
}

static mpc_int32_t mpc123_file_tell(mpc_reader *t){
  reader_data *data=(reader_data*) t->data;
  return lseek(data->fd, 0, SEEK_CUR);
}

/* get filesize */
static mpc_int32_t mpc123_file_get_size(mpc_reader *t){
  reader_data *data=(reader_data*) t->data;
  off_t old_offset=lseek(data->fd, 0, SEEK_CUR);	/* save old pos */
  mpc_int32_t toret=lseek(data->fd, 0, SEEK_END);

  /* restore old situation */
  lseek(data->fd, old_offset, SEEK_SET);

  return toret;
}

/* on an empty disk, you can seek() forever */
static mpc_bool_t mpc123_file_canseek(mpc_reader *t){
  return MPC_TRUE;
}

/*
 * this provides raw data to the decoder library
 * handles plain files
 */
mpc_reader mpc123_file_reader={
  .read = mpc123_file_read,
  .seek = mpc123_file_seek,
  .tell = mpc123_file_tell,
  .get_size = mpc123_file_get_size,
  .canseek = mpc123_file_canseek,
  .data = NULL		/* reader_data * data */
};

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
