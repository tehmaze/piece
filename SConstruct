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
        '-pedantic-errors',
        '-D_GNU_SOURCE',
    ],
    CPPPATH='include',
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


target = env.Program(
    'piece',
    [
        'src/list.c',
        'src/util.c',
        'src/sauce.c',
        'src/screen.c',
        'src/parser.c',
        Glob('src/parser/*.c'),
        'src/writer.c',
        Glob('src/writer/*.c'),
        target_font,
        'src/font_base.c',
        'src/main.c',
    ] + generated
)
