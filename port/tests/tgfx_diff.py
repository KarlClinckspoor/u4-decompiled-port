#!/usr/bin/env python3
"""TITLE.EXE's EGA graphics (GRAPH.ASM, INFLATE.ASM: the original machine code of
TITLE_0.EXE, run in unicorn) against gfx/tgfx.c, entry by entry, on random and edge-case
arguments and on every compressed file the title loads.

TITLE_0.EXE (unpacked) is loaded at a fixed segment with its relocations applied; the
routines are found in TITLE_0.MAP and called through the title's own Gra_* dispatcher
(GraphicsType = 2) with DS = SS = DGROUP, as the C code calls them (cdecl or pascal).
pShapes / pCharset point at far buffers. The C runtime functions _Inflate calls (_open,
_lseek, _malloc, _read, _close, _free, _exit) are replaced by traps served in Python
(files from the data directory, a near heap above the stack): they are not under test.

Before every case both sides get the same random planes, shapes, charset, pictures, line
table and random states; after it the test compares the four 64 KB planes, the palette
registers, the shapes, the charset, the two picture buffers, the line table, GR_SEGMENT,
the random states, and the hooks (speaker toggles, file reads, exit code). It also checks
that the decoder's string stack (100h bytes at DGROUP:6826h) never overflows and that the
pictures decode to exactly 32000 bytes.

    python3 tests/tgfx_diff.py [--cases N] [--seed S] [--data DIR] [--exe TITLE_0.EXE]
                               [--map TITLE_0.MAP] [--build DIR]
"""
import argparse
import ctypes
import os
import random
import re
import struct
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True     # no __pycache__ in the tree
HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
from ega_model import Machine  # noqa: E402
from unicorn import UC_HOOK_MEM_WRITE  # noqa: E402

LOAD = 0x1000                       # the load segment of the EXE image
SHAPES, CHARSET, PIC, BUF = 0x3000, 0x4000, 0x5000, 0x6000    # far buffers
RET_IP = 0x0002                     # the return address (in _TEXT, never executed)
SP_TOP = 0xB860                     # DGROUP: the top of the EXE's stack
HEAP, HEAP_END = 0xB860, 0xF000     # DGROUP: the near heap _malloc serves
TILES = 0xF000                      # DGROUP: tile numbers for the view routines
NAME = 0xFF00                       # DGROUP: a file name
# DGROUP variables (TITLE.H, GRAPH.ASM)
LINES, REVEAL, GR_SEGMENT, SEED = 0x023C, 0x03CC, 0x337E, 0x3A86
GRAPHICSTYPE, PSHAPES, PCHARSET, TXT_X, TXT_Y = 0x7078, 0x707A, 0x7084, 0x7088, 0x708A
PATCH_TANDY = 0x6933
STACK_END = 0x6926                  # just past the decoder's string stack D_6826 (to speed_info)
# the files the title decompresses (EGA): pictures through Gra_LoadPicture, the tiles
# through LoadCompressed
PICTURES = ['TITLE.EGA', 'ANIMATE.EGA', 'TREE.EGA', 'PORTAL.EGA', 'OUTSIDE.EGA', 'INSIDE.EGA',
            'WAGON.EGA', 'GYPSY.EGA', 'ABACUS.EGA', 'HONCOM.EGA', 'VALJUS.EGA', 'SACHONOR.EGA',
            'SPIRHUM.EGA']
RUNTIME = ['_open', '_lseek', '_malloc', '_read', '_close', '_free', '_exit']
ENTRIES = ['_Gra_DrawView', '_Gra_Dot', 'GRA_CLEARMENU', '_Gra_Blit', '_Gra_LoadPicture',
           'Gra_ClearText', 'GRA_INIT', 'GRA_ANIMFLOW', 'GRA_ANIMFLAGS', 'GRA_PUTCHAR',
           'GRA_CLEARSCREEN', '_Gra_DrawViewOver', '_Gra_BlitReveal', 'InterleaveCharset',
           'INTERLEAVESHAPES', 'LOADCOMPRESSED']


