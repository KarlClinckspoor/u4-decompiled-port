#!/usr/bin/env python3
"""Generate port/sys/bss_avatar.c and port/sys/bss_title.c: the uninitialised globals the
assembly modules define (their _BSS segments and VAR.ASM's c_common), for the host.

    python3 port/tools/gen_bss.py

In DOS each module lays its globals out one after the other, and the game sometimes reaches
past one into the next: LoadCombatMap reads a combat map of (char *)&Fighters -
(char *)&Combat bytes into Combat, CMD_HoleUp clears 32 entries of Fighters._tile, which
run on into _gtile, ... The port keeps each module's order and contiguity: one block of
host memory a program, each global an assembler symbol set at its place in it. The modules
follow each other in link order. Objects that hold pointers are bigger on the host (8-byte
pointers, a host jmp_buf), so their offsets differ from DOS's; every run of non-pointer
objects stays exactly as contiguous as in DOS. Private symbols keep their room and get no
name. TITLE.EXE's names take the T_ prefix the port gives that program (port/title).

The sizes on the host come from a probe compiled with the game's headers.
"""
import os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
ROOT = os.path.dirname(PORT)

PROGRAMS = {
    'avatar': dict(src='SRC', inc='avatar', header='u4.h', prefix='',
                   modules=[('LOW.ASM', '_BSS'), ('UTIL2.ASM', '_BSS'), ('U4_GRAPH.ASM', '_BSS'),
                            ('U4_VAR.ASM', 'c_common')]),
    'title': dict(src='SRC-TITLE', inc='title', header='title.h', prefix='T_',
                  modules=[('GRAPH.ASM', '_BSS'), ('LOW.ASM', '_BSS'), ('UTIL2.ASM', '_BSS'),
                           ('VAR.ASM', 'c_common')]),
}


# a jmp_buf: 18 bytes in DOS, 200 with glibc, 256 with MinGW (16-byte aligned): the room
# reserved for it, checked by the generated file
JMP_BUFS = {'GameLoopJmp'}
JMP_BUF_ROOM = 512


def num(expr):
    expr = re.sub(r'\b([0-9][0-9A-Fa-f]*)[hH]\b', lambda m: str(int(m.group(1), 16)), expr)
    return eval(expr, {})


def parse(path, segment):
    """[name, size, public] in order, from one segment of an assembly file"""
    lines = open(path, encoding='latin-1').read().replace('\r', '').split('\n')
    publics = set()
    for l in lines:
        m = re.match(r'^\s*PUBLIC\s+(.*)$', l, re.I)
        if m:
            publics |= {p.strip() for p in m.group(1).split(';')[0].split(',') if p.strip()}
    objs, inside = [], False
    for l in lines:
        code = l.split(';')[0].rstrip()
        if re.match(r'^%s\s+SEGMENT\b' % segment, code, re.I):
            inside = True
            continue
        if re.match(r'^%s\s+ENDS\b' % segment, code, re.I):
            inside = False
            continue
        if not inside:
            continue
        m = re.match(r'^(\S*)\s+(db|dw|dd)\s+(.*)$', code, re.I)
        if not m:
            continue
        name, kind, rest = m.groups()
        unit = {'db': 1, 'dw': 2, 'dd': 4}[kind.lower()]
        rest = rest.strip()
        m2 = re.match(r'^(.*?)\s+dup\s*\((.*)\)$', rest, re.I)
        if m2:
            size = num(m2.group(1)) * unit * len(m2.group(2).split(','))
        else:
            size = unit * len(rest.split(','))
        if not name:
            objs[-1][1] += size        # the rest of the object above (struct parts, padding)
            continue
        objs.append([name, size, name in publics])
    return objs


