#!/usr/bin/env python3
"""Turn the linked programs into the files GOG ships, byte for byte, and check them.

    python3 tools/postlink.py --gamedata DIR [--build DIR] [--out DIR]

--build DIR holds what `tools/dosbuild.py --keep DIR` leaves: U4_MAIN.EXE and TITLE_0.EXE with
their link maps. --gamedata DIR holds the shipped AVATAR.EXE and TITLE.EXE. The rebuilt files
go to --out DIR (default: --build DIR) and are compared with the shipped ones by SHA-1.

What happened to the programs after LINK, as far as the shipped files tell:

1. They were linked without the communal variables (segment C_COMMON, all zeros) in the
   file: the load image stops at the end of the segment before it, and the header asks for
   that much more memory instead. LINK 3.51 writes C_COMMON out, so the build is longer.
2. Both were packed with Microsoft EXEPACK (exepack() below reimplements it: run-length
   coding from the end of the image, an 18-byte header, the unpacker, the relocations
   grouped by 64K frame).
3. TITLE.EXE was then unpacked again with UNP (see SRC-TITLE/DOIT.BAT): its header is
   UNP's (relocations as EXEPACK stored them, no checksum, 3 paragraphs) and keeps the
   memory request of the packed file. Its AskForDisk was patched to return at once: a
   near jump over its body to its final RET (`%%% hacked %%%` in SRC-TITLE/TITLE_1.C).
4. GOG's AVATAR.EXE has a stub appended after the EXEPACK unpacker. The header enters
   it; it sets the stack and jumps to the unpacker, as EXEPACK's header would have.

None of the shipped bytes are in this repository. The parts this script cannot derive from
the build are copied from --gamedata when it runs: EXEPACK's unpacker code and message, the
13 bytes that pad its relocation table to a paragraph (text left over in EXEPACK's buffer),
and GOG's stub with the stack its header gives it. TITLE.EXE needs the unpacker's length too,
for the memory request, and takes it from the shipped AVATAR.EXE.
"""
import argparse, hashlib, os, re, struct, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import unexepack

EXEPACK_HEADER = 18          # with skip_len, ending in 'RB'
EXEPACK_STACK = 0x80
MESSAGE = b'Packed file is corrupt'


# --- the inputs -------------------------------------------------------------------------------
def read_exe(data):
    h = struct.unpack_from('<14H', data, 0)
    (sig, last, pages, nrel, hdrpar, minalloc, maxalloc, ss, sp, csum, ip, cs, reloff, ovl) = h
    size = pages * 512 - ((512 - last) if last else 0)
    rels = [struct.unpack_from('<HH', data, reloff + 4 * i) for i in range(nrel)]   # (off, seg)
    return dict(header=data[:hdrpar * 16], image=data[hdrpar * 16:size], rels=rels,
                minalloc=minalloc, maxalloc=maxalloc, ss=ss, sp=sp, cs=cs, ip=ip, reloff=reloff)


def read_map(path):
    """Segments [(start, length, name)] and public symbols {NAME: linear address} of a map."""
    segs, syms = [], {}
    for line in open(path, encoding='latin-1'):
        m = re.match(r'\s*([0-9A-F]{5})H [0-9A-F]{5}H ([0-9A-F]{5})H (\S+)', line)
        if m:
            segs.append((int(m.group(1), 16), int(m.group(2), 16), m.group(3)))
            continue
        m = re.match(r'\s*([0-9A-F]{4}):([0-9A-F]{4})\s+(\S+)\s*$', line)
        if m:
            syms[m.group(3).lstrip('_').upper()] = int(m.group(1), 16) * 16 + int(m.group(2), 16)
    return segs, syms


def original_layout(segs):
    """(image_end, memory_end) of the program as shipped: the file stops before C_COMMON."""
    common = [s for s, n, name in segs if name == 'C_COMMON']
    if not common:
        sys.exit('no C_COMMON segment in the map')
    image_end = max(s + n for s, n, name in segs if s < common[0])
    memory_end = max(s + n for s, n, name in segs)
    return image_end, memory_end


def paras(n):
    return (n + 15) // 16


