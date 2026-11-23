#!/usr/bin/env python3
"""progress.py: progress card (docs/progress.svg) and numbers (docs/progress.json).

The sources rebuild AVATAR.EXE and TITLE.EXE with Microsoft C 4.00 (tools/dosbuild.py checks them
against tools/build.sha1; ergonomy_joe's own estimate is a few bytes of difference from the
shipped files). One tile is one function of the sources:

  - a C function of SRC/ (AVATAR.EXE), SRC-TITLE/ (TITLE.EXE) or SRC-DRV/ (the CGA driver): the
    definition that starts with the address marker `/*C_xxxx*/` of its original address, and
  - an assembly procedure (`NAME PROC`) of the same folders,

in the order AVATAR, TITLE, driver, then by original address, and its status is:

  matched  the function has a source and a real name (not a placeholder such as C_2F7E)
  attempt  reserved (a function being worked on)
  open     the function still carries an address placeholder as its name
  aside    reserved for code that is not Ultima IV's own: none (the Microsoft C library is linked
           from the compiler's own files and is not in the repository)

The percentage is matched / (matched + attempt + open). The script only reads the sources; it does
not rebuild anything or compare bytes with the shipped files.

    python3 tools/progress.py            # writes docs/progress.svg and docs/progress.json
    python3 tools/progress.py --check    # exit 1 if the committed files are stale
"""
import argparse, glob, json, os, re, sys
from html import escape

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
NAME = 'u4-decompiled-port'
PROGRAMS = [('AVATAR', 'SRC'), ('TITLE', 'SRC-TITLE'), ('DRIVER', 'SRC-DRV')]
CAPTION = ('Function map: 1 function per tile (C functions and assembly procedures), in order '
           'AVATAR, TITLE, CGA driver, then by original address')
COLORS = dict(matched='#2a8c3c', attempt='#b8860b', open='#64748b', aside='#b94a53')
LABELS = dict(matched='Matched', attempt='Attempt', open='Open', aside='Aside')
PLACEHOLDER = re.compile(r'^_?[CDf]_[0-9A-Fa-f]{3,4}$')      # the old address names
MARKER = re.compile(r'/\*C_([0-9A-Fa-f]{4})\*/[ \t]*((?:(?:unsigned|static|register|struct|char|int|long|short|void)\b[ \t*]*)*)\**[ \t]*(\w+)[ \t]*\(')
PROC = re.compile(r'^(\w+)[ \t]+PROC\b[^;\n]*(?:;[ \t]*C_([0-9A-Fa-f]{4}))?', re.M | re.I)


def read(path):
    with open(path, encoding='latin1', newline='') as f:
        return f.read()


def strip_comments(text):
    """Blank every comment but keep the /*C_xxxx*/ markers."""
    def keep(m):
        s = m.group(0)
        return s if re.fullmatch(r'/\*C_[0-9A-Fa-f]{4}\*/', s) else re.sub(r'[^\n]', ' ', s)
    return re.sub(r'/\*.*?\*/', keep, text, flags=re.S)


def c_functions(path):
    """(name, address) of the functions defined in a C file: marker, name, arguments, then a body."""
    text = strip_comments(read(path))
    out = []
    for m in MARKER.finditer(text):
        i, depth = m.end(), 1                       # find the closing parenthesis of the arguments
        while i < len(text) and depth:
            depth += {'(': 1, ')': -1}.get(text[i], 0)
            i += 1
        rest = text[i:]
        if re.match(r'\s*;', rest):                 # a declaration
            continue
        if re.match(r'\s*(?:[^;{}]*;\s*)*\{', rest):   # K&R argument declarations, then the body
            out.append((m.group(3), int(m.group(1), 16)))
    return out


def asm_procs(path):
    out = []
    for m in PROC.finditer(read(path)):
        name, addr = m.group(1), m.group(2)
        if addr is None and PLACEHOLDER.match(name):      # still named by its address
            addr = name[-4:]
        out.append((name, int(addr, 16) if addr else 0))
    return out