def probe(prog, names):
    """{name: (host size, size with 4-byte pointers, is an array)} from the program's
    headers; an array (perhaps of unknown size) gives its element's sizes"""
    cfg = PROGRAMS[prog]
    inc = os.path.join(PORT, 'include')
    hdr = ''
    d = os.path.join(ROOT, cfg['src'])
    for h in os.listdir(d):
        if h.upper().endswith('.H'):
            hdr += open(os.path.join(d, h), encoding='latin-1').read()
    hdr = re.sub(r'/\*.*?\*/', ' ', hdr, flags=re.S)
    names = [n for n in names if re.search(r'\b%s\b' % n, hdr)]
    arr = {n: bool(re.search(r'\b%s\s*\[' % n, hdr)) for n in names}
    res = {}
    with tempfile.TemporaryDirectory() as tmp:
        for bits in ('64', '32'):
            lines = ['#include "%s"' % cfg['header']]
            for n in names:
                lines.append('int probe_%s = sizeof(%s%s);' % (n, n, '[0]' if arr[n] else ''))
            c = os.path.join(tmp, 'p%s.c' % bits)
            open(c, 'w').write('\n'.join(lines) + '\n')
            s = os.path.join(tmp, 'p%s.s' % bits)
            if bits == '32':
                # 4-byte pointers: a freestanding 32-bit compile, the C library stubbed
                stub = os.path.join(tmp, 'stub')
                os.makedirs(stub, exist_ok=True)
                for h in ('stdio.h', 'stdlib.h', 'string.h'):
                    open(os.path.join(stub, h), 'w').write('')
                open(os.path.join(stub, 'setjmp.h'), 'w').write('typedef int jmp_buf[6];\n')
                res_dir = subprocess.run(['clang', '-print-resource-dir'], capture_output=True,
                                         text=True).stdout.strip()
                target = ['--target=i686-linux-gnu', '-ffreestanding', '-nostdinc',
                          '-isystem', os.path.join(res_dir, 'include'), '-isystem', stub]
            else:
                target = ['-m64']
            r = subprocess.run(['clang'] + target + ['-std=gnu89', '-fsigned-char', '-w',
                               '-I', os.path.join(inc, cfg['inc']), '-I', os.path.join(inc, 'msc'), '-I', inc,
                               '-include', os.path.join(inc, 'compat.h'), '-S', c, '-o', s],
                               capture_output=True, text=True)
            if r.returncode:
                sys.exit(r.stderr[-3000:])
            cur = None
            for l in open(s).read().split('\n'):
                m = re.match(r'^_?probe_(\w+):', l)
                if m:
                    cur = m.group(1)
                    continue
                m = re.match(r'^\s*\.long\s+(\d+)', l)
                if m and cur:
                    res.setdefault(cur, {})[bits] = int(m.group(1))
                    cur = None
    return {n: (v['64'], v['32'], arr[n]) for n, v in res.items()}


def generate(prog):
    cfg = PROGRAMS[prog]
    objs = []
    for f, seg in cfg['modules']:
        objs += [o + [f] for o in parse(os.path.join(ROOT, cfg['src'], f), seg)]
    sizes = probe(prog, [o[0] for o in objs if o[2]])
    out = ['/* Generated by port/tools/gen_bss.py: do not edit.',
           ' * %s\'s uninitialised globals from its assembly modules (%s), in DOS\'s' %
           (prog.upper() + '.EXE', ', '.join('%s %s' % m for m in cfg['modules'])),
           ' * order, in one block, so that the game\'s reads and writes past one global into the',
           ' * next land as in DOS. */', '#include <stdint.h>', '#include <setjmp.h>', '',
           '_Static_assert(sizeof(jmp_buf) <= %d, "the room for a jmp_buf");' % JMP_BUF_ROOM, '']
    entries, off = [], 0
    for name, dos, public, f in objs:
        host = dos
        if public and name in sizes:
            h64, h32, is_array = sizes[name]
            if h64 != h32:          # holds pointers: the host's size
                host = dos // h32 * h64 if is_array else max(h64, dos + h64 - h32)
        if name in JMP_BUFS:
            host = JMP_BUF_ROOM     # the host's jmp_buf, whatever its C library
        if host != dos:
            off = (off + 15) & ~15  # host pointers aligned (nothing in DOS reaches into them)
        entries.append((name, off, host, dos, public, f))
        off += host
        if host != dos:
            off = (off + 15) & ~15
    total = (off + 15) & ~15
    block = 'u4_bss_%s' % prog
    out += ['#define BSS_SIZE %d' % total,
            '__attribute__((aligned(16))) uint8_t %s[BSS_SIZE];' % block, '',
            '#if defined(__APPLE__) || (defined(_WIN32) && !defined(_WIN64))',
            '#define SYM(n) "_" #n', '#else', '#define SYM(n) #n', '#endif',
            '#define AT(n, off) __asm__(".globl " SYM(n) "\\n.set " SYM(n) ", " SYM(%s) " + " #off);' % block, '']
    for name, o, host, dos, public, f in entries:
        what = '%d bytes' % host + ('' if host == dos else ' (DOS %d)' % dos)
        if public:
            out.append('AT(%s%s, %d) /* %s, %s */' % (cfg['prefix'], name, o, what, f))
        else:
            out.append('/* %d: %s (%s, private) */' % (o, name, f))
    out.append('')
    p = os.path.join(PORT, 'sys', 'bss_%s.c' % prog)
    open(p, 'w').write('\n'.join(out))
    print('%s: %d objects, %d bytes' % (p, len(entries), total))


if __name__ == '__main__':
    for prog in PROGRAMS:
        generate(prog)
