# mpc123
`mpc123` is a command-line Musepack player and decoder.  You can use it for listening to Musepack (`*.mpc`) files, or for converting them to `.wav`.
```sh
$ mpc123
[...]
Usage: mpc123 [options] [file1 [file2 [..]]]

Options supported:
   --verbose or -v          Increase verbosity
   --quiet or -q            Quiet mode (no title or boilerplate)
   --gain N or -g N         Set gain (audio volume) to N (0-100)
   -o dt                    Set output devicetype to dt
   --audiodevice N or -a N  Use N for audio-out
   --au N or -u N           Use au file N for output
   --cdr N or -c N          Use raw file N for output
   --wav N or -w N          Use wave file N for output
   --list N or -@ N         Use playlist N as list of Musepack files
   --random or -Z           Play files randomly until interrupted
   --shuffle or -z          Shuffle list of files before playing
[...]
```

# Installation
Compilation requires `libmpcdec-dev` and `libao-dev`;
Regular use requires `libmpcdec` and `libao`;

You can find those packages in your apt repository:
```sh
# apt-get install libmpcdec{6,-dev} libao{,-dev}
```
Or on their respective websites:
- [Musepack](http://www.musepack.net/) for libmpcdec
- [Xiph](http://xiph.org/ao/) for libao

# Code
```sh
$ git clone git@github.com:bucciarati/mpc123.git
```
## Compiling the bits:
Only tested on GNU/Linux, please report eventual successes/failures on other OSs.
```sh
$ make
```

At this point you should have an `mpc123` binary. Place it where you want it to live.

# Usage
`mpc123` can play files from command line arguments and from playlists.
A brief usage message is shown if you use the `-h` flag.

#### Specify path on the command line
```sh
$ mpc123 /path/to/file /example/stream.mpc
```
`mpc123` will play the files in the specified order

#### Include it into a playlist file
```sh
$ mpc123 -@ /path/to/playlist
```
`mpc123` will play files specified in the file `/path/to/playlist`. The format is trivial (each line identifies a stream), you can easily generate a playlist with the `find(1)` command:
```sh
$ find /path/to/musicroot -iname \*.mpc -fprint /path/to/playlist
```

During playback, you can have the player pass to the next playlist element (or a random one if the `-z` or `-Z` options were specified), by sending `SIGINT` to the process (this usually happens with `^C`).

Each `-v` option increases the program verbosity. The default verbosity is 0, which means `mpc123` will produce output only in case of errors;

The `-g` option modifies the output volume. Allowed values range from 0 (silent) to 100 (default value, sounds at full power); If you for example want to play at half the sample's original gain, just use `-g 50`;

The `-z` option randomly sorts the playlist once in the program lifetime (it is called _shuffle_);

The `-Z` option selects a random playlist element each time (this feature instead is called _random_);

The `-q` option resets the verbosity level to its default 0;

The `-@` option requires an argument, specifying the path to the playlist;

# AO (Audio Output) command line options
These options control the behavior of the audio output layer;

The `-o` option selects the desired audio output driver (defaults to `oss`).
Examples:
```sh
$ mpc123 -o oss -a /dev/dsp1    # to select the second OSS sound card
$ mpc123 -o alsa09              # use alsa09 as output driver, sets the output
                                #  device to "default"
$ mpc123 -o esd                 # use the esd daemon for output (the daemon
                                #  must be running, of course ..)
$ mpc123 -o null                # for debugging purposes, outputs nothing
```

The `-a` option specifies the output device for the selected driver (defaults to `/dev/dsp`)

Of course `-o` and `-a` options are related. You _can_ use only the -o option to select the alsa09 (for example) driver, and the program will default the output device to some usable value (`/dev/dsp` for oss, `default` for alsa)

# Hacking with mpc123
Clone the Git repository.
Be sure to turn the debug flags on. This can be done in a couple of ways:

```sh
% make DEBUG=1
```
or
```sh
% export DEBUG=1
% make
```

The binary file ``mpc123'' will be compiled with debugging symbols enabled, and some useful debugging macros defined; the macros are:

##### `debug()`
Is to be used to produce output with debugging activated. Takes only one argument, which is the string to print.
##### `debugf()`
The same as `debug()`, except it takes a format string as its first argument. You can use it just like `printf()`.

Both debugging macros automatically add a newline at the end of the given string, so you don't need to add `\n`.

# License
```C
/*
 *  mpc123 - Musepack Console audio player
 *  Copyright (C) 2005-2008 Fernando Vezzosi <fvezzosi at masobit.net>
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
```

