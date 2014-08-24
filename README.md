piece-c
=======

Fast, feature-rich Artscene render engine


Features
========

Supported file formats
----------------------

* ANSi (.ans), mostly [ECMA-048](http://www.ecma-international.org/publications/standards/Ecma-048.htm) compliant
* ASCII (.asc, .diz)
* Artworx (.adf)
* BIN (.bin)
* iCE Draw (.idf)
* PCBoard (.pcb)
* RIPScript (.rip, TODO)
* TundraDraw (.tnd)
* XBIN (.xb), fully [Acid XBIN](http://www.acid.org/info/xbin/xbin.htm) compliant


Supported pixel fonts
---------------------

*MS-DOS ("OEM") fonts*

Font                  | Description
--------------------- | ------------------------------------------------------
cp437                 | MS-DOS Latin US
cp737                 | MS-DOS Greek
cp775                 | MS-DOS Baltic Rim
cp850                 | MS-DOS Latin 1
cp852                 | MS-DOS Latin 2
cp855                 | MS-DOS Cyrillic
cp857                 | MS-DOS Turkish
cp860                 | MS-DOS Portuguese
cp861                 | MS-DOS Icelandic
cp862                 | MS-DOS Hebrew
cp863                 | MS-DOS French Canada
cp865                 | MS-DOS Nordic
cp866                 | MS-DOS Cyrillic CIS 1
cp866b                | MS-DOS Bulgarian
cp866c                | MS-DOS Russian
cp866u                | MS-DOS Ukrainian
cp869                 | MS-DOS Greek 2
cp1251                | MS-DOS Slavic

*Amiga fonts*

Font                  | Description
--------------------- | ------------------------------------------------------
b_strict              | Amiga B-Strict
b_struct              | Amiga B-Strict
microknight           | Amiga MicroKnight
microknight_plus      | Amiga MicroKnight enhanced version
mo_soul               | Amiga mO'sOul
p0t_noodle            | Amiga P0T-NOoDLE
topaz_a500            | Amiga Topaz 1 (A500, A1000, A2000)
topazplus_a500        | Amiga Topaz 1 enhanced version
topaz_a1200           | Amiga Topaz 2 (A600, A1200, A4000)
topazplus_a1200       | Amiga Topaz 2 enhanced version

*Atari fonts*

Font                  | Description
--------------------- | ------------------------------------------------------
atascii_arabic        | Atari ATASCII Arabic
atascii_graphics      | Atari ATASCII Graphics
atascii_international | Atari ATASCII International

*Commodore 64*

Font                  | Description
--------------------- | ------------------------------------------------------
petscii               | Commodore 64 PETSCII (unshifted)
petscii_shifted       | Commodore 64 PETSCII (shifted)


Supported output formats
------------------------

* Text (plain text)
* Image (bmp, gif, jpeg, png)


Requirements
============

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


Installing
----------

To install everything into ``/usr/local``:

    scons install


Uninstalling
------------

To remove the ``piece`` installation:

    scons uninstall


Acknowledgements
----------------

Inspired by the [Ansilove](http://ansilove.sourceforge.net/) ANSi to PNG
converter, the first version of ``piece`` was written in Python. Because we
required more speed, the Python version was ported to pure C and lots of
feauters and font sets were added.