def read_map(path):
    syms = {}
    for line in open(path, errors='replace'):
        mt = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(\S+)\s*$', line)
        if mt:
            syms[mt.group(3)] = (int(mt.group(1), 16), int(mt.group(2), 16))
    mt = re.search(r'([0-9A-F]{4}):0\s+DGROUP', open(path, errors='replace').read())
    return syms, int(mt.group(1), 16)


def build(builddir):
    os.makedirs(builddir, exist_ok=True)
    out = os.path.join(builddir, 'libtgfx_shim.so')
    srcs = [os.path.join(HERE, 'tgfx_shim.c'), os.path.join(PORT, 'gfx', 'ega.c')]
    deps = srcs + [os.path.join(HERE, 'shim_hooks.c')] + [
        os.path.join(PORT, 'gfx', f) for f in ('tgfx.c', 'tgfx.h', 'ega.h', 'ega_hw.h')]
    if not os.path.exists(out) or os.path.getmtime(out) < max(os.path.getmtime(d) for d in deps):
        subprocess.check_call(['gcc', '-shared', '-fPIC', '-O1', '-Wall', '-Wno-unused-function',
                               '-I', os.path.join(PORT, 'gfx'), '-o', out] + srcs)
    return out


class Shim:
    """tgfx.c through ctypes."""

    def __init__(self, path, datadir):
        self.lib = lib = ctypes.CDLL(path)
        self.vram = (ctypes.c_uint8 * 0x40000).in_dll(lib, 'ega_vram')
        self.pal = (ctypes.c_uint8 * 16).in_dll(lib, 'ega_palette_regs')
        self._datadir = ctypes.c_char_p(datadir.encode())
        ctypes.c_void_p.in_dll(lib, 'shim_datadir').value = ctypes.cast(self._datadir, ctypes.c_void_p).value
        for f in ('shim_lines', 'shim_seed', 'shim_reveal_a', 'shim_reveal_b'):
            getattr(lib, f).restype = ctypes.c_void_p
        lib.shim_out_count.restype = ctypes.c_uint
        self.lines = (ctypes.c_uint16 * 200).from_address(lib.shim_lines())
        self.seed = (ctypes.c_uint8 * 4).from_address(lib.shim_seed())
        self.reveal_a = ctypes.c_uint8.from_address(lib.shim_reveal_a())
        self.reveal_b = ctypes.c_uint8.from_address(lib.shim_reveal_b())
        self.shapes = (ctypes.c_uint8 * 0x10000)()
        self.charset = (ctypes.c_uint8 * 0x2000)()
        self.pic = (ctypes.c_uint8 * 0x10000)()
        self.buf = (ctypes.c_uint8 * 0x10000)()
        self.tiles = (ctypes.c_uint8 * 0x1000)()

    def var(self, name, ctype=ctypes.c_int):
        return ctype.in_dll(self.lib, name).value

    def planes(self):
        b = bytes(self.vram)
        return [b[k * 0x10000:(k + 1) * 0x10000] for k in range(4)]

    def set_planes(self, planes):
        ctypes.memmove(self.vram, b''.join(planes), 0x40000)

    def ptr(self, arr, ofs=0):
        return ctypes.cast(ctypes.addressof(arr) + ofs, ctypes.POINTER(ctypes.c_uint8))


