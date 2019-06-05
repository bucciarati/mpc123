/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005-2008 Fernando Vezzosi <fv at linuxvar.it>
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
#include <errno.h>
#include <string.h>

#include <ao/ao.h>

#include "mpc123.h"

/*
 * do the actual playing job
 * things learned from libmpcdec/src/sample.cpp
 */
int do_play_stream(mpc_reader * the_reader, reader_data * data){
  mpc_streaminfo tmp_stream_info;
  mpc_demux *mpc123_decoder = NULL;
  mpc_frame_info frame;
  mpc_status err;
  int played=0;

  void * ao_data=NULL;

  /* decode-phase stuff */
  MPC_SAMPLE_FORMAT buffer[MPC_DECODER_BUFFER_LENGTH];
  unsigned decoded_samples=0, total_decoded=0;

  /* read file's streaminfo data */
  mpc123_decoder = mpc_demux_init(the_reader);
  if( !mpc123_decoder ){
    die("Not a valid musepack file\n");
  }

  mpc_demux_get_info(mpc123_decoder, &tmp_stream_info);

  if( mpc123_ao_init(&ao_data, &tmp_stream_info) != 0 ){
    dief("Could not initialize audio library: error %d\n", errno);
  }

  /* decoding loop */
  while(1){
    frame.buffer = buffer;
    err = mpc_demux_decode(mpc123_decoder, &frame);

    if( err != MPC_STATUS_OK ){      /* eof */
      debugf("End of file after %d samples", total_decoded);
      break;
    }

    if( err == -1 ){ /* decoding error */
      debug("Error decoding stream.");
      say(0, "Error while decoding -- maybe corrupted data?\n");
      break;
    }

    decoded_samples = frame.samples;
    total_decoded += decoded_samples;

    played=mpc123_ao_play(ao_data, (void *)buffer, decoded_samples * 2);

    /* we can't play */
    if( !played )
      break;

    /* next cycle if no flag is set */
    if( options.flags == 0 )
      continue;

    /* we don't want to play anymore .. */
    if( mpc123_flag_isset(MPC123_FL_PLAYNEXT) ){
      mpc123_flag_unset(MPC123_FL_PLAYNEXT);
      break;
    }

    /* break out of loop and leave flag handling to the upper level */
    if( mpc123_flag_isset(MPC123_FL_STOP) )
      break;
  }

  sayf(1, "Total samples decoded: %u\n", total_decoded);
  mpc123_ao_done(ao_data);

  return 0;
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