# --- EXEPACK -----------------------------------------------------------------------------------
def exepack_compress(data):
    """EXEPACK's coding of data, which it unpacks backwards from its end.

    Every run of 8 or more equal bytes becomes a fill (value, length, B0h), everything between
    is copied (bytes, length, B2h); the command of the first block has bit 0 set. This greedy
    rule gives the shipped AVATAR.EXE exactly. Blocks longer than FFFFh bytes do not occur in
    this game, and how EXEPACK splits them is not known, so they are refused.
    """
    blocks, pos, i, n = [], 0, 0, len(data)
    while i < n:
        j = i
        while j < n and data[j] == data[i]:
            j += 1
        if j - i >= 8:
            if i > pos:
                blocks.append((0xB2, data[pos:i]))
            blocks.append((0xB0, data[i:j]))
            pos = j
        i = j
    if pos < n:
        blocks.append((0xB2, data[pos:]))
    out = bytearray()
    for k, (cmd, chunk) in enumerate(blocks):
        if len(chunk) > 0xFFFF:
            raise ValueError('EXEPACK block of %d bytes: splitting is not implemented' % len(chunk))
        out += chunk if cmd == 0xB2 else chunk[:1]
        out += struct.pack('<HB', len(chunk), cmd | (1 if k == 0 else 0))
    return bytes(out)


def exepack_relocations(rels):
    """EXEPACK's relocation table: for each 64K frame, a count and the offsets in it."""
    frames = [[] for _ in range(16)]
    for off, seg in rels:
        lin = seg * 16 + off
        frames[lin >> 16].append(lin & 0xFFFF)
    return b''.join(struct.pack('<H', len(f)) + b''.join(struct.pack('<H', o) for o in f)
                    for f in frames), frames


def exepack(exe, image_end, memory_end, unpacker, pad):
    """EXEPACK of a linked program. unpacker: the code and message that follow the header;
    pad: the bytes that fill the last paragraph of the EXEPACK block (EXEPACK leaves its
    buffer there). Returns (file, info)."""
    dest = exe['image'][:image_end]
    dest += b'\0' * (paras(len(dest)) * 16 - len(dest))
    body = exepack_compress(dest)
    body += b'\xff' * (-len(body) % 16)
    table, _ = exepack_relocations(exe['rels'])
    size = EXEPACK_HEADER + len(unpacker) + len(table)
    head = struct.pack('<7H', exe['ip'], exe['cs'], 0, size, exe['sp'], exe['ss'], len(dest) // 16)
    head += struct.pack('<H', 1) + b'RB'             # skip_len 1: the block follows the data
    block = head + unpacker + table
    want = paras(len(block)) * 16 - len(block)
    block += pad[:want] + b'\0' * (want - len(pad[:want]))
    image = body + block
    cs = len(body) // 16
    ss = len(dest) // 16 + paras(size)                # the unpacker moves itself after dest
    total = max(paras(image_end) + paras(memory_end - image_end), ss + EXEPACK_STACK // 16)
    minalloc = total - len(image) // 16
    header = bytearray(exe['header'][:0x1E]) + bytes(len(exe['header']) - 0x1E)
    fields = dict(nrel=0, minalloc=minalloc, ss=ss, sp=EXEPACK_STACK, ip=EXEPACK_HEADER, cs=cs)
    return finish(header, image, exe['maxalloc'], fields), dict(fields, body=len(body), block=len(block))


def finish(header, image, maxalloc, f):
    """header (its size, relocation offset and anything after 1Ch kept) with the given fields,
    the file size and a zero checksum, followed by image."""
    total = len(header) + len(image)
    reloff = struct.unpack_from('<H', header, 0x18)[0]
    struct.pack_into('<13H', header, 2, total % 512, (total + 511) // 512, f['nrel'], len(header) // 16,
                     f['minalloc'], maxalloc, f['ss'], f['sp'], 0, f['ip'], f['cs'], reloff, 0)
    return bytes(header) + image


# --- the shipped envelopes ---------------------------------------------------------------------
def gog_avatar_parts(gog):
    """From GOG's AVATAR.EXE: the unpacker code and message, the pad of the EXEPACK block,
    GOG's stub (from the entry point to the end) and the stack its header gives it."""
    g = read_exe(gog)
    e = unexepack.parse(gog)
    t = unexepack.follow_trampoline(gog, e)
    if not t:
        sys.exit('GOG AVATAR.EXE: no jump stub at the entry point')
    blk = g['image'][t[0] * 16:]
    if blk[16:18] != b'RB':
        sys.exit('GOG AVATAR.EXE: no EXEPACK header where the stub jumps')
    size = struct.unpack_from('<H', blk, 6)[0]
    m = blk.find(MESSAGE) + len(MESSAGE)
    unpacker = blk[EXEPACK_HEADER:m]
    pad = blk[size:paras(size) * 16]
    stub_at = g['cs'] * 16 + g['ip']
    stub = g['image'][stub_at:]
    k1, sp = struct.unpack_from('<H', stub, 5)[0], struct.unpack_from('<H', stub, 10)[0]
    return dict(unpacker=unpacker, pad=pad, stub=stub, stub_at=stub_at, ss=g['ss'], sp=g['sp'],
                jump=t + ((k1 - 0x10) & 0xFFFF, sp))     # the CS:IP and SS:SP it sets


def avatar(build_exe, build_map, gog):
    exe = read_exe(build_exe)
    image_end, memory_end = original_layout(read_map(build_map)[0])
    parts = gog_avatar_parts(gog)
    packed, info = exepack(exe, image_end, memory_end, parts['unpacker'], parts['pad'])
    p = read_exe(packed)
    mine = (info['cs'], info['ip'], info['ss'], info['sp'])
    if parts['jump'] != mine or parts['stub_at'] != len(p['image']):
        sys.exit('GOG stub does not fit this build: it starts at %X and sets CS:IP %04X:%04X, '
                 'SS:SP %04X:%04X; EXEPACK ends at %X with %04X:%04X, %04X:%04X'
                 % ((parts['stub_at'],) + parts['jump'] + (len(p['image']),) + mine))
    header = bytearray(p['header'])
    image = p['image'] + parts['stub']
    fields = dict(nrel=0, minalloc=p['minalloc'], ss=parts['ss'], sp=parts['sp'], ip=0,
                  cs=len(p['image']) // 16)
    return finish(header, image, p['maxalloc'], fields), packed


def title(build_exe, build_map, gog_avatar):
    exe = read_exe(build_exe)
    segs, syms = read_map(build_map)
    image_end, memory_end = original_layout(segs)
    image = bytearray(exe['image'][:image_end])
    image += bytes(paras(len(image)) * 16 - len(image))
    # AskForDisk returns at once: jump to its final RET (the next function follows it)
    start, nxt = syms['ASKFORDISK'], syms['WRITEPLAYERDISK']
    if image[nxt - 3] == 0xC2:
        ret = nxt - 3
    elif image[nxt - 1] == 0xC3:
        ret = nxt - 1
    else:
        sys.exit('TITLE: AskForDisk does not end with RET')
    image[start:start + 3] = b'\xe9' + struct.pack('<h', ret - (start + 3))
    # the memory request of the EXEPACKed file, which UNP keeps
    exe2 = dict(exe, image=bytes(image))
    parts = gog_avatar_parts(gog_avatar)
    packed, _ = exepack(exe2, image_end, memory_end, parts['unpacker'], parts['pad'])
    minalloc = read_exe(packed)['minalloc']
    # UNP's header: relocations in EXEPACK's order and frames, padded to a paragraph
    _, frames = exepack_relocations(exe['rels'])
    rels = [(o, f << 12) for f in range(16) for o in frames[f]]
    header = bytearray(struct.pack('<14H', 0x5A4D, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1C, 0))
    header += b''.join(struct.pack('<HH', o, s) for o, s in rels)
    header += bytes(-len(header) % 16)
    fields = dict(nrel=len(rels), minalloc=minalloc, ss=exe['ss'], sp=exe['sp'], ip=exe['ip'], cs=exe['cs'])
    return finish(header, bytes(image), exe['maxalloc'], fields)


def run(build, gamedata, out):
    """Write AVATAR.EXE and TITLE.EXE to out.
    Returns {name: (sha1, same as the shipped file, size, size of the shipped file)}."""
    gog_av = open(os.path.join(gamedata, 'AVATAR.EXE'), 'rb').read()
    res = {}
    av, _ = avatar(open(os.path.join(build, 'U4_MAIN.EXE'), 'rb').read(),
                   os.path.join(build, 'U4_MAIN.MAP'), gog_av)
    ti = title(open(os.path.join(build, 'TITLE_0.EXE'), 'rb').read(),
               os.path.join(build, 'TITLE_0.MAP'), gog_av)
    os.makedirs(out, exist_ok=True)
    for name, data in (('AVATAR.EXE', av), ('TITLE.EXE', ti)):
        open(os.path.join(out, name), 'wb').write(data)
        ref = open(os.path.join(gamedata, name), 'rb').read()
        res[name] = (hashlib.sha1(data).hexdigest(), data == ref, len(data), len(ref))
    return res


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--gamedata', required=True, help='directory with the shipped AVATAR.EXE, TITLE.EXE')
    ap.add_argument('--build', default='.', help='U4_MAIN.EXE/.MAP and TITLE_0.EXE/.MAP from dosbuild --keep')
    ap.add_argument('--out', help='where to write AVATAR.EXE and TITLE.EXE (default: --build)')
    a = ap.parse_args()
    res = run(a.build, a.gamedata, a.out or a.build)
    ok = True
    for name, (sha, same, n, m) in res.items():
        print('%-10s %s  %6d bytes  %s' % (name, sha, n, 'identical to the shipped file' if same
                                           else 'DIFFERENT from the shipped file (%d bytes)' % m))
        ok &= same
    sys.exit(0 if ok else 1)


if __name__ == '__main__':
    main()