class Title:
    """TITLE_0.EXE in unicorn."""

    def __init__(self, m, exe, mapfile):
        self.m = m
        syms, dgseg = read_map(mapfile)
        self.sym = {k: v[1] for k, v in syms.items() if v[0] == 0}
        self.dg = LOAD + dgseg
        hdr = struct.unpack('<14H', exe[:28])
        nrel, hdrsize, relo = hdr[3], hdr[4] * 16, hdr[12]
        img = bytearray(exe[hdrsize:])
        for i in range(nrel):
            off, seg = struct.unpack('<HH', exe[relo + 4 * i:relo + 4 * i + 4])
            a = seg * 16 + off
            struct.pack_into('<H', img, a, (struct.unpack_from('<H', img, a)[0] + LOAD) & 0xFFFF)
        m.write(LOAD * 16, img)
        # the C runtime functions become "int F0h; ret" traps
        self.traps = {}
        for f in RUNTIME:
            a = self.sym[f]
            m.write(m.lin(LOAD, a), b'\xCD\xF0\xC3')
            self.traps[a + 2] = f
        m.on_int(0xF0, self._trap)
        self.heap = HEAP
        self.exit_code = None
        self.opens = []
        self.overflow = []
        m.uc.hook_add(UC_HOOK_MEM_WRITE, self._overflow, None,
                      m.lin(self.dg, STACK_END), m.lin(self.dg, STACK_END + 7))
        self.wdg(GRAPHICSTYPE, 2)
        self.wdg(PSHAPES, 0)
        self.wdg(PSHAPES + 2, SHAPES)
        self.wdg(PCHARSET, 0)
        self.wdg(PCHARSET + 2, CHARSET)

    def _overflow(self, uc, access, addr, size, value, ud):
        self.overflow.append(addr - self.dg * 16)

    def dga(self, off):
        return self.m.lin(self.dg, off)

    def wdg(self, off, v):
        self.m.ww(self.dga(off), v)

    def rdg(self, off):
        return self.m.rw(self.dga(off))

    def arg(self, k):
        m = self.m
        return m.rw(m.lin(m.reg('ss'), m.reg('sp') + 2 + 2 * k))

    def _trap(self, m):
        f = self.traps.get(m.reg('ip'))
        ds = m.reg('ds')
        if f == '_open':
            name = m.cstr(m.lin(ds, self.arg(0)))
            h = m.dos.open(name)
            self.opens.append(name)
            m.set(ax=0xFFFF if h is None else h)
        elif f == '_lseek':
            h, off, whence = self.arg(0), self.arg(1) | self.arg(2) << 16, self.arg(3)
            fl = m.dos.files[h]
            fl[1] = (0, fl[1], len(fl[0]))[whence] + off
            m.set(ax=fl[1] & 0xFFFF, dx=fl[1] >> 16)
        elif f == '_malloc':
            n = self.arg(0)
            if self.heap + n > HEAP_END:
                m.set(ax=0)
            else:
                m.set(ax=self.heap)
                self.heap += (n + 1) & ~1
        elif f == '_read':
            h, buf, n = self.arg(0), self.arg(1), self.arg(2)
            fl = m.dos.files[h]
            data = fl[0][fl[1]:fl[1] + n]
            fl[1] += len(data)
            m.write(m.lin(ds, buf), data)
            m.set(ax=len(data))
        elif f == '_close':
            m.dos.files.pop(self.arg(0), None)
            m.set(ax=0)
        elif f == '_free':
            pass
        elif f == '_exit':
            self.exit_code = self.arg(0)
            m.stop('exit')
        else:
            raise RuntimeError('trap at %04X' % m.reg('ip'))

    def call(self, name, args, cf=0):
        m = self.m
        self.exit_code = None
        self.opens = []
        self.overflow = []
        self.heap = HEAP
        m.set(ss=self.dg, sp=SP_TOP, ds=self.dg, es=self.dg, bp=0x1111, si=0x2222, di=0x3333,
              ax=0x4444, bx=0x5555, cx=0x6666, dx=0x7777)
        return m.call(LOAD, self.sym[name], [a & 0xFFFF for a in args], False, LOAD, RET_IP,
                      flags=0x0002 | cf)


