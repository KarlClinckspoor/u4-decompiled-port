#!/usr/bin/env python3
"""Apply the global rename map to the sources, in place.

    python3 tools/rename.py [--map tools/rename_map.json] [ROOT]

tools/rename_map.json has one map per program: "avatar" (SRC/, forVS/ except the title
part) and "title" (SRC-TITLE/, forVS/u4title_main.c). Each is {old identifier: new}.

* C/H: identifier tokens are replaced, in code and preprocessor lines. When an
  address-named symbol (C_xxxx, D_xxxx) is renamed where it is declared or defined (at file
  scope, or a static inside a function), its old name is kept in a comment just in front of
  the new one: /*C_2F7E*/WaitKeyOrTimeout(). In comments, whole-word occurrences of old names
  that look technical (with a digit or '_') are replaced too, except pure address markers.
* ASM: identifiers are replaced case-insensitively (also with the leading '_' of cdecl
  names), in code and in ';' comments. A label defined at column 0 that had an address name
  gets the old name as a comment.
* SRC-DRV/ (the CGA driver) only gets the Gra_* names (its own labels are its own addresses).

Renaming never changes the compiled program: tools/dosbuild.py checks it with the original
compiler.
"""
import argparse, json, os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ctok

ADDR = re.compile(r'^(?:C|D)_[0-9A-F]{4}$')
PURE_MARKER = re.compile(r'^/\*\s*[A-Za-z_]*_[0-9A-Fa-f]{4}(?:[,/][A-Za-z_]*_?[0-9A-Fa-f]{4})*\s*\*+/$')
DECL_KEYWORDS = {'static', 'extern', 'register', 'auto'}


def rename_c(src, gmap, cmap_re):
    toks = ctok.tokenize(src)
    out = []
    depth = paren = 0
    stmt_first = None          # first significant token of the current statement
    prev_sig = None
    prev_comment = None
    for kind, text, s, e in toks:
        if kind in ('ws', 'nl'):
            out.append(text)
            continue
        if kind == 'comment':
            prev_comment = text
            if cmap_re and not PURE_MARKER.match(text):
                text = cmap_re.sub(lambda m: gmap[m.group(0)], text)
            out.append(text)
            continue
        if kind == 'pp':
            sub = []
            for k2, t2, _, _ in ctok.tokenize(text, pp_as_line=False):
                if k2 == 'id' and t2 in gmap:
                    sub.append(gmap[t2])
                elif k2 == 'comment' and cmap_re and not PURE_MARKER.match(t2):
                    sub.append(cmap_re.sub(lambda m: gmap[m.group(0)], t2))
                else:
                    sub.append(t2)
            out.append(''.join(sub))
            prev_sig = None
            stmt_first = None
            continue
        if stmt_first is None:
            stmt_first = text
        if kind == 'id' and text in gmap:
            new = gmap[text]
            declaring = paren == 0 and (depth == 0 or stmt_first in ('static',)) and prev_sig not in ('.', '->')
            if ADDR.match(text) and declaring:
                already = prev_comment is not None and text in prev_comment and out and out[-1] == prev_comment
                if not already:
                    new = '/*%s*/%s' % (text, new)
                    if out and out[-1].endswith('*'):
                        new = ' ' + new
            out.append(new)
        else:
            out.append(text)
        if kind == 'punct':
            if text == '{':
                depth += 1
                stmt_first = None
            elif text == '}':
                depth -= 1
                stmt_first = None
            elif text == ';':
                stmt_first = None
            elif text == '(':
                paren += 1
            elif text == ')':
                paren -= 1
        prev_sig = text
        prev_comment = None
    return ''.join(out)


_asm_tok = re.compile(r"(;[^\n]*)|('[^'\n]*'|\"[^\"\n]*\")|([A-Za-z_@$?][A-Za-z0-9_@$?]*)")


def rename_asm(src, gmap, cmap_re):
    low = {}
    for o, n in gmap.items():
        low[o.lower()] = n
        low['_' + o.lower()] = '_' + n
    lines = []
    for line in src.split('\n'):
        cr = line.endswith('\r')
        if cr:
            line = line[:-1]
        first = []

        def rep(m):
            if m.group(1):
                c = m.group(1)
                return cmap_re.sub(lambda mm: gmap[mm.group(0)], c) if cmap_re else c
            if m.group(3):
                n = low.get(m.group(3).lower())
                if n is not None:
                    first.append((m.start(), m.group(3)))
                    return n
            return m.group(0)
        new = _asm_tok.sub(rep, line)
        if first and first[0][0] == 0 and ADDR.match(first[0][1].lstrip('_')):
            old = first[0][1].lstrip('_')
            if ';' in new:
                c = new.index(';')
                if old not in new[c:]:
                    new = new[:c + 1] + old + ' ' + new[c + 1:]
            else:
                new = new.rstrip() + '\t;' + old
        lines.append(new + ('\r' if cr else ''))
    return '\n'.join(lines)


def comment_re(gmap):
    tech = [k for k in gmap if re.search(r'[0-9_]', k) and len(k) > 3]
    if not tech:
        return None
    return re.compile(r'\b(' + '|'.join(sorted(map(re.escape, tech), key=len, reverse=True)) + r')\b')


def apply(path, gmap):
    cre = comment_re(gmap)
    raw = open(path, 'rb').read()
    s = raw.decode('latin-1')
    up = path.upper()
    if up.endswith(('.C', '.H')):
        n = rename_c(s, gmap, cre)
    elif up.endswith('.ASM'):
        n = rename_asm(s, gmap, cre)
    else:
        return False
    if n != s:
        open(path, 'wb').write(n.encode('latin-1'))
        return True
    return False


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser()
    ap.add_argument('--map', default=os.path.join(here, 'rename_map.json'))
    ap.add_argument('root', nargs='?', default=os.path.dirname(here))
    a = ap.parse_args()
    maps = json.load(open(a.map))
    av = {k: v for k, v in maps['avatar'].items() if k != v}
    ti = {k: v for k, v in maps['title'].items() if k != v}
    gra = {k: v for k, v in av.items() if k.startswith('Gra_')}
    jobs = []
    for d, m in (('SRC', av), ('SRC-TITLE', ti), ('SRC-DRV', gra)):
        p = os.path.join(a.root, d)
        for f in sorted(os.listdir(p)):
            jobs.append((os.path.join(p, f), m))
    vs = os.path.join(a.root, 'forVS')
    if os.path.isdir(vs):
        for f in sorted(os.listdir(vs)):
            jobs.append((os.path.join(vs, f), ti if 'title' in f.lower() else av))
    changed = [p for p, m in jobs if apply(p, m)]
    print('%d files changed' % len(changed))


if __name__ == '__main__':
    main()
