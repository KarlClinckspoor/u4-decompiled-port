#!/usr/bin/env python3
"""Build AVATAR.EXE and TITLE.EXE with the original tools in a headless DOSBox, and check them.

    python3 tools/dosbuild.py [--tools DIR] [--dosbox PATH] [--keep DIR] [--check FILE] [--log]

The DOS build is the reference for every change to the sources: renaming, commenting and the
port's portability macros must all still give the same programs. This copies SRC/ and
SRC-TITLE/ to a scratch C: drive together with Microsoft C 4.00 and MASM 3.00, compiles and
links them in DOSBox as SRC/MAKEFILE and SRC-TITLE/DOIT.BAT do, and prints the SHA-1 of the two
linked programs (before EXEPACK) and of their link maps' public symbol addresses.
With --check (default: tools/build.sha1 when it exists) it fails unless the program hashes are
the ones recorded there.

--tools DIR holds the compiler, in any of these forms:
  - MSC400/ (MSC.EXE, C1/C2/C3.EXE, LINK.EXE, INCLUDE/, LIB/), or
  - "Microsoft C Compiler 4.0.zip" (the eight 360K floppy images), read directly;
and MASM 3.00 as MASM300.EXE, MASM.EXE, or masm300/ holding MASM.EXE or its floppy image.
"""
import argparse, hashlib, io, os, re, shutil, struct, subprocess, sys, tempfile, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)


