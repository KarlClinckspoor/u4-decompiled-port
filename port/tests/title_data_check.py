#!/usr/bin/env python3
"""Check that SRC-TITLE/DATA.C (which the port compiles, as the block port/game/block_data.c)
holds the bytes of SRC-TITLE/DATA.ASM (which the DOS build assembles).

    python3 port/tests/title_data_check.py
"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
ROOT = os.path.dirname(PORT)
INC = os.path.join(PORT, 'include')


def asm_bytes():
    asm = open(os.path.join(ROOT, 'SRC-TITLE', 'DATA.ASM'), encoding='latin-1').read().replace('\r', '')
    seg = asm[asm.index('CONST\tSEGMENT'):asm.index('CONST\tENDS')]
    vals = []
    for l in seg.split('\n'):
        m = re.search(r'\bdb\s+(.*)$', l.split(';')[0])
        if m:
            for v in m.group(1).split(','):
                v = v.strip()
                if v:
                    vals.append(int(v[:-1], 16) if v.lower().endswith('h') else int(v))
    return vals


def c_bytes():
    src = open(os.path.join(PORT, 'game', 'block_data.c')).read()
    src += ('\n#include <stdio.h>\n#undef main\nint main(void){const unsigned char *p = (const unsigned char *)'
            '&u4_block_data; unsigned i; for (i = 0; i < sizeof u4_block_data; i++) printf("%d\\n", p[i]);'
            ' return 0;}\n')
    with tempfile.TemporaryDirectory() as tmp:
        c, exe = os.path.join(tmp, 'b.c'), os.path.join(tmp, 'b')
        open(c, 'w').write(src)
        subprocess.run(['clang', '-w', '-std=gnu89', '-I', os.path.join(INC, 'title'), '-I', os.path.join(INC, 'msc'),
                        '-I', INC, '-include', os.path.join(INC, 'compat.h'), '-include',
                        os.path.join(INC, 'title', 'trename.h'), c, '-o', exe], check=True)
        return [int(x) for x in subprocess.run([exe], capture_output=True, text=True, check=True).stdout.split()]


a, c = asm_bytes(), c_bytes()
diff = [i for i in range(min(len(a), len(c))) if a[i] != c[i]]
if len(a) != len(c) or diff:
    sys.exit('DATA.C differs from DATA.ASM: %d vs %d bytes, first differences at %s' % (len(c), len(a), diff[:10]))
print('DATA.C holds DATA.ASM\'s %d bytes' % len(a))
