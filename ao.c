/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005, 2006 Fernando Vezzosi <fvezzosi at masobit.net>
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

#include "mpc123.h"

#include <ao/ao.h>

typedef struct mpc123_ao_data {
  ao_device * ao_dev;
  void * output_buffer;
  unsigned int output_buffer_size;
} mpc123_ao_data;

/* free a linked-list of ao_option's */
static void walk_free(ao_option * elem){
  if(elem->next){
    walk_free(elem->next);
  }
  free(elem);
}

void mpc123_choose_default_dev_by_driver(){
  /* device is already specified? then we don't need work here */
  if ( options.ao_dev ){
    debugf("WARNING: device already present (\"%s\"), but they want me"
           " to find another one .. why ??", options.ao_dev);
    return;
  }

  /* default device for oss is /dev/dsp */
  if ( !strcmp(options.ao_driver, "oss") ){
    options.ao_dev="/dev/dsp";
    return;
  }

  /* this matches "alsa" as well as "alsa09", use default card */
  if ( !strncmp(options.ao_driver, "alsa", strlen("alsa")) ){
    options.ao_dev="default";
    return;
  }

  /* other drivers don't need special device specs */
}

int mpc123_ao_init(void ** d, mpc_streaminfo * streaminfo){
  int ao_drvnum=-1;
  ao_sample_format ao_fmt;
  ao_info * ao_dinfo=NULL;
  ao_option * opt_head=NULL;
  int i;

  *d=malloc(sizeof(mpc123_ao_data));
  mpc123_ao_data * data=(mpc123_ao_data *) *d;
  
  ao_initialize();

  ao_drvnum=ao_driver_id(options.ao_driver);
  if( ao_drvnum < 0 )
    die("No suitable output driver\n");

  /* set esd options */
  if( !strcmp(options.ao_driver, "esd") ){
    opt_head=malloc(sizeof(ao_option));
    opt_head->key="esd";
    opt_head->value=options.ao_dev;
    opt_head->next=NULL;
    debugf("[ESD] ao_opts{%s}=%s", opt_head->key, opt_head->value);
  }

  /* set oss options */
  if( !strcmp(options.ao_driver, "oss") ){
    opt_head=malloc(sizeof(ao_option));
    opt_head->key="dsp";
    opt_head->value=options.ao_dev;
    opt_head->next=NULL;
    debugf("[OSS] ao_opts{%s}=%s", opt_head->key, opt_head->value);
  }

  /* set alsa options */
  if( !strcmp(options.ao_driver, "alsa09") ){
    opt_head=malloc(sizeof(ao_option));
    opt_head->key="dev";
    opt_head->value=options.ao_dev;
    opt_head->next=NULL;
    debugf("[ALSA] ao_opts{%s}=%s", opt_head->key, opt_head->value);
  }

  ao_dinfo=ao_driver_info(ao_drvnum);

  debugf("<aodriver> type=%s",
        ao_dinfo->type == AO_TYPE_LIVE ? "live" : 
        ao_dinfo->type == AO_TYPE_FILE ? "file" :
        "unknown");
  debugf("<aodriver> name=%s", ao_dinfo->name);
  debugf("<aodriver> sname=%s", ao_dinfo->short_name);
  debugf("<aodriver> comment=%s", ao_dinfo->comment);
  debug("<aodriver> options={");
  for(i=0; i<ao_dinfo->option_count; i++){
    debugf("  %s", ao_dinfo->options[i]);
  }
  debug("}");

  sayf(2, "Using ao driver \"%s\" (libao id %d)\n", ao_dinfo->name, ao_drvnum);

  /* initialize ao_format struct */
  /* XXX VERY WRONG */
  ao_fmt.bits=16;       /*tmp_stream_info.average_bitrate;*/
  ao_fmt.rate=streaminfo->sample_freq;
  ao_fmt.channels=streaminfo->channels;
  ao_fmt.byte_format=AO_FMT_LITTLE;

  /* output audio params */
  debugf("<ao_fmt> bits = %d", ao_fmt.bits);
  debugf("<ao_fmt> rate = %d", ao_fmt.rate);
  debugf("<ao_fmt> channels = %d", ao_fmt.channels);
  debugf("<ao_fmt> byte_format = %d", ao_fmt.byte_format);

#if 0
  if(strcmp(options.ao_driver,"wav")         /* ao_driver IS NOT wav */
     && strcmp(options.ao_driver,"raw")      /*              NOR raw */
     && strcmp(options.ao_driver,"au")){     /*              NOR au  */
#endif
  if(ao_dinfo->type == AO_TYPE_LIVE){
    /* live playing! */
    data->ao_dev=ao_open_live(ao_drvnum, &ao_fmt, opt_head);
  }else{
    /* output to file (act as a decoder) */
    data->ao_dev=ao_open_file(ao_drvnum, options.foutput, TRUE, &ao_fmt, NULL);
  }
  
  if( !data->ao_dev ){
    dief("Could not open audio output: [%d] %s\n", errno, strerror(errno));
  }

  if(opt_head)
    walk_free(opt_head);

  return 0;
}

#ifdef MPC_FIXED_POINT
inline static int shift_signed(MPC_SAMPLE_FORMAT val, int shift)
{
  if (shift > 0)
    val <<= shift;
  else if (shift < 0)
    val >>= -shift;
  return (int) val;
}
#endif

/* play buffer[samples * 2] on device data->ao_dev */
int mpc123_ao_play(void * d, MPC_SAMPLE_FORMAT * buffer, unsigned samples){
  mpc123_ao_data * data=(mpc123_ao_data *) d;
  unsigned n; int val;
  unsigned char * output;
  unsigned bytes=samples * 2;   /* 16 bit == 2 bytes */

  const unsigned pSize = 16;
  const int clip_min    = -1 << (pSize - 1);
  const int clip_max    = (1 << (pSize - 1)) - 1;
  const int float_scale =  1 << (pSize - 1);

  /* if the buffer is already allocated, use it */
  output = data->output_buffer ? (unsigned char *) data->output_buffer :
          (data->output_buffer = (unsigned char *) malloc(bytes));

  /* realloc if the size changed */
  if(data->output_buffer_size != bytes){
    debugf("osize=%d nsize=%d", data->output_buffer_size, bytes);
    output = data->output_buffer = realloc(data->output_buffer, bytes);
    data->output_buffer_size = bytes;
  }

  if(!output){
    dief("Out of memory (needed 0x%08x bytes)\n", bytes);
  }

  /* code stolen from xmms-musepack */
  for (n = 0; n < samples; n++) {
#ifdef MPC_FIXED_POINT
    val = shift_signed(buffer[n], pSize - MPC_FIXED_POINT_SCALE_SHIFT);
#else
    /* adjust gain */
    buffer[n] *= options.volume;

    /* convert to pcm */
    val = (int) (buffer[n] * float_scale);
#endif
    if (val < clip_min)
      val = clip_min;
    else if (val > clip_max)
      val = clip_max;

    /* store pcm bytes in buffer, in the right order */
    output[n * 2] = (unsigned char) (val & 0xFF);
    output[n * 2 + 1] = (unsigned char) ((val >> 8) & 0xFF);
  }

  /* ao_play(ao_device *, void *, uint_32); */
  val=ao_play(data->ao_dev, (void *)output, bytes);
  return val;
}

/* called when done playing */
int mpc123_ao_done(void * d){
  mpc123_ao_data * data=(mpc123_ao_data *) d;

  ao_close(data->ao_dev);
  free(data);

  return 0;
}

/* vim:ft=c:tw=78:ts=2:et:cin:
 */
