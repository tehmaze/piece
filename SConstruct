import os
import sys
import mkfont
import multiprocessing
import distutils.sysconfig


for flag, description, default in (
        ('static',    'static libraries',                           True),
        ('optimize',  'optimize compiler',                          True),
        ('build-man', 'rebuild manual pages (requires md2man)',     False),
        ('libsauce',  'libsauce',                                   True),
        ('libpiece',  'libpiece',                                   True),
        ('debug',     'build with debug flags (disables optimize)', False),
    ):

    if default:
        AddOption(
            '--without-' + flag,
            dest=flag.replace('-', '_'),
            action='store_false',
            default=True,
            help='Build without ' + description
        )
    else:
        AddOption(
            '--with-' + flag,
            dest=flag.replace('-', '_'),
            action='store_true',
            default=False,
            help='Build with ' + description
        )

default=dict(
    prefix='/usr/local',
    bin_prefix='$PREFIX/bin',
    lib_prefix='$PREFIX/lib',
    share_prefix='$PREFIX/share',
    man_prefix='$SHARE_PREFIX/man',
)
AddOption(
    '--prefix',
    dest='prefix',
    metavar='PATH',
    default=default['prefix'],
    help='Install prefix for binaries and libraries, defaults to: %s' % default['prefix'],
)
AddOption(
    '--bin-prefix',
    dest='bin_prefix',
    metavar='PATH',
    default=default['bin_prefix'],
    help='Install prefix for binaries, defaults to: %s' % default['bin_prefix'],
)
AddOption(
    '--lib-prefix',
    dest='lib_prefix',
    metavar='PATH',
    default=default['lib_prefix'],
    help='Install prefix for libraries, defaults to: %s' % default['lib_prefix'],
)
AddOption(
    '--share-prefix',
    dest='share_prefix',
    metavar='PATH',
    default=default['share_prefix'],
    help='Install prefix for shared data, defaults to: %s' % default['share_prefix'],
)
AddOption(
    '--man-prefix',
    dest='man_prefix',
    metavar='PATH',
    default=default['man_prefix'],
    help='Install prefix for manuals, defaults to: %s' % default['man_prefix'],
)

num_cpu = int(os.environ.get('NUM_CPU', multiprocessing.cpu_count()))
SetOption('num_jobs', num_cpu)

env = Environment(
    CC='clang',
    CCFLAGS=[
        '-std=c99',
        '-Wall',
        '-Wextra',
        '-Wno-pointer-arith',
        '-pedantic-errors',
        '-D_GNU_SOURCE',
    ],
    CPPPATH=[
        '/usr/include',
        'include',
    ],
    LINKFLAGS=[
    ],
)

if GetOption('debug'):
    if GetOption('optimize'):
        print >>sys.stderr, '--with-debug and --with-optimize are mutually exclusive options'
        sys.exit(1)

    env.Append(
        CCFLAGS=[
            '-g',
            '-DDEBUG',
        ],
        LINKFLAGS=[
            '-g',
        ],
    )

if GetOption('static'):
    # Make relocatable binaries
    env.Append(
        CCFLAGS=[
            '-fPIC',
        ],
    )

if GetOption('optimize'):
    env.Append(
        CCFLAGS=[
            '-Os',
        ],
        LINKFLAGS=[
            '-Os',
        ],
    )

else:
    env.Append(
        CCFLAGS=[
            '-O0',
            '-g',
        ],
    )

required_headers = (
    'stdio.h',
    'stdlib.h',
)
required_libs = (
    ('gd', 'gd.h'),
)
env.Append(
    BUILDERS={
        'Font_bin2hex': Builder(
            action='contrib/bin2hex -o $TARGET $SOURCE',
            suffix='.hex',
            src_suffix='.bin',
        ),
        'Font_fnt2bin': Builder(
            action='contrib/fnt2bin -o $TARGET $SOURCE',
            suffix='.bin',
            src_suffix='.fnt',
        ),
        'Font_png2hex': Builder(
            action='contrib/png2hex $SOURCE > $TARGET',
            suffix='.hex',
            src_suffix='.png',
        ),
    }
)
env.VariantDir('build', 'src')

if ARGUMENTS.get('VERBOSE') != '1':
    env.Append(
        ARCOMSTR     = '\x1b[1;32marchive\x1b[0m $TARGET',
        CCCOMSTR     = '\x1b[1;33mcompile\x1b[0m $TARGET',
        CXXCOMSTR    = '\x1b[1;33mcompile\x1b[0m $TARGET',
        LINKCOMSTR   = '\x1b[1;35mlinking\x1b[0m $TARGET',
        RANLIBCOMSTR = '\x1b[1;32mranlib \x1b[0m $TARGET',
        SHCCCOMSTR   = '\x1b[1;33mcompile\x1b[0m $TARGET',
        SHLINKCOMSTR = '\x1b[1;35mlinking\x1b[0m $TARGET',
    )


if env.GetOption('clean'):
    Delete('bin')
    Delete('lib')

elif not env.GetOption('help'):
    cfg = Configure(env)
    cfg.CheckCC()
    cfg.CheckSHCC()

    for header in required_headers:
        if not cfg.CheckCHeader(header):
            print 'Did not find {0}'.format(header)
            Exit(1)

    for name, header in required_libs:
        if header is None:
            if not cfg.CheckLib(name):
                print 'Did not find lib{0}.a or {0}.lib'.format(name)
                Exit(1)
        else:
            if not cfg.CheckLibWithHeader(name, header, 'c'):
                print 'Did not find lib{0}.a or {0}.lib'.format(name)
                Exit(1)

    env.ParseConfig('pkg-config --cflags --libs gdlib')
    env = cfg.Finish()

    print 'Running with -j', GetOption('num_jobs')