class Harness:
    def __init__(self, args):
        self.rng = random.Random(args.seed)
        self.data = args.data
        self.m = Machine(args.data)
        self.t = Title(self.m, open(args.exe, 'rb').read(), args.map)
        self.c = Shim(build(args.build), args.data)
        self.failures = []
        self.counts = {}
        self.verbose = args.verbose
        self.max_out = {}

    # ---- state ----
    def put_far(self, seg, host, data):
        self.m.write(self.m.lin(seg, 0), data)
        ctypes.memmove(host, data, len(data))

    def picture_bytes(self):
        """a random picture: random bytes with black spans (all four planes 0)"""
        r = self.rng
        b = bytearray(r.randbytes(0x10000))
        for _ in range(r.randint(0, 40)):
            a, n = r.randrange(0x1F40), r.randint(1, 400)
            for k in range(4):
                s = a + 0x1F40 * k
                b[s:s + n] = bytes(len(b[s:s + n]))
        return bytes(b)

    def randomize(self):
        r, m, c, t = self.rng, self.m, self.c, self.t
        planes = [r.randbytes(0x10000) for _ in range(4)]
        m.ega.load(planes)
        c.set_planes(planes)
        m.ega.reset_registers()
        m.ega.latch = [r.randrange(256) for _ in range(4)]
        self.put_far(SHAPES, c.shapes, r.randbytes(0x10000))
        self.put_far(CHARSET, c.charset, r.randbytes(0x2000))
        self.put_far(PIC, c.pic, self.picture_bytes())
        self.put_far(BUF, c.buf, r.randbytes(0x10000))
        seed = r.choice([bytes(4), r.randbytes(4), r.randbytes(4)])
        m.write(t.dga(SEED), seed)
        ctypes.memmove(c.seed, seed, 4)
        rv = r.randbytes(2)
        m.write(t.dga(REVEAL), rv)
        c.reveal_a.value, c.reveal_b.value = rv[0], rv[1]
        m.ega.port61 = r.choice([0x30, 0x31, 0x32, 0x33, 0x4C, 0xFC, 0x00])
        m.ega.out61 = []
        m.ega.int10_log = []
        m.dos.log = []
        c.lib.shim_reset_hooks()

    def compare(self, entry, desc, extra=()):
        m, c, t = self.m, self.c, self.t
        bad = []
        cp = c.planes()
        for k in range(4):
            a, b = bytes(m.ega.planes[k]), cp[k]
            if a != b:
                diffs = [i for i in range(0x10000) if a[i] != b[i]]
                bad.append('plane %d: %d bytes differ, first at %04Xh (dos %02X, c %02X)'
                           % (k, len(diffs), diffs[0], a[diffs[0]], b[diffs[0]]))
        if list(m.ega.palette) != list(c.pal):
            bad.append('palette dos %s c %s' % (m.ega.palette, list(c.pal)))
        for name, seg, n, host in (('shapes', SHAPES, 0x10000, c.shapes),
                                   ('charset', CHARSET, 0x2000, c.charset),
                                   ('picture', PIC, 0x10000, c.pic),
                                   ('buffer', BUF, 0x10000, c.buf)):
            a, b = m.read(m.lin(seg, 0), n), bytes(host)
            if a != b:
                diffs = [i for i in range(n) if a[i] != b[i]]
                bad.append('%s: %d bytes differ, first at %04Xh (dos %02X, c %02X)'
                           % (name, len(diffs), diffs[0], a[diffs[0]], b[diffs[0]]))
        lines = struct.unpack('<200H', m.read(t.dga(LINES), 400))
        if list(lines) != list(c.lines):
            i = next(i for i in range(200) if lines[i] != c.lines[i])
            bad.append('Lines[%d] dos %04X c %04X' % (i, lines[i], c.lines[i]))
        a, b = m.read(t.dga(SEED), 4), bytes(c.seed)
        if a != b:
            bad.append('seed 3A86h dos %s c %s' % (a.hex(), b.hex()))
        a, b = m.read(t.dga(REVEAL), 2), bytes([c.reveal_a.value, c.reveal_b.value])
        if a != b:
            bad.append('reveal state 03CCh dos %s c %s' % (a.hex(), b.hex()))
        if t.overflow:
            bad.append('the decoder\'s string stack overflowed: writes at %s' % [hex(x) for x in t.overflow[:4]])
        bad.extend(extra)
        self.counts[entry] = self.counts.get(entry, 0) + 1
        if bad:
            self.failures.append((entry, desc, bad))
            if self.verbose or len(self.failures) <= 20:
                print('FAIL %s %s' % (entry, desc))
                for b in bad:
                    print('    ' + b)

    def run(self, entry, name, dargs, fn, cargs, extra=None):
        p61 = self.m.ega.port61
        self.t.call(name, dargs, cf=self.rng.randint(0, 1))
        getattr(self.c.lib, fn)(*cargs)
        self.compare(entry, '%s%s' % (name, tuple(dargs)), list(extra(p61)) if extra else [])

    def noise_check(self, p61):
        outs = self.m.ega.out61
        bad = []
        if not outs or outs[-1] != p61:
            bad.append('port 61h not restored: start %02X outs %s' % (p61, outs[-3:]))
        flips = len(outs) - 1
        if self.c.var('shim_noise_calls') != 1 or self.c.var('shim_noise_last') != flips:
            bad.append('speaker: dos %d toggles, tgfx noise(%d) x%d'
                       % (flips, self.c.var('shim_noise_last'), self.c.var('shim_noise_calls')))
        return bad

    def file_check(self, name, expect_exit=None, out_len=None):
        bad = []
        reads = self.c.var('shim_reads')
        cname = (ctypes.c_char * 256).in_dll(self.c.lib, 'shim_last_name').value.decode()
        if len(self.t.opens) != reads or self.t.opens[-1].upper() != cname.upper():
            bad.append('file reads: dos %s, tgfx %d (%s)' % (self.t.opens, reads, cname))
        if expect_exit is None and self.c.var('shim_frees') != reads:
            bad.append('tgfx frees %d reads %d' % (self.c.var('shim_frees'), reads))
        ce = self.c.var('shim_exit_code') if self.c.var('shim_exit_calls') else None
        if self.t.exit_code != expect_exit or ce != expect_exit:
            bad.append('exit: dos %s tgfx %s expected %s' % (self.t.exit_code, ce, expect_exit))
        if out_len is not None and expect_exit is None:
            n = self.c.lib.shim_out_count()
            self.max_out[name.upper()] = n
            if n != out_len:
                bad.append('%s decodes to %d bytes, not %d' % (name, n, out_len))
        return bad

    def tiles(self, n):
        t = self.rng.randbytes(n + 2)
        self.m.write(self.t.dga(TILES), t)
        ctypes.memmove(self.c.tiles, t, len(t))

    def name(self, s):
        self.m.write(self.t.dga(NAME), s.encode() + b'\0')
        return ctypes.c_char_p(s.encode())

    # ---- the cases ----
    def cases(self, N):
        r, c, L, t = self.rng, self.c, self.c.lib, self.t

        # Gra_Init first: the others need its line table and GR_SEGMENT
        for i in range(max(3, N // 20)):
            self.randomize()
            pal = [r.randrange(64) for _ in range(16)]
            self.m.ega.palette = list(pal)
            ctypes.memmove(c.pal, bytes(pal), 16)
            pt = r.choice([0, r.randint(1, 0xFFFF)])
            t.wdg(PATCH_TANDY + 2, pt)
            t.call('GRA_INIT', [])
            L.tgfx_init(c.shapes, c.charset)
            bad = []
            if t.rdg(GR_SEGMENT) != 0xA000:
                bad.append('GR_SEGMENT %04X' % t.rdg(GR_SEGMENT))
            if t.rdg(PATCH_TANDY + 2) != 0:
                bad.append('patch_tandy not cleared')
            frees = [e for e in self.m.dos.log if e[0].startswith('free')]
            if len(frees) != 1:
                bad.append('int 21h AH=49h: %s' % frees)
            modes = [e for e in self.m.ega.int10_log if e[0] in (0x000D, 0x1002)]
            if len(modes) != 2:
                bad.append('int 10h: %s' % self.m.ega.int10_log)
            self.compare('Gra_Init', '(patch_tandy %04X)' % pt, bad)

        for name, fn in (('InterleaveCharset', 'tgfx_interleave_charset'),
                         ('INTERLEAVESHAPES', 'tgfx_interleave_shapes')):
            for i in range(max(5, N // 10)):
                self.randomize()
                host = c.charset if 'Charset' in name else c.shapes
                self.run(name, name, [], fn, [host])

        for i in range(N):
            self.randomize()
            line = r.choice([104, r.randint(0, 199)])
            h = r.randint(1, (199 - line) // 16 + 1)
            w = r.choice([19, r.randint(1, 20), r.randint(0, 255), 0x100 + r.randint(0, 3)])
            self.tiles(h * ((w & 0xFF) or 256))
            junk = [r.randint(0, 0xFFFF) for _ in range(3)]
            self.run('Gra_DrawView', '_Gra_DrawView', [h, w, TILES, line] + junk,
                     'tgfx_draw_view', [h, w, c.tiles, line])

        for i in range(N):
            self.randomize()
            line = r.choice([104, r.randint(0, 199)])
            h = r.randint(1, (199 - line) // 16 + 1)
            w = r.choice([19, r.randint(1, 20), r.randint(0, 255), 0x100 + r.randint(0, 3)])
            self.tiles(h * ((w & 0xFF) or 256))
            off = r.choice([0, r.randint(0, 0x200)])
            self.run('Gra_DrawViewOver', '_Gra_DrawViewOver', [h, w, TILES, line, off, BUF, r.randint(0, 99)],
                     'tgfx_draw_view_over', [h, w, c.tiles, line, c.ptr(c.buf, off)])

        for i in range(N):
            self.randomize()
            a = [r.randint(0, 199), r.choice([r.randint(0, 319), r.randint(0, 0xFFFF)]),
                 r.choice([0, 1, 2, 3, 0x102, 0xFF03])]
            self.run('Gra_Dot', '_Gra_Dot', a, 'tgfx_dot', a)

        for name, fn, n in (('GRA_CLEARMENU', 'tgfx_clear_menu', 'Gra_ClearMenu'),
                            ('Gra_ClearText', 'tgfx_clear_text', 'Gra_ClearText'),
                            ('GRA_CLEARSCREEN', 'tgfx_clear_screen', 'Gra_ClearScreen')):
            for i in range(max(5, N // 10)):
                self.randomize()
                self.run(n, name, [], fn, [])

        for reveal in (False, True):
            for i in range(N):
                self.randomize()
                h = r.randint(1, 200)
                sy, dy = r.randint(0, 200 - h), r.randint(0, 200 - h)
                w = r.choice([r.randint(0, 40), 40, 6, 30])
                sx, dx = r.randint(0, 40), r.randint(0, 40)
                if r.random() < 0.3:                     # the title's calls
                    w, h, sx, sy, dy, dx = r.choice([(4, 4, 19, 17, 17, 19), (21, 9, 9, 21, 21, 9),
                                                     (30, 45, 5, 34, 34, 5), (40, 152, 0, 0, 0, 0),
                                                     (6, 32, 12, 64, 0, 34), (33, 6, 3, 81, 80, 3)])
                off = r.choice([0, 0, r.randint(0, 0x100)])
                if reveal and w == 0:
                    off = 0     # 65536 bytes a line: the offsets wrap at the segment's end
                if reveal:
                    step = r.choice([r.randint(0, 56), r.randint(0, 56) + 0x100 * r.randint(1, 255)])
                    self.run('Gra_BlitReveal', '_Gra_BlitReveal', [w, h, sx, sy, off, PIC, dy, step, dx],
                             'tgfx_blit_reveal', [w, h, sx, sy, c.ptr(c.pic, off), dy, step, dx],
                             extra=self.noise_check)
                else:
                    rand = r.choice([0xFFFF, r.randint(0, 0xFFFF)])
                    hh = h if r.random() < 0.8 else h + 0x100 * r.randint(1, 255)   # a byte
                    self.run('Gra_Blit', '_Gra_Blit', [w, hh, sx, sy, off, PIC, dy, rand, dx],
                             'tgfx_blit', [w, hh, sx, sy, c.ptr(c.pic, off), dy, dx])

        for i in range(N):
            self.randomize()
            tile = r.choice([0, 1, 2, 0x4C, 0x44, 0x46, 0x47, 0x45, r.randint(0, 255), r.randint(256, 511)])
            self.run('Gra_AnimFlow', 'GRA_ANIMFLOW', [tile], 'tgfx_anim_flow', [tile])

        for i in range(N):
            self.randomize()
            self.run('Gra_AnimFlags', 'GRA_ANIMFLAGS', [], 'tgfx_anim_flags', [])

        for i in range(N):
            self.randomize()
            y, x = r.randint(0, 24), r.randint(0, 39)
            ch = r.choice([r.randint(0x20, 0x7F), 9, r.randint(0, 255), r.randint(0, 0xFFFF)])
            t.wdg(TXT_X, x)
            t.wdg(TXT_Y, y)
            self.run('Gra_PutChar', 'GRA_PUTCHAR', [ch], 'tgfx_putchar', [y, x, ch])

        # Gra_LoadPicture: every picture the title loads; LoadCompressed: SHAPES.EGZ and,
        # to try the decoder on more data, the pictures too
        for f in PICTURES:
            for k in range(2):
                self.randomize()
                off = 0 if k == 0 else r.randint(1, 0x100)
                nm = f.lower() if k else f
                cn = self.name(nm)
                t.call('_Gra_LoadPicture', [NAME, off, BUF], cf=r.randint(0, 1))
                L.tgfx_load_picture(cn, c.ptr(c.buf, off))
                self.compare('Gra_LoadPicture', '(%s, %d)' % (nm, off), self.file_check(nm, None, 32000))
        for f in ['SHAPES.EGZ'] + PICTURES:
            self.randomize()
            off = r.choice([0, r.randint(1, 0x100)])
            cn = self.name(f)
            t.call('LOADCOMPRESSED', [off, BUF, NAME])
            L.tgfx_load_compressed(cn, c.ptr(c.buf, off))
            self.compare('LoadCompressed', '(%s, %d)' % (f, off),
                         self.file_check(f, None, 32768 if f == 'SHAPES.EGZ' else None))
        for entry, name in (('Gra_LoadPicture', '_Gra_LoadPicture'), ('LoadCompressed', 'LOADCOMPRESSED')):
            self.randomize()
            cn = self.name('nosuch.ega')
            t.call(name, [NAME, 0, BUF] if name[0] == '_' else [0, BUF, NAME])
            getattr(L, 'tgfx_load_picture' if name[0] == '_' else 'tgfx_load_compressed')(cn, c.buf)
            self.compare(entry, '(nosuch.ega)', self.file_check('nosuch.ega', 0x36))
        # the real sequence: SHAPES.EGZ, Gra_Init (interleaves), then views of real tiles
        self.randomize()
        cn = self.name('SHAPES.EGZ')
        t.call('LOADCOMPRESSED', [0, SHAPES, NAME])
        L.tgfx_load_compressed(cn, c.shapes)
        bad = self.file_check('SHAPES.EGZ', None, 32768)
        t.call('GRA_INIT', [])
        L.tgfx_init(c.shapes, c.charset)
        self.compare('Gra_Init', '(after SHAPES.EGZ)', bad)
        for i in range(max(5, N // 10)):
            self.tiles(5 * 19)
            self.run('Gra_DrawView', '_Gra_DrawView', [5, 19, TILES, 104, 0, 0, 2],
                     'tgfx_draw_view', [5, 19, c.tiles, 104])
            for tile in (0, 1, 2, 0x4C, 0x44, 0x46, 0x47, 0x45):
                self.run('Gra_AnimFlow', 'GRA_ANIMFLOW', [tile], 'tgfx_anim_flow', [tile])
            self.run('Gra_AnimFlags', 'GRA_ANIMFLAGS', [], 'tgfx_anim_flags', [])


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--cases', type=int, default=200, help='random cases per entry (default 200)')
    ap.add_argument('--seed', type=int, default=4)
    ap.add_argument('--data', default='/home/claude/u4data', help='the game files')
    ap.add_argument('--exe', default='/home/claude/scratch/r1/TITLE_0.EXE', help='TITLE.EXE, unpacked')
    ap.add_argument('--map', default=None, help='its link map (default: the .MAP next to it)')
    ap.add_argument('--build', default=os.path.join(tempfile.gettempdir(), 'u4gfx_tests'),
                    help='scratch directory for the shim library')
    ap.add_argument('-v', '--verbose', action='store_true')
    args = ap.parse_args()
    args.map = args.map or os.path.splitext(args.exe)[0] + '.MAP'
    h = Harness(args)
    h.cases(args.cases)
    total = sum(h.counts.values())
    print('TITLE.EXE (EGA) vs tgfx.c: %d cases' % total)
    for k in sorted(h.counts):
        nf = sum(1 for f in h.failures if f[0] == k)
        print('  %-18s %5d cases  %s' % (k, h.counts[k], 'ok' if not nf else '%d FAILED' % nf))
    if h.failures:
        print('%d failures' % len(h.failures))
        return 1
    print('all identical (string stack never past 100h bytes; pictures decode to 32000 bytes)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
