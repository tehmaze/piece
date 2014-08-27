use 5.006;
use strict;
use warnings FATAL => 'all';
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME               => 'Image::Piece',
    AUTHOR             => q{Wijnand Modderman-Lenstra <maze@pyth0n.org>},
    VERSION_FROM       => 'lib/Image/Piece.pm',
    LICENSE            => 'MIT',
    PL_FILES           => {},
    MIN_PERL_VERSION   => 5.006,
    CONFIGURE_REQUIRES => {
        'ExtUtils::MakeMaker' => 0,
    },
    BUILD_REQUIRES     => {
        'Test::More'   => 0,
    },
    PREREQ_PM          => {},
    OBJECT             => 'Piece.o',
    CCFLAGS            => '-g',
    INC                => '-I. -I../../include',
    LIBS               => ['-lgd'],
    dist               => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
    clean              => { FILES => 'Image-Piece-*' },
    dynamic_lib        => {
        OTHERLDFLAGS => '../../lib/libsauce.a ../../lib/libpiece.a',
    },
);
