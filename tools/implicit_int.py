#!/usr/bin/env python3
"""Give the shared sources' implicit ints an explicit int16.

    python3 tools/implicit_int.py [--dry-run]

K&R C lets a function be declared or defined without a return type, and a parameter go
undeclared: both are int, 16 bits under Microsoft C 4.00 but 32 on a modern host. After
tools/widths.py every written int is int16 (portable.h), so whatever clang still sees as
plain "int" in SRC/ and SRC-TITLE/ is implicit. This finds them with clang's AST (built
with the port's headers, port/include) and writes int16: before the function's name (and
its calling convention keyword and address comment) for a return type, as a declaration
line before the body for a parameter. int16 is int under MSC, so the DOS build compiles to
the same bytes (tools/dosbuild.py).
"""
import json, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PORT = os.path.join(ROOT, 'port')
PROGRAMS = (('SRC', 'avatar'), ('SRC-TITLE', 'title'))


def ast(cfile, incdir):
    cmd = ['clang', '-std=gnu89', '-fsigned-char', '-w', '-Xclang', '-ast-dump=json', '-fsyntax-only',
           '-I', incdir, '-I', os.path.join(PORT, 'include', 'msc'), '-I', os.path.join(PORT, 'include'),
           '-include', os.path.join(PORT, 'include', 'compat.h'), '-x', 'c', cfile]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if not r.stdout:
        sys.exit('clang failed on %s:\n%s' % (cfile, r.stderr[-2000:]))
    return json.loads(r.stdout)


def implicit_sites(tree, src_dir):
    """(kind, name, file, line, col) of the decls whose type is plain int"""
    out = set()
    cur = {'file': None}

    def upd(loc):
        """clang prints a location's file only when it differs from the last one printed:
        follow them in the order they are printed (spelling before expansion)"""
        if not loc:
            return
        if 'spellingLoc' in loc or 'expansionLoc' in loc:
            upd(loc.get('spellingLoc'))
            upd(loc.get('expansionLoc'))
            return
        if 'file' in loc:
            cur['file'] = loc['file']

    def walk(n, fn=None):
        loc = n.get('loc', {})
        upd(loc.get('spellingLoc', loc))
        f = cur['file'] or ''          # the file of the name (its spelling)
        upd(loc.get('expansionLoc'))
        upd(n.get('range', {}).get('begin', {}))
        upd(n.get('range', {}).get('end', {}))
        k = n.get('kind')
        mine = os.path.dirname(os.path.realpath(f)) == os.path.realpath(src_dir) and \
            os.path.basename(f).upper() != 'PORTABLE.H'
        if mine and k in ('FunctionDecl', 'ParmVarDecl', 'VarDecl'):
            t = n.get('type', {}).get('qualType', '')
            base = t.split('(')[0].strip()
            loc = n.get('loc', {})
            loc = loc.get('spellingLoc', loc)
            if (k == 'FunctionDecl' and base == 'int') or (k != 'FunctionDecl' and t == 'int'):
                out.add((k, n.get('name'), os.path.realpath(f), loc.get('line'), loc.get('col'),
                         fn if k == 'ParmVarDecl' else None))
        inner_fn = n.get('name') if k == 'FunctionDecl' else fn
        for c in n.get('inner', []):
            walk(c, inner_fn)

    walk(tree)
    return out


SKIP_BEFORE = re.compile(r'(?:/\*[^*]*\*+(?:[^/*][^*]*\*+)*/|\b(?:cdecl|pascal|far|near)\b|\s)*$')


def patch_file(path, sites):
    lines = open(path, encoding='latin-1', newline='').read().split('\n')
    params = []
    for kind, name, _, line, col, fn in sorted(sites, key=lambda s: (-s[3], -s[4])):
        i = line - 1
        text = lines[i]
        if kind == 'FunctionDecl':
            pos = col - 1
            assert text[pos:pos + len(name)] == name, (path, line, name, text)
            head = text[:pos]
            m = SKIP_BEFORE.search(head)
            ins = m.start()
            while ins < len(head) and head[ins] in ' \t':
                ins += 1
            lines[i] = text[:ins] + 'int16 ' + text[ins:]
        elif kind == 'ParmVarDecl':
            params.append((i, name))
        else:
            m = re.search(r'\bregister\s+(?=%s\b)' % re.escape(name), text)
            assert m, (path, line, name, text)
            lines[i] = text[:m.end()] + 'int16 ' + text[m.end():]
    for i, name in sorted(params, reverse=True):
        # the K&R declarations end at the line holding the body's '{'
        j = i
        while not lines[j].lstrip().startswith('{'):
            j += 1
        eol = '\r' if lines[j].endswith('\r') else ''
        lines.insert(j, 'int16 %s;%s' % (name, eol))
    open(path, 'w', encoding='latin-1', newline='').write('\n'.join(lines))


def main():
    dry = '--dry-run' in sys.argv
    sites = set()
    for sub, inc in PROGRAMS:
        d = os.path.join(ROOT, sub)
        incdir = os.path.join(PORT, 'include', inc)
        for name in sorted(os.listdir(d)):
            if name.upper().endswith('.C'):
                sites |= implicit_sites(ast(os.path.join(d, name), incdir), d)
    byfile = {}
    for s in sites:
        byfile.setdefault(s[2], []).append(s)
    for path, ss in sorted(byfile.items()):
        print('%-40s %d' % (os.path.relpath(path, ROOT), len(ss)))
        if not dry:
            patch_file(path, ss)
    print('total', len(sites))


if __name__ == '__main__':
    main()