# Build .png from .b64
# Build .hex from .png
source_hex = [
    Glob('build/piece/font/hex/*.hex'),
] + [
    env.Font_png2hex(png)
    for png in Glob('build/piece/font/png/*.png')
] + [
    env.Font_bin2hex(env.Font_fnt2bin(fnt))
    for fnt in Glob('build/piece/font/fnt/*.fnt')
]

generated = []

def build_config_h(target, source, env):
    config = env.Dictionary()
    config.update(dict(
        prefix=GetOption('prefix'),
        bin_prefix=GetOption('bin_prefix').replace('$PREFIX', GetOption('prefix')),
        lib_prefix=GetOption('lib_prefix').replace('$PREFIX', GetOption('prefix')),
        with_includes=' '.join('-I%s' % inc for inc in config['CPPPATH'] if inc.startswith(os.sep)),
        with_libs=' '.join('-l%s' % lib for lib in config['LIBS']),
        with_libpiece=int(GetOption('libpiece')),
        with_libsauce=int(GetOption('libsauce')),
    ))

    for dst, src in zip(target, source):
        with open(str(src), 'r') as sfd:
            with open(str(dst), 'w') as dfd:
                dfd.write(sfd.read().format(**config))
        print '\x1b[1;31mcreated\x1b[0m %s from %s' % (dst, src)

def build_font_c(target, source, env):
    mkfont.convert_to(map(str, source), str(target[0]))
    generated.append(str(target[0]))
    print '\x1b[1;31mcreated\x1b[0m %s from %d fonts' % (target[0], len(source))

piece_font_src = env.Command(
    source=source_hex,
    target='build/piece/font.c',
    action=build_font_c,
)

piece_config_src = env.Command(
    source='include/piece/config.h.in',
    target='include/piece/config.h',
    action=build_config_h,
)

sauce_src = [
    'build/sauce/sauce.c',
]
if GetOption('libsauce'):
    if GetOption('static'):
        libsauce = env.StaticLibrary(
            'lib/sauce',
            sauce_src,
        )
    else:
        libsauce = env.SharedLibrary(
            'lib/sauce',
            sauce_src,
        )

libpiece_src = [
    'build/sauce/sauce.c',
    'build/piece/list.c',
    'build/piece/util.c',
    'build/piece/screen.c',
    'build/piece/palette.c',
    'build/piece/sauce.c',
    'build/piece/parser.c',
    Glob('build/piece/parser/*.c'),
    'build/piece/writer.c',
    Glob('build/piece/writer/*.c'),
    'build/piece/font_base.c',
    'build/piece/chrfont.c',
] + generated + piece_font_src

if GetOption('libpiece'):
    if GetOption('static'):
        libpiece = env.StaticLibrary(
            'lib/piece',
            libpiece_src + [
                'build/piece/lib.c',
            ]
        )

    else:
        libpiece = env.SharedLibrary(
            'lib/piece',
            libpiece_src + [
                'build/piece/lib.c',
            ]
        )

if GetOption('static'):
    sauce = env.Program(
        'bin/sauce',
        ['build/sauce/main.c', libsauce],
    )
    piece = env.Program(
        'bin/piece',
        ['build/piece/main.c', libsauce, libpiece],
    )

else:
    env_sauce = env.Clone()
    env_sauce.Append(LIBS=['sauce'])
    sauce = env_sauce.Program(
        'bin/sauce',
        ['build/sauce/main.c'],
    )
    env_piece = env.Clone()
    env_piece.Append(LIBS=['sauce', 'piece'])
    piece = env_piece.Program(
        'bin/piece',
        ['build/piece/main.c'],
    )

piece_config = env.Program(
    'bin/piece-config',
    ['build/piece/main-config.c'],
)

piece_chrfont = env.Program(
    'bin/piece-chrfont',
    ['build/piece/main-chrfont.c', libpiece],
)

prefix = dict(
    all=GetOption('prefix'),
    bin=GetOption('bin_prefix').replace('$PREFIX', GetOption('prefix')),
    lib=GetOption('lib_prefix').replace('$PREFIX', GetOption('prefix')),
)
prefix['share'] = GetOption('share_prefix').replace('$PREFIX', prefix['all'])
prefix['man'] = GetOption('man_prefix').replace('$SHARE_PREFIX', prefix['share'])

if 'install' in COMMAND_LINE_TARGETS or 'uninstall' in COMMAND_LINE_TARGETS:
    targets = [
        ('bin', piece, []),
        ('bin', piece_config, []),
        ('bin', sauce, []),
        ('lib', libpiece, []),
        ('lib', libsauce, []),
        ('include', ['include/piece.h'], []),
        ('include', Glob('include/piece/*.h'), ['piece']),
        ('include', Glob('include/piece/parser/*.h'), ['piece/parser']),
        ('include', Glob('include/piece/writer/*.h'), ['piece/writer']),
        ('include', ['include/sauce.h'], []),
        ('man', Glob('man/*.1'), ['man1']),
    ]
    for typ, item, subs in targets:
        dirs = []
        try:
            dirs.append(prefix[typ])
        except KeyError:
            dirs.append(prefix['all'])
            dirs.append(typ)
        dirs.extend(subs)
        path = os.path.join(*tuple(dirs))
        if 'install' in COMMAND_LINE_TARGETS:
            env.Alias('install', env.Install(path, item))
        for base in list(item):
            base = os.path.basename(str(base))
            env.Command('uninstall-' + base, os.path.join(path, base), [
                Delete('$SOURCE'),
            ])
            env.Alias('uninstall', 'uninstall-' + base)

if GetOption('build_man'):
    SConscript(dirs=['src/man'])

# vim:ft=python:
