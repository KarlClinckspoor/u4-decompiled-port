"""An EGA (mode 0Dh) and a little DOS for unicorn, to run the game's 16-bit graphics code.

Used by ega_diff.py (EGA.DRV) and tgfx_diff.py (TITLE.EXE's GRAPH.ASM / INFLATE.ASM) to
compare the original machine code against the C transliterations byte for byte.

What is modelled (only what the code under test touches, but correctly for that):

  * video memory A000:0000-FFFF as four 64 KB planes, through unicorn MMIO callbacks:
      - every CPU read loads the four latches; read mode 0 returns the plane chosen by the
        read map select (GC 4), read mode 1 returns the colour compare result (GC 2, GC 7);
      - writes go through the map mask (sequencer 2); write mode 0 (data rotate and
        set/reset / enable set/reset, logical function and bit mask), write mode 1 (latch
        copy), write mode 2 (colour from the low CPU bits, then function and bit mask);
      - word accesses are two byte accesses, low byte first (the EGA is an 8-bit card).
  * ports: sequencer 3C4h/3C5h, graphics controller 3CEh/3CFh (byte and word OUTs: index in
    AL, data in AH), input status 3DAh (retrace bits toggle at each read), the speaker
    port 61h (every OUT is logged, so speaker toggles can be counted).
  * INT 10h: AH=00h set mode (mode 0Dh resets the controller registers and clears the four
    planes), AX=1000h/1001h/1002h palette and overscan (recorded in palette/overscan).
  * INT 21h (class Dos): 3Dh open, 3Eh close, 3Fh read, 42h lseek, 48h allocate, 49h free,
    4Ch exit; files are served read-only from a directory (case-insensitive names).
  * other software interrupts can be given handlers with Machine.on_int().
"""
import os
import struct

from unicorn import (Uc, UcError, UC_ARCH_X86, UC_MODE_16, UC_HOOK_INSN, UC_HOOK_INTR,
                     UC_HOOK_MEM_INVALID)
from unicorn.x86_const import *

VIDEO = 0xA0000
SEG_REGS = {'cs': UC_X86_REG_CS, 'ds': UC_X86_REG_DS, 'es': UC_X86_REG_ES, 'ss': UC_X86_REG_SS}
REGS = {'ax': UC_X86_REG_AX, 'bx': UC_X86_REG_BX, 'cx': UC_X86_REG_CX, 'dx': UC_X86_REG_DX,
        'si': UC_X86_REG_SI, 'di': UC_X86_REG_DI, 'bp': UC_X86_REG_BP, 'sp': UC_X86_REG_SP,
        'ip': UC_X86_REG_IP, 'flags': UC_X86_REG_EFLAGS}
REGS.update(SEG_REGS)

CF = 0x0001


class EmulationStop(Exception):
    pass


