piece-c
=======

Fast, feature-rich Artscene render engine


Features
--------

Supported file formats:

* ANSi (.ans), mostly [ECMA-048](http://www.ecma-international.org/publications/standards/Ecma-048.htm) compliant
* ASCII (.asc, .diz)
* Artworx (.adf)
* BIN (.bin, TODO)
* iCE Draw (.idf)
* PCBoard (.pcb)
* RIPScript (.rip, TODO)
* TundraDraw (.tnd)
* XBIN (.xb), fully [Acid XBIN](http://www.acid.org/info/xbin/xbin.htm) compliant


Supported pixel fonts:

* MS-DOS Code Page 437, 850, 852, 857, 860, 861, 862, 863, 865, 866, 869
* Amiga Topaz 1, 1+ (A500, A1000, A2000), 2, 2+ (A600, A1200, A4000)
* Amiga B-Strict, B-Struct
* Amiga MicroKnight, MicroKnight+, mO'sOul, P0T-NOoDLE
* Atari ATASCII
* Commodore 64 PETSCII, PETSCII shifted (PET, VIC-20, C64, CBM-II, Plus/4, C16, C116 and C128)


Supported output formats:

* Text (plain text)
* Image (bmp, gif, jpeg, png)


Requirements
------------

You need the following to compile ``piece``:

* [libgd](http://libgd.bitbucket.org/)
* [Scons](http://www.scons.org/)
* [Python](http://python.org/)


Building
--------

To compile everything from source:

    scons

To clean the build root:

    scons -c


Acknowledgements
----------------

Inspired by the [Ansilove](http://ansilove.sourceforge.net/) ANSi to PNG
converter, the first version of ``piece`` was written in Python. Because we
required more speed, the Python version was ported to pure C and lots of
feauters and font sets were added.

