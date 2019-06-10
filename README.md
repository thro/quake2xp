
============================================  
Noabody: https://sourceforge.net/u/noabody/profile/  
Bugfix: https://sourceforge.net/p/quake2xp/bugs/39/  
============================================  

Quake2XP Linux Notes  

1. Building and installing  
2. Copying data  
3. Notes  
4. Contact  
5. TODO  

============================================  
1. Building and installing  
============================================  

The source code can be downloaded from SourceForge as:
$ svn checkout svn://svn.code.sf.net/p/quake2xp/code/trunk quake2xp-code

The following libraries are needed to compile Quake2XP.
- DevIL
- OpenGL
- OpenAL (>= 1.14, see troubleshooting)
- SDL
- Vorbisfile (which requires Ogg and Vorbis)

In Ubuntu they can be installed with the following command.
$ sudo apt-get install build-essential libvorbis-dev libdevil-dev \
  libsdl1.2-dev libopenal-dev

As the project uses the Waf build system, Python must also be present. Once
you have the mentioned packages, build and install with:

$ python waf configure
$ python waf
$ sudo python waf install

By default the installation prefix is "/usr/local", but can be changed via
arguments. In fact, Quake2XP will run from any directory because the data path
is added to the executable, and libraries are loaded at run-time. For example,
you can install it in "$HOME/local" as follows.

$ python waf configure --prefix=$HOME/local
$ python waf
$ python waf install

If you have the required libraries but still get an error, see below for
contact information.

You can also uninstall it with "python waf uninstall".

============================================  
2. Copying data  
============================================  

Before running the program, you need to copy the following data to
"$PREFIX/share/quake2xp" (under baseq2/). Note that all EXEs are
self-extracting archives (i.e. can be extracted without Wine).

- baseq2/pak0.pak from the original Quake II CD

- Updated baseq2/ (without DLLs) from q2-3.20-x86-full.exe
  Available at ftp://ftp.idsoftware.com/idstuff/quake2/ or any mirror.
  After extracting it you should have baseq2/pak1.pak and baseq2/pak2.pak

- Quake2XP game data
  Available at https://yadi.sk/d/RP5yRSM-sx5ck
  You should put all the .pkx files in the baseq2/xatrix/rogue directory

- Quake2XP shaders
  For the moment, shaders must be installed manually. Download with:
  $ svn checkout svn://svn.code.sf.net/p/quake2xp/code/glsl glsl
  Then may copy the "glsl" folder under your "baseq2/".

- Quake2XP relight
  Relight must be installed manually too. Download with:
  $ svn checkout svn://svn.code.sf.net/p/quake2xp/code/maps maps
  for vanilla
  and
  $ svn checkout svn://svn.code.sf.net/p/quake2xp/code/mapsx mapsx
  for xatix

Expansion packs

If you have the official expansion packs ("xatrix" and "rogue") copy the
pak*.pak files and video/ folder to the corresponding directory under
"$PREFIX/share/quake2xp" (not under baseq2/). Then start the game as:
$ quake2xp +set game xatrix
for "the reconing" mp
and
$ quake2xp +set compatibility 1 +set game rogue
or other mod folder

Additional mods

The process of installing other mods is similar, but there is a potential
issue. In case the folder contains a gamex86.dll file, you probably need a
Linux version of it. The equivalent game.so is (in general) easy to build if
you have the source code, but keep in mind it's different for x86 and x86_64.

If the mod already includes one appropiate for your architecture, just rename
it to game.so and start Quake2XP as mentioned before for expansion packs.

============================================  
3. Notes  
============================================  

If you experience sound distortions (specially under water), check your OpenAL
Soft version. There is a known problem in 1.13, that was fixed in 1.14. If
your distribution is outdated or you can't update the system package, just
download and build it from source, and then run the game as:
$ LD_PELOAD=/home/user/Downloads/openal-soft-1.14/build/libopenal.so quake2xp

The framerate is unlimited by default, but you can adjust it with
"cl_maxfps 60" or similar. You may want to do it for smooth playing when
running background processes, for saving battery or just to avoid hearing the
GPU cooling fan.

In case you experience audio delays, try selecting "PulseAudio" instead of
"ALSA" as the sound device in the options menu.

The music system has three modes (accessible through the options menu or the
"s_musicsrc" cvar with integers from 0 to 3):
- disabled: do not play anything.
- CD-ROM: plays the appropiate tracks from an inserted CD.
- soundtrack files: play ogg/wav files with the name
  "baseq2/music/trackXX.EXT" (where XX is 02, 03, etc). They will be used as
  the original CD tracks (different for each level).
- any files: plays any ogg/wav files found in "baseq2/music".

If random playing is enabled, it should do what's expected. The command
"music" can switch tracks if playing random or any files.

============================================  
4. Contact  
============================================  

If you have any problems or suggestions regarding the Linux version, feel free
to mail me at "alepulver at gmail.com".

The Quake2XP author's address is "barnes at yandex.ru".

Website: http://quake2xp.sourceforge.net/

============================================  
5. TODO  
============================================  

- add note about 3zb2, after testing
- get launchpad account, create Ubuntu package and promote in
  forums (english and spanish)

============================================  
