#!/usr/bin/env python3
"""Unpack a Microsoft EXEPACK'ed DOS program, and compare DOS programs by their load image.

    python3 tools/unexepack.py IN.EXE OUT.EXE        unpack
    python3 tools/unexepack.py --compare A.EXE B.EXE  compare load images (packed or not)

The original AVATAR.EXE is packed with EXEPACK; the build of SRC/ is compared with it
after unpacking both into (load image, relocations, entry point, stack). GOG's AVATAR.EXE
enters through a small stub that sets SS:SP and jumps to the EXEPACK unpacker elsewhere in
the image (mov ax,ds / mov es,ax / add ax,K1 / mov ss,ax / mov sp,N / sub ax,K2 / push ax /
mov ax,IP / push ax / ... / retf); unpack() follows it. Relocations are compared by linear
address, since EXEPACK stores them as 1000h-paragraph frames, and trailing zeros (the
uninitialised data EXEPACK drops) are reported apart.
"""
import struct, sys

TRAMPOLINE = bytes.fromhex('8cd88ec005')


def follow_trampoline(data, e):
    """If the entry point is GOG's jump stub, return the header CS:IP it jumps to, else None."""
    s = e['image'][e['cs'] * 16 + e['ip']:]
    if not s.startswith(TRAMPOLINE) or s[7:9] != b'\x8e\xd0' or s[12] != 0x2d or s[16] != 0xb8:
        return None
    k1, k2, ip = (struct.unpack_from('<H', s, o)[0] for o in (5, 13, 17))
    return (k1 - k2 - 0x10) & 0xffff, ip   # DS = PSP; the image starts at PSP+10h


def parse(data):
    h = struct.unpack_from('<14H', data, 0)
    (sig, last, pages, nrel, hdrpar, minalloc, maxalloc, ss, sp, csum, ip, cs, reloff, ovl) = h
    size = pages * 512 - ((512 - last) if last else 0)
    image = data[hdrpar * 16:size]
    rels = [struct.unpack_from('<HH', data, reloff + 4 * i) for i in range(nrel)]
    return dict(image=image, rels=sorted((s, o) for o, s in rels), ss=ss, sp=sp, cs=cs, ip=ip,
                minalloc=minalloc, maxalloc=maxalloc)


def unpack(data):
    e = parse(data)
    t = follow_trampoline(data, e)
    if t:
        e['cs'], e['ip'] = t
    img = e['image']
    hdr = img[e['cs'] * 16:]
    r_ip, r_cs, _mem, exe_size, r_sp, r_ss, dest_len = struct.unpack_from('<7H', hdr, 0)
    if hdr[16:18] == b'RB':
        hlen = 18
    elif hdr[14:16] == b'RB':
        hlen = 16
    else:
        raise ValueError('not EXEPACK')
    packed = bytearray(img[:e['cs'] * 16])
    out = bytearray(dest_len * 16)
    out[:len(packed)] = packed
    src = len(packed) - 1
    while packed[src] == 0xFF:
        src -= 1
    dst = len(out) - 1
    while True:
        cmd = packed[src]; src -= 1
        n = packed[src] << 8 | packed[src - 1]; src -= 2
        if cmd & 0xFE == 0xB0:
            b = packed[src]; src -= 1
            for _ in range(n):
                out[dst] = b; dst -= 1
        elif cmd & 0xFE == 0xB2:
            for _ in range(n):
                out[dst] = packed[src]; dst -= 1; src -= 1
        else:
            raise ValueError('bad EXEPACK command %02x' % cmd)
        if cmd & 1:
            break
    # relocations follow the error message
    msg = hdr.find(b'Packed file is corrupt')
    p = msg + len(b'Packed file is corrupt')
    rels = []
    for seg in range(16):
        cnt = struct.unpack_from('<H', hdr, p)[0]; p += 2
        for i in range(cnt):
            off = struct.unpack_from('<H', hdr, p)[0]; p += 2
            rels.append((seg * 0x1000, off))
    image = bytes(out[:len(out) - (len(out) - dst - 1 if False else 0)])
    return dict(image=bytes(out), rels=sorted(rels), ss=r_ss, sp=r_sp, cs=r_cs, ip=r_ip,
                minalloc=e['minalloc'], maxalloc=e['maxalloc'])


def load(path):
    data = open(path, 'rb').read()
    try:
        return unpack(data)
    except (ValueError, IndexError, struct.error):
        return parse(data)


def write_exe(e, path):
    rels = e['rels']
    hdr_size = 0x1c + 4 * len(rels)
    hdrpar = (hdr_size + 15) // 16
    body = e['image']
    total = hdrpar * 16 + len(body)
    h = struct.pack('<14H', 0x5a4d, total % 512, (total + 511) // 512, len(rels), hdrpar,
                    e['minalloc'], e['maxalloc'], e['ss'], e['sp'], 0, e['ip'], e['cs'], 0x1c, 0)
    h += b''.join(struct.pack('<HH', o, s) for s, o in rels)
    h += b'\0' * (hdrpar * 16 - len(h))
    open(path, 'wb').write(h + body)


def compare(a, b):
    A, B = load(a), load(b)
    ia, ib = A['image'], B['image']
    n = min(len(ia), len(ib))
    diffs = [i for i in range(n) if ia[i] != ib[i]]
    print('image sizes %d / %d, %d differing bytes in common part' % (len(ia), len(ib), len(diffs)))
    tail = ia[n:] or ib[n:]
    zero_tail = not tail.strip(b'\0')
    if tail:
        print('  the longer image has %d more bytes, %s' % (len(tail), 'all zero' if zero_tail else 'not all zero'))
    runs = []
    for i in diffs:
        if runs and i - runs[-1][1] <= 4:
            runs[-1][1] = i
        else:
            runs.append([i, i])
    for s, e in runs[:40]:
        print('  %05X-%05X  %s | %s' % (s, e, ia[s:e + 1].hex(), ib[s:e + 1].hex()))
    for k in ('cs', 'ip', 'ss', 'sp'):
        if A[k] != B[k]:
            print('  %s %04X / %04X' % (k, A[k], B[k]))
    lin = lambda rels: sorted(s * 16 + o for s, o in rels)
    same_rels = lin(A['rels']) == lin(B['rels'])
    if not same_rels:
        print('  relocations differ: %d / %d' % (len(A['rels']), len(B['rels'])))
    same_regs = all(A[k] == B[k] for k in ('cs', 'ip', 'ss', 'sp'))
    ok = not diffs and zero_tail and same_rels and same_regs
    print('same load image%s' % (' (but for trailing zeros)' if tail else '') if ok else 'DIFFERENT')
    return ok


if __name__ == '__main__':
    if sys.argv[1] == '--compare':
        sys.exit(0 if compare(sys.argv[2], sys.argv[3]) else 1)
    write_exe(unpack(open(sys.argv[1], 'rb').read()), sys.argv[2])
