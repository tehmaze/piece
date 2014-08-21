#!/usr/bin/env python2

import os
import sys


def convert(filename, stream=sys.stdout):
    fontname = os.path.splitext(os.path.basename(filename))[0]
    fontname = fontname.replace('-', '_')
    glyphs   = []
    comments = []
    h = 16
    w = 8
    with open(filename) as handle:
        for line in handle:
            line = line.rstrip()
            if line.startswith('# Height:'):
                h = int(line.split(': ')[1])
            elif line.startswith('# Width:'):
                w = int(line.split(': ')[1])
            elif line.startswith('#'):
                comments.append(line[1:].strip())
            else:
                glyphs.append(line.split(':')[1].decode('hex'))

    l = len(glyphs)
    if comments:
        for comment in comments:
            stream.write('// {0}\n'.format(comment))
        stream.write('\n')
    stream.write('const unsigned char {0}_font_glyphs[{t}] = {{\n'.format(
    #stream.write('static unsigned char **{0}_font_glyphs[] = {{\n'.format(
        fontname,
        l=l,
        h=h,
        t=l * h,
    ))
    as_hex = lambda c: '0x%02x' % (ord(c),)
    for i, glyph in enumerate(glyphs):
        stream.write('    {0}'.format(
            ', '.join(map(as_hex, glyph)),
        ))
        if i != (l - 1):
            stream.write(',')
        stream.write('\n')
    stream.write('};\n\n')
    stream.write('static font {0}_font = {{\n'.format(fontname))
    stream.write('    "{0}",\n'.format(fontname))
    stream.write('    {w},\n    {h},\n    {l},\n'.format(w=w, h=h, l=l))
    stream.write('    {0}_font_glyphs\n'.format(fontname))
    stream.write('};\n\n')

    return fontname


def convert_to(sources, target):
    with open(target, 'w') as handle:
        handle.write('/* This file is generated, do not modify */\n\n')
        handle.write('/* Splint directives */\n/*@+charint@*/\n\n')
        handle.write('#include <stdlib.h>\n')
        handle.write('#include "font.h"\n\n')
        handle.write('#include "util.h"\n\n')
        fontnames = []
        for source in sources:
            fontnames.append(convert(str(source), handle))

        handle.write('void font_init(void) {\n')
        handle.write('    fonts = allocate(sizeof(list));\n');
        handle.write('    list_new(fonts, NULL);\n')
        for fontname in fontnames:
            handle.write('    list_append(fonts, &{0}_font);\n'.format(fontname))
        handle.write('}\n\n')


def run():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('font', nargs=1)

    options = parser.parse_args()
    convert(options.font[0])


if __name__ == '__main__':
    sys.exit(run())
