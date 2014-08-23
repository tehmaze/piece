import os
import mkfont

required_headers = (
    'stdio.h',
    'stdlib.h',
)
required_libs = (
    ('gd', 'gd.h'),
)
env = Environment(
    CC='clang',
    CCFLAGS=[
        '-g',
        '-std=c99',
        '-Wall',
        '-Wextra',
        '-Wno-pointer-arith',
        '-pedantic-errors',
        '-D_GNU_SOURCE',
    ],
    CPPPATH='include',
    LINKFLAGS=[
    ],
)
env.Append(
    BUILDERS={
        'Fontb642png': Builder(
            action='contrib/b642png -o $TARGET $SOURCE',
            suffix='.png',
            src_suffix='.b64',
        ),
        'Fontpng2hex': Builder(
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


cfg = Configure(env)
if env.GetOption('clean'):
    Delete('bin')
    Delete('lib')
else:
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
env_sauce = env.Clone()
env_sauce.Append(
    LINKFLAGS=['-lsauce'],
)

# Build .png from .b64
# Build .hex from .png
source_hex = [
    Glob('build/piece/font/hex/*.hex'),
] + [
    env.Fontpng2hex(png) for png in Glob('build/piece/font/png/*.png') + [
        env.Fontb642png(b64) for b64 in Glob('build/piece/font/b64/*.b64')
    ]
]

generated = []

def build_font_c(target, source, env):
    mkfont.convert_to(map(str, source), str(target[0]))
    generated.append(str(target[0]))


target_font = env.Command(
    source=source_hex,
    target='build/piece/font.c',
    action=build_font_c,
)


sauce_src = [
    'build/sauce/sauce.c',
]
sauce = env_sauce.Program(
    'bin/sauce',
    ['build/sauce/main.c'],
)
libsauce = env.SharedLibrary(
    'lib/sauce',
    sauce_src,
)
libsauce_static = env.StaticLibrary(
    'lib/sauce',
    sauce_src,
)

piece_src = [
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
    target_font,
    'build/piece/font_base.c',
] + generated
piece = env.Program(
    'bin/piece',
    piece_src + ['build/piece/main.c'],
)
libpiece = env.SharedLibrary(
    'lib/piece',
    piece_src,
)
libpiece_static = env.StaticLibrary(
    'lib/piece',
    piece_src,
)

if 'install' in COMMAND_LINE_TARGETS or 'uninstall' in COMMAND_LINE_TARGETS:
    prefix = '/usr/local'
    targets = [
        ('bin', piece, []),
        ('bin', sauce, []),
        ('lib', libpiece, []),
        ('lib', libpiece_static, []),
        ('lib', libsauce, []),
        ('lib', libsauce_static, []),
        ('include', ['include/piece.h'], []),
        ('include', Glob('include/piece/*.h'), ['piece']),
        ('include', ['include/sauce.h'], []),
    ]
    for typ, item, subs in targets:
        dirs = [prefix]
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