class EGA:
    """The EGA's planes, latches and registers (see the module comment)."""

    def __init__(self):
        self.planes = [bytearray(0x10000) for _ in range(4)]
        self.latch = [0, 0, 0, 0]
        self.seq_index = 0
        self.seq = [0] * 5
        self.gc_index = 0
        self.gc = [0] * 9
        self.palette = [0] * 16
        self.overscan = 0
        self.mode = 3
        self.port61 = 0x30
        self.out61 = []          # every value written to port 61h, in order
        self.in61 = 0            # count of reads of port 61h
        self.int10_log = []      # (ax, bx, cx, dx) of each INT 10h
        self.status = 0          # 3DAh
        self.reset_registers()

    def reset_registers(self):
        """The register state after INT 10h AX=000Dh (and the state every routine of the
        game expects on entry): map mask 0Fh, write mode 0, no rotation, function replace,
        set/reset off, read map 0, bit mask FFh."""
        self.seq = [0x03, 0x09, 0x0F, 0x00, 0x06]
        self.gc = [0, 0, 0, 0, 0, 0, 0x05, 0x0F, 0xFF]

    # ---- video memory ----
    def read(self, off):
        p = self.planes
        self.latch = [p[0][off], p[1][off], p[2][off], p[3][off]]
        if self.gc[5] & 0x08:       # read mode 1: colour compare
            cc, dc = self.gc[2], self.gc[7]
            r = 0xFF
            for k in range(4):
                if dc & (1 << k):
                    want = 0xFF if cc & (1 << k) else 0
                    r &= ~(self.latch[k] ^ want) & 0xFF
            return r
        return self.latch[self.gc[4] & 3]

    def write(self, off, v):
        mapmask = self.seq[2]
        mode = self.gc[5] & 3
        func = self.gc[3] & 0x18
        rot = self.gc[3] & 7
        bitmask = self.gc[8]
        for k in range(4):
            if not mapmask & (1 << k):
                continue
            latch = self.latch[k]
            if mode == 1:
                self.planes[k][off] = latch
                continue
            if mode == 0:
                if self.gc[1] & (1 << k):
                    d = 0xFF if self.gc[0] & (1 << k) else 0
                else:
                    d = ((v >> rot) | (v << (8 - rot))) & 0xFF if rot else v
            elif mode == 2:
                d = 0xFF if v & (1 << k) else 0
            else:
                raise EmulationStop('EGA write mode 3 does not exist on the EGA')
            if func == 0x08:
                d &= latch
            elif func == 0x10:
                d |= latch
            elif func == 0x18:
                d ^= latch
            self.planes[k][off] = (d & bitmask) | (latch & ~bitmask & 0xFF)

    def mmio_read(self, uc, off, size, ud):
        v = self.read(off & 0xFFFF)
        if size == 2:
            v |= self.read((off + 1) & 0xFFFF) << 8
        elif size != 1:
            raise EmulationStop('video read of %d bytes' % size)
        return v

    def mmio_write(self, uc, off, size, value, ud):
        self.write(off & 0xFFFF, value & 0xFF)
        if size == 2:
            self.write((off + 1) & 0xFFFF, (value >> 8) & 0xFF)
        elif size != 1:
            raise EmulationStop('video write of %d bytes' % size)

    # ---- ports ----
    def port_in(self, port):
        if port == 0x3DA:
            self.status ^= 0x09
            return self.status
        if port == 0x61:
            self.in61 += 1
            return self.port61
        if port == 0x3C4:
            return self.seq_index
        if port == 0x3C5:
            return self.seq[self.seq_index] if self.seq_index < 5 else 0
        if port == 0x3CE:
            return self.gc_index
        if port == 0x3CF:
            return self.gc[self.gc_index] if self.gc_index < 9 else 0
        raise EmulationStop('IN from port %03Xh' % port)

    def port_out(self, port, v):
        if port == 0x3C4:
            self.seq_index = v & 0xFF
        elif port == 0x3C5:
            if self.seq_index >= 5:
                raise EmulationStop('sequencer register %d' % self.seq_index)
            self.seq[self.seq_index] = v & 0xFF
        elif port == 0x3CE:
            self.gc_index = v & 0xFF
        elif port == 0x3CF:
            if self.gc_index >= 9:
                raise EmulationStop('graphics controller register %d' % self.gc_index)
            self.gc[self.gc_index] = v & 0xFF
        elif port == 0x61:
            self.port61 = v & 0xFF
            self.out61.append(v & 0xFF)
        else:
            raise EmulationStop('OUT %02Xh to port %03Xh' % (v, port))

    def speaker_toggles(self, start=None):
        """The number of times the OUTs to port 61h flipped bit 1 (the speaker data)."""
        cur = self.port61 if start is None else start
        n = 0
        for v in self.out61:
            if (v ^ cur) & 2:
                n += 1
            cur = v
        return n

    # ---- INT 10h ----
    def int10(self, m):
        ax, bx, cx, dx = (m.reg('ax'), m.reg('bx'), m.reg('cx'), m.reg('dx'))
        self.int10_log.append((ax, bx, cx, dx))
        ah, al = ax >> 8, ax & 0xFF
        if ah == 0x00:
            self.mode = al & 0x7F
            if self.mode == 0x0D:
                self.reset_registers()
                self.palette = [0, 1, 2, 3, 4, 5, 6, 7, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17]
                self.overscan = 0
                if not al & 0x80:
                    for p in self.planes:
                        p[:] = bytes(0x10000)
        elif ax == 0x1000:
            self.palette[bx & 0x0F] = (bx >> 8) & 0x3F
        elif ax == 0x1001:
            self.overscan = (bx >> 8) & 0x3F
        elif ax == 0x1002:
            regs = m.read(m.lin(m.reg('es'), dx), 17)
            self.palette = [b & 0x3F for b in regs[:16]]
            self.overscan = regs[16] & 0x3F
        else:
            raise EmulationStop('INT 10h AX=%04Xh' % ax)

    def snapshot(self):
        return [bytes(p) for p in self.planes]

    def load(self, planes):
        for k in range(4):
            self.planes[k][:] = planes[k]


