#!/usr/bin/env python3
"""Check that renaming local variables keeps Microsoft C 4.00's stack layout.

    python3 tools/localorder.py OLD.C NEW.C     compare the locals of two versions of a file
    python3 tools/localorder.py --hash NAME...  print each name's bucket

Microsoft C 4.00 keeps a function's local symbols in a hash table of 11 buckets, the
hash being the sum of the name's characters modulo 11, each bucket listing the most recently
declared symbol first. The locals get their stack slots in that order: bucket 0 first
(nearest to BP), and inside a bucket the last declared first. Renaming locals therefore
moves them on the stack unless the order stays the same; and the stack offsets are in the
code, so the program would change.

This tool lists, for every block of every function, the order in which the compiler will
allocate its locals (registers and statics included, to be safe), under the old and the new
names, and reports the blocks whose order changed. The functions and blocks are matched by
their position in the file, so only renames may differ between the two files.
"""
import os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import ctok

TYPE_WORDS = {'int', 'char', 'unsigned', 'signed', 'short', 'long', 'register', 'static', 'struct',
              'union', 'void', 'far', 'near', 'huge', 'auto', 'const', 'float', 'double', 'enum',
              'U16', 'S16', 'tMap32x32', 'tMap8x8x8', 'jmp_buf', 'cdecl', 'pascal'}


def bucket(name):
    return sum(map(ord, name)) % 11


def alloc_order(names):
    """names in declaration order -> order of allocation"""
    idx = {n: i for i, n in enumerate(names)}
    return sorted(names, key=lambda n: (bucket(n), -idx[n]))


def sig(src):
    return [t for t in ctok.tokenize(src, pp_as_line=True) if t[0] not in ('ws', 'nl', 'comment', 'pp')]


def typedef_names(toks):
    out = set()
    for i, t in enumerate(toks):
        if t[1] == 'typedef':
            j = i
            depth = 0
            last = None
            while j < len(toks) and not (toks[j][1] == ';' and depth == 0):
                if toks[j][1] in '({[':
                    depth += 1
                elif toks[j][1] in ')}]':
                    depth -= 1
                if toks[j][0] == 'id' and depth <= 1:
                    last = toks[j][1]
                j += 1
            if last:
                out.add(last)
    return out


def parse_decl(toks, i, types):
    """parse one declaration starting at toks[i]; return (names, index after ';')"""
    names = []
    depth = 0
    j = i
    expect_tag = False
    while j < len(toks):
        k, t = toks[j][0], toks[j][1]
        if t == ';' and depth == 0:
            return names, j + 1
        if t in ('{', '('):
            depth += 1
        elif t in ('}', ')'):
            depth -= 1
        elif t == '[':
            depth += 1
        elif t == ']':
            depth -= 1
        elif t == '=' and depth == 0:
            # skip the initializer up to ',' or ';' at depth 0
            j += 1
            d2 = 0
            while j < len(toks) and not (toks[j][1] in (',', ';') and d2 == 0):
                if toks[j][1] in '({[':
                    d2 += 1
                elif toks[j][1] in ')}]':
                    d2 -= 1
                j += 1
            continue
        elif k == 'id':
            if t in ('struct', 'union', 'enum'):
                expect_tag = True
            elif expect_tag:
                expect_tag = False
            elif t not in types and t not in TYPE_WORDS:
                nxt = toks[j + 1][1] if j + 1 < len(toks) else ';'
                # a declarator name, not a parameter name inside (...)
                if nxt in ('[', ',', ';', '=', ')') and (depth == 0 or (depth == 1 and toks[j - 1][1] == '*' and nxt == ')')):
                    names.append(t)
        j += 1
    return names, j