# --- FAT12 floppy images, read without mtools ---------------------------------------------
def fat12_files(img):
    """{PATH: bytes} for every file of a FAT12 floppy image (paths upper case, '/'-separated)."""
    bps, spc, rsv, nfat, nroot = struct.unpack_from('<HBHBH', img, 11)
    spf = struct.unpack_from('<H', img, 22)[0]
    fat = img[rsv * bps:(rsv + spf) * bps]
    root_off = (rsv + nfat * spf) * bps
    data_off = root_off + nroot * 32
    csize = spc * bps

    def nxt(c):
        v = struct.unpack_from('<H', fat, c * 3 // 2)[0]
        return (v >> 4) if c & 1 else (v & 0xFFF)

    def chain(c):
        out = []
        while 2 <= c < 0xFF8:
            out.append(img[data_off + (c - 2) * csize:data_off + (c - 1) * csize])
            c = nxt(c)
        return b''.join(out)

    files = {}

    def walk(raw, prefix):
        for i in range(0, len(raw), 32):
            e = raw[i:i + 32]
            if len(e) < 32 or e[0] == 0:
                break
            if e[0] == 0xE5 or e[11] & 0x08:
                continue
            name = e[0:8].decode('latin-1').rstrip()
            ext = e[8:11].decode('latin-1').rstrip()
            full = prefix + name + ('.' + ext if ext else '')
            clus = struct.unpack_from('<H', e, 26)[0]
            size = struct.unpack_from('<I', e, 28)[0]
            if e[11] & 0x10:
                if name not in ('.', '..'):
                    walk(chain(clus), full + '/')
            else:
                files[full] = chain(clus)[:size]

    walk(img[root_off:data_off], '')
    return files


def install_msc(tools, dst):
    """Put MSC 4.00 into dst (MSC.EXE ..., INCLUDE/, LIB/)."""
    for name in os.listdir(tools):
        if name.upper() == 'MSC400' and os.path.isdir(os.path.join(tools, name)):
            shutil.copytree(os.path.join(tools, name), dst)
            return
    zips = [n for n in os.listdir(tools) if n.lower().endswith('.zip') and 'c compiler 4' in n.lower()]
    if not zips:
        sys.exit('no MSC400/ or "Microsoft C Compiler 4.0.zip" in %s' % tools)
    os.makedirs(os.path.join(dst, 'INCLUDE', 'SYS'))
    os.makedirs(os.path.join(dst, 'LIB'))
    with zipfile.ZipFile(os.path.join(tools, zips[0])) as z:
        for member in sorted(z.namelist()):
            if not member.lower().endswith('.img'):
                continue
            for path, data in fat12_files(z.read(member)).items():
                base = path.split('/')[-1]
                if path.endswith('.H'):
                    out = os.path.join(dst, 'INCLUDE', *path.split('/'))
                elif base.endswith('.LIB') or (base.endswith('.OBJ') and '/' not in path):
                    out = os.path.join(dst, 'LIB', base)
                elif base.endswith('.EXE') and '/' not in path:
                    out = os.path.join(dst, base)
                else:
                    continue
                if not os.path.exists(out):
                    os.makedirs(os.path.dirname(out), exist_ok=True)
                    open(out, 'wb').write(data)


def install_masm(tools, dst):
    cands = [os.path.join(tools, n) for n in ('MASM300.EXE', 'masm300.exe', 'MASM.EXE', 'masm.exe')]
    for d in ('masm300', 'MASM300'):
        p = os.path.join(tools, d)
        if os.path.isdir(p):
            cands += [os.path.join(p, n) for n in os.listdir(p)]
    for c in cands:
        if os.path.isfile(c) and os.path.basename(c).upper() in ('MASM300.EXE', 'MASM.EXE'):
            shutil.copy(c, dst)
            return
    for c in cands:
        if os.path.isfile(c) and c.lower().endswith('.img'):
            files = fat12_files(open(c, 'rb').read())
            if 'MASM.EXE' in files:
                open(dst, 'wb').write(files['MASM.EXE'])
                return
    sys.exit('no MASM 3.00 (MASM300.EXE, masm300/MASM.EXE or its floppy image) in %s' % tools)


# --- the build steps ------------------------------------------------------------------------
def avatar_steps(src):
    """(commands, link) from SRC/MAKEFILE: one MSC or MASM command per object, in its order."""
    text = open(os.path.join(src, 'MAKEFILE'), encoding='latin-1').read().replace('\r', '')
    cflags = re.search(r'^CFLAGS=(.*)$', text, re.M).group(1).strip()
    cmds = []
    lines = text.split('\n')
    for i, line in enumerate(lines):
        m = re.match(r'^(\w+)\.obj:\s*(\w+)\.(c|asm)\s*$', line, re.I)
        if not m:
            continue
        cmd = lines[i + 1].strip()
        cmd = cmd.replace('$(CFLAGS)', cflags).replace('$(ASM)', r'\TOOLS\MASM300 /Mx')
        cmd = re.sub(r'\s+', ' ', cmd)
        # DOS truncates file names to 8 characters; say so explicitly
        cmd = re.sub(r'(\w{9,});$', lambda mm: mm.group(1)[:8] + ';', cmd)
        cmds.append(cmd.replace('msc ', 'MSC '))
    return cmds, r'LINK /MAP @U4.RSP'


def title_steps(src):
    text = open(os.path.join(src, 'DOIT.BAT'), encoding='latin-1').read().replace('\r', '')
    cflags = re.search(r'^set CFLAGS=(.*)$', text, re.M | re.I).group(1).strip()
    cmds, link = [], None
    for line in text.split('\n'):
        line = line.strip()
        up = line.upper()
        if up.startswith('MSC '):
            cmds.append(line.split('>>')[0].replace('%CFLAGS%', cflags).strip())
        elif up.startswith('%AS%'):
            cmds.append(line.split('>>')[0].replace('%AS%', r'\TOOLS\MASM300').strip())
        elif up.startswith('LINK '):
            link = line.split('>>')[0].replace('%LIB%', r'\TOOLS\MSC400\LIB').strip()
    return cmds, link


def map_symbols(path):
    """The 'Publics by Value' part of a LINK map: [(address, name)]."""
    if not os.path.exists(path):
        return []
    out, on = [], False
    for line in open(path, encoding='latin-1'):
        if 'Publics by Value' in line:
            on = True
            continue
        if on:
            m = re.match(r'\s*([0-9A-F]{4}:[0-9A-F]{4})\s+(?:Abs\s+)?(\S+)', line)
            if m:
                out.append((m.group(1), m.group(2)))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default=os.path.join(ROOT, 'SRC'))
    ap.add_argument('--title-src', default=os.path.join(ROOT, 'SRC-TITLE'))
    ap.add_argument('--tools', default=os.path.join(ROOT, 'tools'))
    ap.add_argument('--dosbox', default=shutil.which('dosbox-x') or shutil.which('dosbox') or 'dosbox')
    ap.add_argument('--keep', help='copy the linked programs and maps here')
    ap.add_argument('--check', help='file of expected hashes (default: tools/build.sha1)')
    ap.add_argument('--write-check', action='store_true', help='record the hashes in tools/build.sha1')
    ap.add_argument('--log', action='store_true', help='print the build log')
    a = ap.parse_args()

    with tempfile.TemporaryDirectory() as tmp:
        c = os.path.join(tmp, 'c')
        os.makedirs(os.path.join(c, 'TOOLS'))
        install_msc(a.tools, os.path.join(c, 'TOOLS', 'MSC400'))
        install_masm(a.tools, os.path.join(c, 'TOOLS', 'MASM300.EXE'))
        ign = shutil.ignore_patterns('*.OBJ', '*.obj', '*.EXE', '*.exe', '*.MAP', '*.map', '*.md')
        shutil.copytree(a.src, os.path.join(c, 'A'), ignore=ign)
        shutil.copytree(a.title_src, os.path.join(c, 'T'), ignore=ign)
        acmds, alink = avatar_steps(a.src)
        tcmds, tlink = title_steps(a.title_src)
        lines = ['@echo off', r'set PATH=\TOOLS\MSC400', r'set INCLUDE=\TOOLS\MSC400\INCLUDE',
                 r'set LIB=\TOOLS\MSC400\LIB', r'cd \A']
        lines += ['%s >> \\BUILD.LOG' % x for x in acmds + [alink]]
        lines.append(r'cd \T')
        lines += ['%s >> \\BUILD.LOG' % x for x in tcmds + [tlink]]
        open(os.path.join(c, 'BUILD.BAT'), 'w', newline='').write('\r\n'.join(lines) + '\r\n')
        conf = os.path.join(tmp, 'dosbox.conf')
        open(conf, 'w').write('[sdl]\noutput=surface\n[dosbox]\nmemsize=16\n[cpu]\ncycles=max\n'
                              '[autoexec]\nmount c "%s"\nc:\ncall \\BUILD.BAT\nexit\n' % c)
        env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        cmd = ([a.dosbox, '-conf', conf, '-noconsole'] if 'dosbox-x' not in os.path.basename(a.dosbox).lower()
               else [a.dosbox, '-conf', conf, '-nogui', '-exit'])
        subprocess.run(cmd, env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=1800)
        logp = os.path.join(c, 'BUILD.LOG')
        log = open(logp, encoding='latin-1').read() if os.path.exists(logp) else ''
        if a.log:
            print(log)

        def find(d, name):
            for n in os.listdir(d):
                if n.upper() == name:
                    return os.path.join(d, n)
            return None

        results = {}
        for label, d, exe, mp in (('AVATAR', 'A', 'U4_MAIN.EXE', 'U4_MAIN.MAP'),
                                  ('TITLE', 'T', 'TITLE_0.EXE', 'TITLE_0.MAP')):
            e = find(os.path.join(c, d), exe)
            if not e:
                print(log)
                sys.exit('%s: %s was not built' % (label, exe))
            data = open(e, 'rb').read()
            syms = map_symbols(find(os.path.join(c, d), mp) or '')
            results[label] = hashlib.sha1(data).hexdigest()
            print('%-6s %s  %6d bytes  (%d public symbols)' % (label, results[label], len(data), len(syms)))
            if a.keep:
                os.makedirs(a.keep, exist_ok=True)
                shutil.copy(e, os.path.join(a.keep, exe))
                m = find(os.path.join(c, d), mp)
                if m:
                    shutil.copy(m, os.path.join(a.keep, mp))
        errs = [l for l in log.splitlines() if re.search(r'\berror\b|E r r o r', l, re.I)]
        if errs:
            print('build messages:\n  ' + '\n  '.join(errs[:40]))

    chk = a.check or os.path.join(HERE, 'build.sha1')
    if a.write_check:
        with open(os.path.join(HERE, 'build.sha1'), 'w', newline='\n') as f:
            for k in ('AVATAR', 'TITLE'):
                f.write('%s %s\n' % (results[k], k))
        print('recorded in tools/build.sha1')
    elif os.path.exists(chk):
        want = dict(reversed(l.split()) for l in open(chk) if l.strip())
        bad = [k for k in results if want.get(k) and want[k] != results[k]]
        if bad:
            sys.exit('DIFFERENT from %s: %s' % (os.path.relpath(chk), ', '.join(bad)))
        print('identical to %s' % os.path.relpath(chk))


if __name__ == '__main__':
    main()