class Dos:
    """INT 21h: files served read-only from datadir, a bump allocator for 48h/49h."""

    def __init__(self, datadir, heap_start=0x7000, heap_end=0x9F00):
        self.datadir = datadir
        self.files = {}
        self.next_handle = 5
        self.heap = heap_start
        self.heap_end = heap_end
        self.blocks = {}
        self.log = []            # ('open', name) ... for the tests
        self.exit_code = None

    def find(self, name):
        name = name.replace('\\', '/').split('/')[-1]
        if ':' in name:
            name = name.split(':')[-1]
        for f in os.listdir(self.datadir):
            if f.upper() == name.upper():
                return os.path.join(self.datadir, f)
        return None

    def open(self, name):
        path = self.find(name)
        self.log.append(('open', name, path is not None))
        if path is None:
            return None
        h = self.next_handle
        self.next_handle += 1
        self.files[h] = [open(path, 'rb').read(), 0, name]
        return h

    def int21(self, m):
        ah = m.reg('ax') >> 8
        fl = m.reg('flags') & ~CF
        if ah == 0x3D:
            name = m.cstr(m.lin(m.reg('ds'), m.reg('dx')))
            h = self.open(name)
            if h is None:
                m.set(ax=2, flags=fl | CF)
            else:
                m.set(ax=h, flags=fl)
        elif ah == 0x3E:
            h = m.reg('bx')
            if self.files.pop(h, None) is None:
                m.set(ax=6, flags=fl | CF)
            else:
                self.log.append(('close', h))
                m.set(flags=fl)
        elif ah == 0x3F:
            h, n, dx = m.reg('bx'), m.reg('cx'), m.reg('dx')
            f = self.files.get(h)
            if f is None:
                m.set(ax=6, flags=fl | CF)
                return
            data = f[0][f[1]:f[1] + n]
            f[1] += len(data)
            m.write(m.lin(m.reg('ds'), dx), data)
            self.log.append(('read', f[2], n, len(data)))
            m.set(ax=len(data), flags=fl)
        elif ah == 0x42:
            h, al = m.reg('bx'), m.reg('ax') & 0xFF
            off = (m.reg('cx') << 16) | m.reg('dx')
            f = self.files.get(h)
            if f is None:
                m.set(ax=6, flags=fl | CF)
                return
            if off & 0x80000000:
                off -= 1 << 32
            base = (0, f[1], len(f[0]))[al]
            f[1] = max(0, base + off)
            m.set(ax=f[1] & 0xFFFF, dx=f[1] >> 16, flags=fl)
        elif ah == 0x48:
            paras = m.reg('bx')
            if self.heap + paras > self.heap_end:
                m.set(ax=8, bx=self.heap_end - self.heap, flags=fl | CF)
                return
            seg = self.heap
            self.heap += paras
            self.blocks[seg] = paras
            self.log.append(('alloc', paras))
            m.set(ax=seg, flags=fl)
        elif ah == 0x49:
            seg = m.reg('es')
            if self.blocks.pop(seg, None) is None:
                self.log.append(('free-invalid', seg))
                m.set(ax=9, flags=fl | CF)
            else:
                self.log.append(('free', seg))
                m.set(flags=fl)
        elif ah == 0x4C:
            self.exit_code = m.reg('ax') & 0xFF
            m.stop('exit')
        else:
            raise EmulationStop('INT 21h AH=%02Xh' % ah)


