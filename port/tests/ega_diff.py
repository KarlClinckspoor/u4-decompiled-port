#!/usr/bin/env python3
"""EGA.DRV (the original machine code, run in unicorn) against gfx/ega.c (its C
transliteration), entry by entry, on random and edge-case arguments.

The driver is loaded as AVATAR.EXE loads it (U4_GRAPH.ASM LoadGraphicsDriver): the file at
offset 0 of a segment; entry n is at the offset in the n-th word of the table at its start
and is far-called with the C arguments on the stack (CallDriver turns AVATAR's near call
into a far one, so the driver finds its first argument at [bp+6] and returns with RETF).
DS = SS = AVATAR's DGROUP on entry. Init gets near pointers to pShapes and pCharset and the
offset of exit(); the driver keeps the caller's CS as exit()'s segment.

Before every case both sides get the same random planes, shapes, charset, picture buffer,
random seed and latches; after it the test compares the four 64 KB planes, the palette
registers, the shapes, the charset, the picture buffer, the random seed and the hooks
(speaker toggles, file reads, exit code, busy-wait loop count). ega.c is built with
tests/ega_shim.c into a shared library in a scratch directory.

    python3 tests/ega_diff.py [--cases N] [--seed S] [--data DIR] [--build DIR]
"""
import argparse
import ctypes
import os
import random
import struct
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True     # no __pycache__ in the tree
HERE = os.path.dirname(os.path.abspath(__file__))
PORT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
from ega_model import Machine  # noqa: E402
from unicorn import UC_HOOK_CODE  # noqa: E402

DRV = 0x2000          # the driver's segment
DG = 0x1000           # AVATAR's DGROUP (DS = SS)
CALLER = 0x0F00       # the caller's code segment: return address and exit()
RET_IP, EXIT_IP = 0x0100, 0x0200
SHAPES, CHARSET, BUF = 0x3000, 0x4000, 0x5000   # far buffers (segments, offset 0)
P_SHAPES, P_CHARSET = 0x0100, 0x0104            # DGROUP variables pShapes, pCharset
NAME = 0x0200          # DGROUP: a file name
TILES = 0x0400         # DGROUP: the tile numbers of DrawTiles
KARMAS = 0x0600        # DGROUP: 8 pointers to the karmas, then the karmas at 0x0620
SEED = 0x36            # the driver's random seed (CS:0036h)

# the pictures AVATAR loads with the driver (run-length coded); the title's are LZW
TITLE_PICS = {'TITLE', 'ANIMATE', 'TREE', 'PORTAL', 'OUTSIDE', 'INSIDE', 'WAGON', 'GYPSY',
              'ABACUS', 'HONCOM', 'VALJUS', 'SACHONOR', 'SPIRHUM', 'CHARSET', 'SHAPES'}


def build(builddir):
    os.makedirs(builddir, exist_ok=True)
    out = os.path.join(builddir, 'libega_shim.so')
    src = os.path.join(HERE, 'ega_shim.c')
    deps = [src, os.path.join(HERE, 'shim_hooks.c'), os.path.join(PORT, 'gfx', 'ega.c'), os.path.join(PORT, 'gfx', 'ega.h'),
            os.path.join(PORT, 'gfx', 'ega_hw.h')]
    if not os.path.exists(out) or os.path.getmtime(out) < max(os.path.getmtime(d) for d in deps):
        subprocess.check_call(['gcc', '-shared', '-fPIC', '-O1', '-Wall', '-Wno-unused-function',
                               '-I', os.path.join(PORT, 'gfx'), '-o', out, src])
    return out


class Shim:
    """ega.c through ctypes."""

    def __init__(self, path, datadir):
        self.lib = lib = ctypes.CDLL(path)
        self.vram = (ctypes.c_uint8 * 0x40000).in_dll(lib, 'ega_vram')
        self.pal = (ctypes.c_uint8 * 16).in_dll(lib, 'ega_palette_regs')
        self._datadir = ctypes.c_char_p(datadir.encode())
        ctypes.c_void_p.in_dll(lib, 'shim_datadir').value = ctypes.cast(self._datadir, ctypes.c_void_p).value
        for f in ('shim_seed', 'shim_shapes', 'shim_charset'):
            getattr(lib, f).restype = ctypes.c_void_p
        self.seed = (ctypes.c_uint8 * 4).from_address(lib.shim_seed())
        self.shapes = (ctypes.c_uint8 * 0x10000)()
        self.charset = (ctypes.c_uint8 * 0x2000)()
        self.buf = (ctypes.c_uint8 * 0x10000)()
        self.tiles = (ctypes.c_uint8 * 0x200)()
        self.karma_vals = (ctypes.c_uint16 * 8)()
        self.karma_ptrs = (ctypes.POINTER(ctypes.c_uint16) * 8)(
            *[ctypes.pointer(ctypes.c_uint16.from_buffer(self.karma_vals, 2 * i)) for i in range(8)])

    def var(self, name, ctype=ctypes.c_int):
        return ctype.in_dll(self.lib, name).value

    def planes(self):
        b = bytes(self.vram)
        return [b[k * 0x10000:(k + 1) * 0x10000] for k in range(4)]

    def set_planes(self, planes):
        ctypes.memmove(self.vram, b''.join(planes), 0x40000)


