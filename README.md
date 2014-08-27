# piece-c

Fast, feature-rich Artscene render engine


# Example

```shellsession
$ piece -o docs/font/cp437_8x16.png docs/font/test.ans
```

And the resulting image:

<img src="docs/font/cp437_8x16.png" alt="Example rendering" />


# Features

## Supported file formats

* ANSi (.ans), mostly [ECMA-048](http://www.ecma-international.org/publications/standards/Ecma-048.htm) compliant
* ASCII (.asc, .diz)
* Artworx (.adf)
* BIN (.bin)
* iCE Draw (.idf)
* PCBoard (.pcb)
* RIPScript (.rip, TODO)
* TundraDraw (.tnd)
* XBIN (.xb), fully [Acid XBIN](http://www.acid.org/info/xbin/xbin.htm) compliant


## Supported pixel fonts

### MS-DOS ("OEM") fonts

Font                  | Description           | Sizes
--------------------- | --------------------- | -------------------------------
cp437                 | MS-DOS Latin US       | [8x8](docs/font/cp437_8x8.png) [thin_8x8](docs/font/cp437_thin_8x8.png) [8x14](docs/font/cp437_8x14.png) [8x16](docs/font/cp437_8x16.png) [thin_8x16](docs/font/cp437_thin_8x16.png)
cp737                 | MS-DOS Greek          | [8x16](docs/font/cp737_8x16.png)
cp775                 | MS-DOS Baltic Rim     | [8x16](docs/font/cp775_8x16.png)
cp850                 | MS-DOS Latin 1        | [8x8](docs/font/cp850_8x8.png) [8x14](docs/font/cp850_8x14.png) [8x16](docs/font/cp850_8x16.png) [thin_8x8](docs/font/cp850_thin_8x8.png) [thin_8x16](docs/font/cp850_thin_8x16.png)
cp852                 | MS-DOS Latin 2        | [8x16](docs/font/cp852_8x16.png) 
cp855                 | MS-DOS Cyrillic       | [8x16](docs/font/cp855_8x16.png) 
cp857                 | MS-DOS Turkish        | [8x16](docs/font/cp857_8x16.png) 
cp860                 | MS-DOS Portuguese     | [8x16](docs/font/cp860_8x16.png) 
cp861                 | MS-DOS Icelandic      | [8x16](docs/font/cp861_8x16.png) 
cp862                 | MS-DOS Hebrew         | [8x16](docs/font/cp862_8x16.png) 
cp863                 | MS-DOS French Canada  | [8x16](docs/font/cp863_8x16.png) 
cp865                 | MS-DOS Nordic         | [8x8](docs/font/cp865_8x8.png) [8x14](docs/font/cp865_8x14.png) [8x16](docs/font/cp865_8x16.png) [thin_8x8](docs/font/cp865_thin_8x8.png) [thin_8x16](docs/font/cp865_thin_8x16.png) 
cp866                 | MS-DOS Cyrillic CIS 1 | [8x8](docs/font/cp866_8x8.png) [8x14](docs/font/cp866_8x14.png) [8x16](docs/font/cp866_8x16.png) 
cp866b                | MS-DOS Bulgarian      | [8x16](docs/font/cp866b_8x16.png) 
cp866c                | MS-DOS Russian        | [8x16](docs/font/cp866c_8x16.png) 
cp866u                | MS-DOS Ukrainian      | [8x8](docs/font/cp866u_8x8.png) [8x14](docs/font/cp866u_8x14.png) [8x16](docs/font/cp866u_8x16.png) 
cp869                 | MS-DOS Greek 2        | [8x16](docs/font/cp869_8x16.png) 
cp1251                | MS-DOS Slavic         | [8x8](docs/font/cp1251_8x8.png) [8x14](docs/font/cp1251_8x14.png) [8x16](docs/font/cp1251_8x16.png) 

### Amiga fonts

Font                  | Description                        | Sizes
--------------------- | ---------------------------------- | ------------------
b_strict              | Amiga B-Strict                     | [8x8](docs/font/b_strict.png)
b_struct              | Amiga B-Strict                     | [8x8](docs/font/b_struct.png)
microknight           | Amiga MicroKnight                  | [8x16](docs/font/microknight.png)
microknight_plus      | Amiga MicroKnight enhanced version | [8x16](docs/font/microknightplus.png)
mo_soul               | Amiga mO'sOul                      | [8x16](docs/font/mo_soul.png)
p0t_noodle            | Amiga P0T-NOoDLE                   | [8x16](docs/font/p0t_noodle.png)
topaz_a500            | Amiga Topaz 1 (A500, A1000, A2000) | [8x16](docs/font/topaz_a500.png)
topazplus_a500        | Amiga Topaz 1 enhanced version     | [8x16](docs/font/topazplus_a500.png)
topaz_a1200           | Amiga Topaz 2 (A600, A1200, A4000) | [8x16](docs/font/topaz_a1200.png)
topazplus_a1200       | Amiga Topaz 2 enhanced version     | [8x16](docs/font/topazplus_a1200.png)

### Atari fonts

Font                  | Description                 | Sizes
--------------------- | --------------------------- | ------------------------
atascii_arabic        | Atari ATASCII Arabic        | [8x16](docs/font/atascii_arabic_8x16.png)
atascii_graphics      | Atari ATASCII Graphics      | [8x16](docs/font/atascii_graphics_8x16.png)
atascii_international | Atari ATASCII International | [8x16](docs/font/atascii_international_8x16.png)

### Commodore 64 fonts

Font                  | Description                      | Sizes
--------------------- | -------------------------------- | -------------------
petscii               | Commodore 64 PETSCII (unshifted) | [8x16](docs/font/petscii.png)
petscii_shifted       | Commodore 64 PETSCII (shifted)   | [8x16](docs/font/petscii_shifted.png)

### Miscellanious fonts

Font                  | Description               | Sizes
--------------------- | ------------------------- | --------------------------
armscii8              | Armenian ASCII (ARMSCII)  | [8x8](docs/font/armscii8_8x8.png) [8x14](docs/font/armscii8_8x14.png) [8x16](docs/font/armscii8_8x16.png) 
haik8                 | haik8 code page (ARMSCII) | [8x8](docs/font/haik8_8x8.png) [8x14](docs/font/haik8_8x14.png) [8x16](docs/font/haik8_8x16.png) 
iso                   | West European             | [8x8](docs/font/iso_8x8.png) [8x14](docs/font/iso_8x14.png) [8x16](docs/font/iso_8x16.png) [thin_8x16](docs/font/iso_thin_8x16.png) 
iso02                 | Central European          | [8x8](docs/font/iso02_8x8.png) [8x14](docs/font/iso02_8x14.png) [8x16](docs/font/iso02_8x16.png)  
iso04                 | Baltic                    | [8x8](docs/font/iso04_8x8.png) [8x14](docs/font/iso04_8x14.png) [8x16](docs/font/iso04_8x16.png)
iso05                 | Cyrillic                  | [8x8](docs/font/iso05_8x8.png) [8x14](docs/font/iso05_8x14.png) [8x16](docs/font/iso05_8x16.png) 
iso07                 | Greek                     | [8x8](docs/font/iso07_8x8.png) [8x14](docs/font/iso07_8x14.png) [8x16](docs/font/iso07_8x16.png) 
iso08                 | Hebrew                    | [8x8](docs/font/iso08_8x8.png) [8x14](docs/font/iso08_8x14.png) [8x16](docs/font/iso08_8x16.png) 
iso09                 | Turkish                   | [8x16](docs/font/iso09_8x16.png) 
iso15                 | West European             | [8x8](docs/font/iso15_8x8.png) [8x14](docs/font/iso15_8x14.png) [8x16](docs/font/iso15_8x16.png) [thin_8x16](docs/font/iso15_thin_8x16.png) 
koi8_r                | Russian                   | [8x14](docs/font/koi8_r_8x14.png) [8x16](docs/font/koi8_r_8x16.png) [8x8](docs/font/koi8_r_8x8.png)
koi8_rb               | Russian                   | [8x16](docs/font/koi8_rb_8x16.png)
koi8_rc               | Russian                   | [8x16](docs/font/koi8_rc_8x16.png)
koi8_u                | Ukrainian                 | [8x8](docs/font/koi8_u_8x8.png) [8x14](docs/font/koi8_u_8x14.png) [8x16](docs/font/koi8_u_8x16.png)
swiss                 | ?                         | [8x8](docs/font/swiss_8x8.png) [8x14](docs/font/swiss_8x14.png) [8x16](docs/font/swiss_8x16.png) 
swiss_1131            | Belarusian                | [8x16](docs/font/swiss_1131_8x16.png)
swiss_1251            | Cyrillic                  | [8x16](docs/font/swiss_1251_8x16.png)

## Supported output formats

* Text (plain text)
* Image (bmp, gif, jpeg, png)

## Supported dynamic languages

* [Python](http://www.python.org/) version 2.6 and 2.7


# Requirements

You need the following to compile ``piece``:

* [libgd](http://libgd.bitbucket.org/)
* [Scons](http://www.scons.org/)
* [Python](http://python.org/)


# Building

First inspect the build options, by running:

```shellsession
$ scons --help
```

To compile everything from source:

```shellsession
$ scons
```

To clean the build root:

```shellsession
$ scons -c
```

## Building the Python extension

To build the Python extension, make sure you have the Python development
headers installed.

On a Debian/Ubuntu based system:

```shellsession
$ sudo apt-get install python-dev
```

On a Red Hat/RPM based system:

```shellsession
$ sudo yum install python-devel
```

To build the extension:

```
$ scons --with-python
```


# Installing

To install everything into ``/usr/local``:

```shellsession
$ sudo scons install
```


# Uninstalling

To remove the ``piece`` installation:

```shellsession
$ sudo scons uninstall
```


# Acknowledgements

Inspired by the [Ansilove](http://ansilove.sourceforge.net/) ANSi to PNG
converter, the first version of ``piece`` was written in Python. Because we
required more speed, the Python version was ported to pure C and lots of
features and font sets were added.

The fonts found in the [fnt](/src/piece/font/fnt/) folder, are part of the
FreeBSD [newcons](https://wiki.freebsd.org/Newcons) project and BSD licensed.