class Machine:
    """A 16-bit real mode PC: 640 KB of RAM, the EGA, INT 10h/21h, helpers to call code."""

    def __init__(self, datadir):
        self.uc = uc = Uc(UC_ARCH_X86, UC_MODE_16)
        uc.mem_map(0, 0xA0000)
        self.ega = EGA()
        uc.mmio_map(VIDEO, 0x10000, self.ega.mmio_read, None, self.ega.mmio_write, None)
        uc.mem_map(0xB0000, 0x50000)
        self.dos = Dos(datadir)
        self.int_handlers = {0x10: self.ega.int10, 0x21: self.dos.int21}
        self.error = None
        self.stop_reason = None
        uc.hook_add(UC_HOOK_INSN, self._in, None, 1, 0, UC_X86_INS_IN)
        uc.hook_add(UC_HOOK_INSN, self._out, None, 1, 0, UC_X86_INS_OUT)
        uc.hook_add(UC_HOOK_INTR, self._intr)
        uc.hook_add(UC_HOOK_MEM_INVALID, self._invalid)

    # ---- hooks ----
    def _guard(self, fn, *a):
        try:
            return fn(*a)
        except EmulationStop as e:
            self.error = str(e)
            self.uc.emu_stop()
        except Exception as e:          # keep the traceback text for the report
            import traceback
            self.error = traceback.format_exc()
            self.uc.emu_stop()
        return 0

    def _in(self, uc, port, size, ud):
        def f():
            v = self.ega.port_in(port)
            if size == 2:
                v |= self.ega.port_in(port + 1) << 8
            return v
        return self._guard(f)

    def _out(self, uc, port, size, value, ud):
        def f():
            self.ega.port_out(port, value & 0xFF)
            if size == 2:
                self.ega.port_out(port + 1, (value >> 8) & 0xFF)
        self._guard(f)

    def _intr(self, uc, intno, ud):
        def f():
            h = self.int_handlers.get(intno)
            if h is None:
                raise EmulationStop('INT %02Xh at %04X:%04X' % (intno, self.reg('cs'), self.reg('ip')))
            h(self)
        self._guard(f)

    def _invalid(self, uc, access, address, size, value, ud):
        self.error = 'invalid memory access at %05Xh' % address
        return False

    def on_int(self, n, handler):
        self.int_handlers[n] = handler

    def stop(self, reason):
        self.stop_reason = reason
        self.uc.emu_stop()

    # ---- registers and memory ----
    def reg(self, name):
        return self.uc.reg_read(REGS[name]) & 0xFFFF

    def set(self, **kw):
        for k, v in kw.items():
            self.uc.reg_write(REGS[k], v & 0xFFFF)

    @staticmethod
    def lin(seg, off):
        return ((seg & 0xFFFF) << 4) + (off & 0xFFFF)

    def read(self, addr, n):
        return bytes(self.uc.mem_read(addr, n))

    def write(self, addr, data):
        self.uc.mem_write(addr, bytes(data))

    def rw(self, addr):
        return struct.unpack('<H', self.read(addr, 2))[0]

    def ww(self, addr, v):
        self.write(addr, struct.pack('<H', v & 0xFFFF))

    def cstr(self, addr):
        s = b''
        while True:
            c = self.read(addr + len(s), 1)
            if c == b'\0' or len(s) > 128:
                return s.decode('latin-1')
            s += c

    def push(self, v):
        sp = (self.reg('sp') - 2) & 0xFFFF
        self.set(sp=sp)
        self.ww(self.lin(self.reg('ss'), sp), v)

    # ---- calling code ----
    def run(self, cs, ip, stop_cs, stop_ip, max_insns=0):
        """Run from cs:ip until stop_cs:stop_ip is reached (the pushed return address)."""
        self.error = None
        self.stop_reason = None
        self.set(cs=cs)
        try:
            self.uc.emu_start(self.lin(cs, ip), self.lin(stop_cs, stop_ip), count=max_insns)
        except UcError as e:
            self.error = self.error or ('%s at %04X:%04X' % (e, self.reg('cs'), self.reg('ip')))
        if self.error:
            raise RuntimeError(self.error)
        return self.stop_reason

    def call(self, cs, ip, args, far, ret_cs, ret_ip, regs=None, flags=0x0002):
        """Push args (args[0] last, so it is the first on the stack: C order) and a near or
        far return address, set the registers, run until the return address is reached.
        flags is the FLAGS value on entry (CF, DF clear by default)."""
        for a in reversed(args):
            self.push(a)
        if far:
            self.push(ret_cs)
        self.push(ret_ip)
        self.set(flags=flags, **(regs or {}))
        return self.run(cs, ip, ret_cs, ret_ip)