def functions(src):
    """[(function name, [[locals of block 0], [block 1], ...])]"""
    toks = sig(src)
    types = typedef_names(toks) | TYPE_WORDS
    out = []
    i = 0
    depth = 0
    while i < len(toks):
        t = toks[i][1]
        if t == '{' and depth == 0:
            # is it a function body? look back for ')' or a K&R declaration ';' after ')'
            prev = toks[i - 1][1] if i else ''
            back = i - 1
            while back >= 0 and toks[back][1] not in ('=',) and not (toks[back][1] == ';' and False):
                if toks[back][1] in ('=', ']') and toks[back][1] == '=':
                    break
                back -= 1
                if back >= 0 and toks[back][1] in (')',):
                    break
            if prev in ('=',) or (i >= 2 and toks[i - 2][1] in ('struct', 'union', 'enum')) or prev in ('struct', 'union', 'enum'):
                # initializer or struct body: skip it
                d = 0
                while i < len(toks):
                    if toks[i][1] == '{':
                        d += 1
                    elif toks[i][1] == '}':
                        d -= 1
                        if d == 0:
                            break
                    i += 1
                i += 1
                continue
            # function name: the identifier before the first '(' of this definition
            j = i - 1
            name = '?'
            pd = 0
            while j >= 0:
                if toks[j][1] == ')':
                    pd += 1
                elif toks[j][1] == '(':
                    pd -= 1
                    if pd == 0 and toks[j - 1][0] == 'id':
                        name = toks[j - 1][1]
                if toks[j][1] in ('}', ) or (toks[j][1] == ';' and pd == 0 and name != '?'):
                    break
                j -= 1
            blocks = []
            i = body(toks, i, types, blocks)
            out.append((name, blocks))
            continue
        if t == '{':
            depth += 1
        elif t == '}':
            depth -= 1
        i += 1
    return out


def body(toks, i, types, blocks):
    """toks[i] is '{'; collect declarations at the start of this block and nested blocks"""
    assert toks[i][1] == '{'
    i += 1
    names = []
    blocks.append(names)
    # declarations at the start of the block
    while i < len(toks) and (toks[i][1] in types or toks[i][1] in TYPE_WORDS):
        n, i = parse_decl(toks, i, types)
        names.extend(n)
    while i < len(toks):
        t = toks[i][1]
        if t == '{':
            i = body(toks, i, types, blocks)
            continue
        if t == '}':
            return i + 1
        i += 1
    return i


def compare(old, new):
    fo, fn = functions(old), functions(new)
    ok = True
    if len(fo) != len(fn):
        print('different number of functions: %d / %d' % (len(fo), len(fn)))
        return False
    for (no, bo), (nn, bn) in zip(fo, fn):
        if len(bo) != len(bn):
            print('%s: different number of blocks' % nn)
            ok = False
            continue
        for k, (lo, ln) in enumerate(zip(bo, bn)):
            if len(lo) != len(ln):
                print('%s block %d: different number of locals %s / %s' % (nn, k, lo, ln))
                ok = False
                continue
            ren = dict(zip(lo, ln))
            want = [ren[x] for x in alloc_order(lo)]
            got = alloc_order(ln)
            if want != got:
                ok = False
                print('%s block %d: stack order changes' % (nn, k))
                print('   old: ' + ', '.join('%s(%d)' % (x, bucket(x)) for x in alloc_order(lo)))
                print('   new: ' + ', '.join('%s(%d)' % (x, bucket(x)) for x in got))
                print('   need the order: ' + ', '.join(want))
    return ok


def main():
    if sys.argv[1] == '--hash':
        for n in sys.argv[2:]:
            print('%-24s %2d' % (n, bucket(n)))
        return
    if sys.argv[1] == '--list':
        for name, blocks in functions(open(sys.argv[2], encoding='latin-1').read()):
            print(name, blocks)
        return
    old = open(sys.argv[1], encoding='latin-1').read()
    new = open(sys.argv[2], encoding='latin-1').read()
    sys.exit(0 if compare(old, new) else 1)


if __name__ == '__main__':
    main()
