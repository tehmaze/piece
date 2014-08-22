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
        #'-pg',
        '-std=c99',
        '-Wall',
        '-Wextra',
        '-Wno-pointer-arith',
        '-pedantic-errors',
        '-D_GNU_SOURCE',
    ],
    CPPPATH='include',
    LINKFLAGS=[
        #'-pg',
    ],
)
cfg = Configure(env)
if not env.GetOption('clean'):
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

env = cfg.Finish()

generated = []

def build_font_c(target, source, env):
    mkfont.convert_to(map(str, source), str(target[0]))
    generated.append(str(target[0]))


target_font = env.Command(
    source=Glob('font/*.hex'),
    target='src/font.c',
    action=build_font_c,
)


sauce = env.Program(
    'sauce',
    [
        'src/list.c',
        'src/util.c',
        'src/font_base.c',
        'src/sauce.c',
        'src/mainsauce.c',
    ],
)
piece = env.Program(
    'piece',
    [
        'src/list.c',
        'src/util.c',
        'src/sauce.c',
        'src/screen.c',
        'src/palette.c',
        'src/parser.c',
        Glob('src/parser/*.c'),
        'src/writer.c',
        Glob('src/writer/*.c'),
        target_font,
        'src/font_base.c',
        'src/main.c',
    ] + generated
)

prefix = '/usr/local'
binaries = [piece, sauce]
for bin in binaries:
    path = os.path.join(prefix, 'bin')
    base = str(list(bin)[0])
    env.Alias('install', env.Install(path, bin))
    env.Command('uninstall-' + base, os.path.join(path, base), [
        Delete('$SOURCE'),
    ])
    env.Alias('uninstall', 'uninstall-' + base)