class Driver:
    """EGA.DRV in unicorn."""

    def __init__(self, m, drv):
        self.m = m
        self.table = struct.unpack('<26H', drv[:52])
        m.write(m.lin(DRV, 0), drv)
        m.write(m.lin(CALLER, EXIT_IP), b'\xCD\xF0')     # exit(): INT F0h
        self.exit_code = None
        m.on_int(0xF0, self._exit)
        m.ww(m.lin(DG, P_SHAPES), 0)
        m.ww(m.lin(DG, P_SHAPES + 2), SHAPES)
        m.ww(m.lin(DG, P_CHARSET), 0)
        m.ww(m.lin(DG, P_CHARSET + 2), CHARSET)

    def _exit(self, m):
        self.exit_code = m.rw(m.lin(m.reg('ss'), m.reg('sp') + 2))
        m.stop('exit')

    def call(self, n, *args):
        m = self.m
        self.exit_code = None
        m.set(ss=DG, sp=0xFFF0, ds=DG, es=DG, bp=0x1234, si=0x5678, di=0x9ABC)
        assert self.table[n], 'no entry %d' % n
        return m.call(DRV, self.table[n], [a & 0xFFFF for a in args], True, CALLER, RET_IP)


class Harness:
    def __init__(self, args):
        self.rng = random.Random(args.seed)
        self.data = args.data
        self.m = Machine(args.data)
        self.drv = Driver(self.m, open(os.path.join(args.data, 'EGA.DRV'), 'rb').read())
        self.c = Shim(build(args.build), args.data)
        self.failures = []
        self.counts = {}
        self.verbose = args.verbose

    # ---- state ----
    def randomize(self):
        r, m, c = self.rng, self.m, self.c
        planes = [r.randbytes(0x10000) for _ in range(4)]
        m.ega.load(planes)
        c.set_planes(planes)
        m.ega.reset_registers()
        m.ega.latch = [r.randrange(256) for _ in range(4)]
        shapes, charset, buf = r.randbytes(0x10000), r.randbytes(0x2000), r.randbytes(0x10000)
        m.write(m.lin(SHAPES, 0), shapes)
        ctypes.memmove(c.shapes, shapes, len(shapes))
        m.write(m.lin(CHARSET, 0), charset)
        ctypes.memmove(c.charset, charset, len(charset))
        m.write(m.lin(BUF, 0), buf)
        ctypes.memmove(c.buf, buf, len(buf))
        seed = r.randbytes(4)
        m.write(m.lin(DRV, SEED), seed)
        ctypes.memmove(c.seed, seed, 4)
        m.ega.port61 = r.choice([0x30, 0x31, 0x32, 0x33, 0x4C, 0xFC, 0x00])
        m.ega.out61 = []
        m.ega.int10_log = []
        m.dos.log = []
        self.c.lib.shim_reset_hooks()

    def compare(self, entry, desc, extra=()):
        m, c = self.m, self.c
        bad = []
        cp = c.planes()
        for k in range(4):
            a, b = bytes(m.ega.planes[k]), cp[k]
            if a != b:
                diffs = [i for i in range(0x10000) if a[i] != b[i]]
                bad.append('plane %d: %d bytes differ, first at %04Xh (drv %02X, c %02X)'
                           % (k, len(diffs), diffs[0], a[diffs[0]], b[diffs[0]]))
        if list(m.ega.palette) != list(c.pal):
            bad.append('palette drv %s c %s' % (m.ega.palette, list(c.pal)))
        for name, seg, n, host in (('shapes', SHAPES, 0x10000, c.shapes),
                                   ('charset', CHARSET, 0x2000, c.charset),
                                   ('buffer', BUF, 0x10000, c.buf)):
            a, b = m.read(m.lin(seg, 0), n), bytes(host)
            if a != b:
                i = next(i for i in range(n) if a[i] != b[i])
                bad.append('%s differs first at %04Xh (drv %02X, c %02X)' % (name, i, a[i], b[i]))
        a, b = m.read(m.lin(DRV, SEED), 4), bytes(c.seed)
        if a != b:
            bad.append('seed drv %s c %s' % (a.hex(), b.hex()))
        bad.extend(extra)
        self.counts[entry] = self.counts.get(entry, 0) + 1
        if bad:
            self.failures.append((entry, desc, bad))
            if self.verbose or len(self.failures) <= 20:
                print('FAIL %s %s' % (entry, desc))
                for b in bad:
                    print('    ' + b)

    def run(self, entry, n, cargs, dargs=None, fn=None, extra=None):
        """Call driver entry n with dargs (default cargs) and ega.c's fn(*cargs)."""
        dargs = cargs if dargs is None else dargs
        p61 = self.m.ega.port61
        self.drv.call(n, *dargs)
        getattr(self.c.lib, fn)(*cargs)
        ex = list(extra(p61) if extra else [])
        self.compare(entry, '%s%s' % (fn, tuple(dargs)), ex)

    def noise_check(self, sound):
        def check(p61):
            outs = self.m.ega.out61
            bad = []
            if not outs or outs[-1] != p61:
                bad.append('port 61h not restored: start %02X outs %s' % (p61, outs[-3:]))
            # every OUT but the last (the restore) is one click: (in 61h & FCh) ^ 2 alternately
            flips = len(outs) - 1
            want = flips if sound & 0xFF else -1
            got = self.c.var('shim_noise_last')
            if self.c.var('shim_noise_calls') != 1 or got != want:
                bad.append('speaker: driver %d toggles (sound %d), ega.c noise(%d) x%d'
                           % (flips, sound & 0xFF, got, self.c.var('shim_noise_calls')))
            return bad
        return check

    def file_check(self, name, expect_exit=None, waits=None):
        def check(p61):
            bad = []
            opens = [e for e in self.m.dos.log if e[0] == 'open']
            reads = self.c.var('shim_reads')
            cname = (ctypes.c_char * 256).in_dll(self.c.lib, 'shim_last_name')
            if len(opens) != reads or (opens and opens[-1][1].upper() != name.upper()):
                bad.append('file reads: driver %s, ega.c %d (%s)' % (opens, reads, cname.value))
            allocs = sum(1 for e in self.m.dos.log if e[0] == 'alloc')
            frees = sum(1 for e in self.m.dos.log if e[0] == 'free')
            if expect_exit is None and allocs != frees:
                bad.append('driver allocs %d frees %d' % (allocs, frees))
            if expect_exit is None and self.c.var('shim_frees') != reads:
                bad.append('ega.c frees %d reads %d' % (self.c.var('shim_frees'), reads))
            ce = self.c.var('shim_exit_code') if self.c.var('shim_exit_calls') else None
            if self.drv.exit_code != expect_exit or ce != expect_exit:
                bad.append('exit: driver %s ega.c %s expected %s' % (self.drv.exit_code, ce, expect_exit))
            if waits is not None:
                cw = self.c.var('shim_wait_total', ctypes.c_long)
                if waits[0] != cw:
                    bad.append('busy wait: driver %d loops, ega.c %d' % (waits[0], cw))
            return bad
        return check

    def quad_ok(self, x0, x1, y0, y1, y0p, y1p):
        """the driver's walk (see ega_draw_quad): every column must have 0 <= ya <= yb < 200"""
        sgn = lambda v: (v > 0) - (v < 0)
        dx = x1 - x0
        sx, sa, sb = sgn(dx), sgn(y0p - y0), sgn(y1p - y1)
        ea = eb = abs(dx) >> 1
        x, ya, yb = x0, y0, y1
        for _ in range(400):
            if not (0 <= ya <= yb < 200 and 0 <= x < 320):
                return False
            x += sx
            ea -= abs(y0p - y0)
            eb -= abs(y1p - y1)
            if eb < 0:
                ya += sa
                ea += abs(dx)
            if eb < 0:
                yb += sb
                eb += abs(dx)
            if x == x1:
                return True
        return False

    def cases(self, n):
        r = self.rng
        N = n
        L = self.c.lib

        # 21: Init (first: the others need its line table and pointers)
        for i in range(max(3, N // 50)):
            self.randomize()
            pal = [r.randrange(64) for _ in range(16)]
            self.m.ega.palette = list(pal)
            ctypes.memmove(self.c.pal, bytes(pal), 16)
            self.drv.call(21, P_SHAPES, P_CHARSET, EXIT_IP)
            L.ega_init(self.c.shapes, self.c.charset)
            bad = []
            if self.m.read(self.m.lin(DRV, 0x4E), 4) != struct.pack('<HH', EXIT_IP, CALLER):
                bad.append('exit pointer not stored')
            self.compare('Init', '()', bad)

        for i in range(N):
            self.randomize()
            row, col = r.randint(0, 11), r.randint(0, 18)
            self.run('ActiveBox', 0, (row, col), fn='ega_active_box')

        for i in range(N):
            self.randomize()
            zoom = r.choice([1, 1, 2, 2, 0, 3, 0x101])
            a = (r.randint(0, 511), r.randint(0, 15), r.randint(0, 199), r.randint(0, 72), zoom)
            self.run('ZoomTileRow', 1, a, fn='ega_zoom_tile_row')

        for i in range(N):
            self.randomize()
            h = r.randint(1, 12)
            row = r.randint(0, 199 - 16 * (h - 1))
            w = r.choice([1, 2, 5, 11, 19, r.randint(1, 40), 0x100 + r.randint(1, 20), 0, 0x200])
            a = (h, w, r.randint(0, 511), row, r.randint(0, 79))
            self.run('FillTiles', 2, a, fn='ega_fill_tiles')

        for i in range(max(10, N // 5)):
            for n, fn in ((3, 'ega_shift_down'), (4, 'ega_shift_up')):
                self.randomize()
                sound = r.choice([0, 1, 0xFF, 0x100, 0x1234])
                self.run(fn[4:], n, (sound,), fn=fn, extra=self.noise_check(sound))

        for i in range(N):
            self.randomize()
            h = r.randint(1, 12)
            t = r.randbytes(11 * h + 1)
            self.m.write(self.m.lin(DG, TILES), t)
            ctypes.memmove(self.c.tiles, t, len(t))
            w = r.choice([11, 0, 5])
            self.drv.call(5, h, w, TILES)
            L.ega_draw_tiles(h, w, self.c.tiles)
            self.compare('DrawTiles', '(%d, %d, tiles)' % (h, w))

        for n, name, fn in ((6, 'Dot', 'ega_dot'), (14, 'DotOr', 'ega_dot_or'), (15, 'DotXor', 'ega_dot_xor')):
            for i in range(N):
                self.randomize()
                a = (r.randint(0, 199), r.choice([r.randint(0, 319), r.randint(0, 0xFFFF)]),
                     r.choice([0, 1, 2, 3, 0x103, 0xFF02]))
                self.run(name, n, a, fn=fn)

        for n, name, fn in ((8, 'CR', 'ega_cr'), (9, 'InvertView', 'ega_invert_view'),
                            (10, 'ClearView', 'ega_clear_view'), (13, 'ClearStatus', 'ega_clear_status'),
                            (22, 'ClearScreen', 'ega_clear_screen')):
            for i in range(max(5, N // 10)):
                self.randomize()
                self.run(name, n, (), fn=fn)

        for i in range(N):
            self.randomize()
            self.run('InvertStatusLine', 11, (r.choice([r.randint(0, 23), -1]),), fn='ega_invert_status_line')

        for i in range(N):
            self.randomize()
            self.run('InvertChar', 12, (r.randint(0, 39), r.randint(0, 24)), fn='ega_invert_char')

        # 18: DrawQuad, with the game's walls (edges symmetric) and random ones
        done = 0
        while done < N:
            x0, x1 = r.randint(0, 319), r.randint(0, 319)
            if x0 == x1:
                continue
            if r.random() < 0.5:
                y0, y1 = sorted((r.randint(0, 199), r.randint(0, 199)))
                d = r.randint(-min(y0, abs(x1 - x0)), min(199 - y1, abs(x1 - x0)))
                q = (x0, x1, y0, y1, y0 - d, y1 + d)
            else:
                y0, y1 = sorted((r.randint(0, 199), r.randint(0, 199)))
                y0p, y1p = sorted((r.randint(0, 199), r.randint(0, 199)))
                q = (x0, x1, y0, y1, y0p, y1p)
            if not self.quad_ok(*q):
                continue
            done += 1
            self.randomize()
            a = q + (r.choice([0, 1, 2, 3, 0x102]), r.choice([0, 0, 0, 1, 0x100, 0xFF]))
            self.run('DrawQuad', 18, a, fn='ega_draw_quad')

        for i in range(N):
            self.randomize()
            vals = [r.choice([0, 0, 1, 99, 0x100]) for _ in range(8)]
            for k in range(8):
                self.m.ww(self.m.lin(DG, KARMAS + 2 * k), KARMAS + 0x20 + 2 * k)
                self.m.ww(self.m.lin(DG, KARMAS + 0x20 + 2 * k), vals[k])
                self.c.karma_vals[k] = vals[k]
            self.drv.call(19, KARMAS)
            L.ega_virtue_glyph(self.c.karma_ptrs)
            self.compare('VirtueGlyph', str(vals))

        for i in range(N):
            self.randomize()
            self.run('AnimCampfire', 20, (), fn='ega_anim_campfire')

        for i in range(N):
            self.randomize()
            self.run('AnimFlow', 23, (r.randint(0, 511),), fn='ega_anim_flow')

        for i in range(N):
            self.randomize()
            self.run('AnimFlags', 24, (), fn='ega_anim_flags')

        for i in range(N):
            self.randomize()
            a = (r.randint(0, 24), r.randint(0, 39), r.choice([r.randint(0, 255), r.randint(0, 0xFFFF)]))
            self.run('PutChar', 25, a, fn='ega_putchar')

        # 16: LoadPicture, every picture AVATAR draws with it, to the screen and to a buffer
        pics = sorted(f for f in os.listdir(self.data)
                      if f.upper().endswith('.EGA') and f.upper()[:-4] not in TITLE_PICS)
        for f in pics:
            for to_buf in (False, True, True):
                self.randomize()
                ofs = r.choice([0, 0, r.randint(0, 0x8000)]) if to_buf else r.choice([0, 0, r.randint(0, 40)])
                self.load_picture(f.lower(), ofs, to_buf)
        self.randomize()
        self.load_picture('nosuch.ega', 0, False, expect_exit=3)

        # 17: RevealCodex (slow: the driver's delay loops run)
        for f in ('rune_5.ega', 'RUNE_5.EGA', 'key7.ega'):
            self.randomize()
            self.reveal(f)

    def put_name(self, name):
        self.m.write(self.m.lin(DG, NAME), name.encode() + b'\0')
        return ctypes.c_char_p(name.encode())

    def load_picture(self, name, ofs, to_buf, expect_exit=None):
        cname = self.put_name(name)
        self.drv.call(16, NAME, ofs, BUF if to_buf else 0)
        self.c.lib.ega_load_picture(cname, ofs, self.c.buf if to_buf else None)
        self.compare('LoadPicture', '(%s, %d, %s)' % (name, ofs, 'buf' if to_buf else 'screen'),
                     self.file_check(name, expect_exit)(0))

    def reveal(self, name):
        cname = self.put_name(name)
        loops = [0]

        def count(uc, addr, size, ud):
            loops[0] += 1
        h = self.m.uc.hook_add(UC_HOOK_CODE, count, None, self.m.lin(DRV, 0xBEB), self.m.lin(DRV, 0xBEB))
        self.drv.call(17, NAME)
        self.m.uc.hook_del(h)
        self.c.lib.ega_reveal_codex(cname)
        self.compare('RevealCodex', '(%s)' % name, self.file_check(name, None, loops)(0))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--cases', type=int, default=200, help='random cases per entry (default 200)')
    ap.add_argument('--seed', type=int, default=4)
    ap.add_argument('--data', default='/home/claude/u4data', help='the game files (EGA.DRV, pictures)')
    ap.add_argument('--build', default=os.path.join(tempfile.gettempdir(), 'u4gfx_tests'),
                    help='scratch directory for the shim library')
    ap.add_argument('-v', '--verbose', action='store_true')
    args = ap.parse_args()
    h = Harness(args)
    h.cases(args.cases)
    total = sum(h.counts.values())
    print('EGA.DRV vs ega.c: %d cases' % total)
    for k in sorted(h.counts):
        nf = sum(1 for f in h.failures if f[0] == k)
        print('  %-18s %5d cases  %s' % (k, h.counts[k], 'ok' if not nf else '%d FAILED' % nf))
    if h.failures:
        print('%d failures' % len(h.failures))
        return 1
    print('all identical')
    return 0


if __name__ == '__main__':
    sys.exit(main())