def classify():
    flat = []
    for label, folder in PROGRAMS:
        rows = []
        base = os.path.join(ROOT, folder)
        for path in sorted(glob.glob(os.path.join(base, '*'))):
            ext = path.upper().rsplit('.', 1)[-1]
            if ext == 'C':
                fs = c_functions(path)
            elif ext == 'ASM':
                fs = asm_procs(path)
            else:
                continue
            for name, addr in fs:
                rows.append(dict(program=label, file=os.path.basename(path), addr=addr, name=name,
                                 status='open' if PLACEHOLDER.match(name) else 'matched'))
        flat += sorted(rows, key=lambda r: (r['addr'], r['file'], r['name']))
    return flat


def summary(rows):
    c = {k: 0 for k in COLORS}
    for r in rows:
        c[r['status']] += 1
    total = c['matched'] + c['attempt'] + c['open']
    return dict(c, total=total, percent=round(100.0 * c['matched'] / total, 1) if total else 100.0)


def svg(flat):
    s = summary(flat)
    n = len(flat)
    W, PAD = 840, 20
    inner = W - 2 * PAD
    cols = 1
    for cols_try in range(20, 200):      # square tiles of at least 11 px
        if inner / cols_try < 11:
            break
        cols = cols_try
    cell = inner / cols
    top = 160
    H = int(top + -(-n // cols) * cell + 22)
    out = []
    a = out.append
    a('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d" '
      'font-family="Roboto, Helvetica, Arial, sans-serif" role="img" aria-label="%s">'
      % (W, H, W, H, escape('Decompilation progress: %d of %d functions matched' % (s['matched'], s['total']))))
    a('<rect width="%d" height="%d" rx="14" fill="#0f172a"/>' % (W, H))
    a('<text x="%d" y="38" font-size="21" fill="#ffffff">Decompilation progress</text>' % PAD)
    a('<text x="%d" y="38" font-size="15" fill="#ffffff" text-anchor="end">%s</text>' % (W - PAD, escape(NAME)))
    a('<text x="%d" y="62" font-size="15" fill="#e5e7eb">%s / %s functions matched (%.1f%%)</text>'
      % (PAD, format(s['matched'], ','), format(s['total'], ','), s['percent']))
    a('<rect x="%d" y="74" width="%d" height="18" rx="5" fill="#334155"/>' % (PAD, inner))
    if s['total']:
        a('<rect x="%d" y="74" width="%.2f" height="18" rx="5" fill="%s"/>'
          % (PAD, inner * s['matched'] / s['total'], COLORS['matched']))
    for x, k in zip([PAD, 232, 430, 628], ['matched', 'attempt', 'open', 'aside']):
        a('<rect x="%d" y="104" width="10" height="10" fill="%s"/>' % (x, COLORS[k]))
        a('<text x="%d" y="113" font-size="12" fill="#e5e7eb">%s: %s</text>' % (x + 14, LABELS[k], format(s[k], ',')))
    a('<text x="%d" y="140" font-size="11" fill="#e5e7eb">%s</text>' % (PAD, escape(CAPTION)))
    for i, r in enumerate(flat):
        x, y = PAD + (i % cols) * cell, top + (i // cols) * cell
        a('<rect x="%.2f" y="%.2f" width="%.2f" height="%.2f" fill="%s" stroke="#0f172a" stroke-width="1">'
          '<title>%s %s C_%04X %s (%s)</title></rect>'
          % (x, y, cell, cell, COLORS[r['status']], r['program'], r['file'], r['addr'], escape(r['name']), r['status']))
    a('</svg>')
    return '\n'.join(out) + '\n'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--check', action='store_true')
    args = ap.parse_args()
    flat = classify()
    result = {label: summary([r for r in flat if r['program'] == label]) for label, _ in PROGRAMS}
    result['all'] = summary(flat)
    js = json.dumps(result, indent=1) + '\n'
    paths = {os.path.join(ROOT, 'docs', 'progress.json'): js, os.path.join(ROOT, 'docs', 'progress.svg'): svg(flat)}
    if args.check:
        stale = [p for p, t in paths.items() if not os.path.exists(p) or read(p) != t]
        if stale:
            print('stale:', *stale)
            sys.exit(1)
        return
    for p, t in paths.items():
        with open(p, 'w', encoding='utf-8', newline='\n') as f:
            f.write(t)
    for k, r in result.items():
        print('%-8s matched %4d  open %d  -> %.1f%% of %d' % (k, r['matched'], r['open'], r['percent'], r['total']))


if __name__ == '__main__':
    main()
