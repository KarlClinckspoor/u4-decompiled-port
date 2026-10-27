#!/usr/bin/env python3
"""Unpack a Microsoft EXEPACK'ed DOS program, and compare DOS programs by their load image.

    python3 tools/unexepack.py IN.EXE OUT.EXE        unpack
    python3 tools/unexepack.py --compare A.EXE B.EXE  compare load images (packed or not)

The original AVATAR.EXE is packed with EXEPACK; the build of SRC/ is compared with it
after unpacking both into (load image, relocations, entry point, stack).
"""
import struct, sys


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
    if A['rels'] != B['rels']:
        print('  relocations differ: %d / %d' % (len(A['rels']), len(B['rels'])))
    return not diffs and len(ia) == len(ib)


if __name__ == '__main__':
    if sys.argv[1] == '--compare':
        sys.exit(0 if compare(sys.argv[2], sys.argv[3]) else 1)
    write_exe(unpack(open(sys.argv[1], 'rb').read()), sys.argv[2])
