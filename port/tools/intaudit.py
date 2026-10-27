#!/usr/bin/env python3
"""List the expressions whose value can differ between Turbo C (16-bit int) and the host.

    python3 port/tools/intaudit.py [FILE.c ...] > audit.txt

Microsoft C 4.00 does unsigned 16-bit arithmetic when an operand is `unsigned` (uint16) or
`unsigned char` (it promotes unsigned types to unsigned int: the pre-ANSI "unsigned
preserving" rule) and none is long; the host promotes both to a signed 32-bit int first. So a comparison, a division,
a modulo or a right shift whose operand is (or contains, before any narrowing) such an
operation can give a different answer, and so can an `int` arithmetic result that leaves
16 bits before it is compared, divided, shifted right or widened. Hexadecimal constants
8000h..FFFFh are unsigned int in MSC and int on the host. MSC also compares a char with an
unsigned char (or a constant above 7Fh) byte by byte for == and !=, where the host compares
the promoted values.

This walks clang's AST and prints, per site, the operator, the DOS type of its operands
and the source text, for a person to decide (a cast in the source that gives the DOS
result, proved by tools/dosbuild.py, or "no difference possible").
"""
import json, os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
SRC = os.path.join(ROOT, 'SRC')
INC = os.path.join(ROOT, 'port', 'include')
# the files SRC/MAKEFILE compiles with /J (char is unsigned)
UNSIGNED_CHAR = {'U4_USE', 'U4_FILE', 'U4_MAIN', 'U4_MAIN2', 'U4_MAP', 'U4_END', 'U4_ANIM', 'U4_COMBA',
                 'U4_SPELL', 'U4_Q_N_V', 'U4_COMBB'}

CMP = {'<', '>', '<=', '>=', '/', '%', '>>', '==', '!='}
ARITH = {'+', '-', '*', '<<', '|', '^', '&', '~'}


def dos_type(qt):
    """'s16','u16','s32','u32','s8','u8', or None"""
    t = qt.get('desugaredQualType', qt.get('qualType', ''))
    q = qt.get('qualType', '')
    names = q + ' ' + t
    if 'uint16' in q or t in ('unsigned short',):
        return 'u16'
    if 'int16' in q or t == 'short':
        return 's16'
    if 'uint32' in q or t == 'unsigned int':
        return 'u32'
    if 'int32' in q or t == 'int':
        return 's32' if 'int32' in q else 'int'
    if t in ('unsigned char',) or (t == 'char' and CHAR_UNSIGNED):
        return 'u8'
    if t in ('char', 'signed char'):
        return 's8'
    if t in ('long', 'long long', 'unsigned long'):
        return 'wide'
    return None


def strip(n):
    while n.get('kind') in ('ImplicitCastExpr', 'ParenExpr') and n.get('inner'):
        if n.get('kind') == 'ImplicitCastExpr' and n.get('castKind') not in ('IntegralCast', 'LValueToRValue', 'NoOp'):
            break
        n = n['inner'][0]
    return n


def dtype_of(n):
    """the type the expression has in Turbo C"""
    n0 = strip(n)
    k = n0.get('kind')
    if k == 'IntegerLiteral':
        v = int(n0.get('value', '0'))
        txt = n0.get('_text', '')
        if 0x8000 <= v <= 0xffff and not txt.startswith(tuple('123456789')):
            return 'u16'
        if v > 0x7fff:
            return 's32'
        return 's16'
    if k == 'CharacterLiteral':
        return 's16'
    if k == 'BinaryOperator' and n0['opcode'] in CMP | ARITH | {'&&', '||'}:
        if n0['opcode'] in ('<', '>', '<=', '>=', '==', '!=', '&&', '||'):
            return 's16'
        a, b = dtype_of(n0['inner'][0]), dtype_of(n0['inner'][1])
        return arith(a, b, n0['opcode'])
    if k == 'UnaryOperator':
        return dtype_of(n0['inner'][0]) if n0['opcode'] in ('-', '~', '+') else 's16'
    if k == 'CStyleCastExpr':
        return dos_type(n0['type']) or 's16'
    if k == 'ConditionalOperator':
        return arith(dtype_of(n0['inner'][1]), dtype_of(n0['inner'][2]), '?')
    t = dos_type(n0.get('type', {}))
    if t == 'u8':
        return 'u16'   # MSC 4.00 promotes unsigned char to unsigned int (unsigned preserving)
    if t == 's8':
        return 's16'
    if t == 'int':
        return 's16'   # an implicit-int function or an int the port still has
    return t or 's16'


def arith(a, b, op):
    if op in ('<<', '>>'):
        return a if a in ('u16', 's32', 'u32') else 's16'
    s = {a, b}
    if 'u32' in s:
        return 'u32'
    if 's32' in s:
        return 's32'
    if 'u16' in s:
        return 'u16'
    return 's16'


SIMPLE = {'DeclRefExpr', 'MemberExpr', 'ArraySubscriptExpr', 'CallExpr', 'UnaryOperator'}


def nonneg_const(n):
    n0 = strip(n)
    if n0.get('kind') in ('IntegerLiteral', 'CharacterLiteral'):
        return int(n0.get('value', '0')) >= 0
    return False


def neg_const(n):
    n0 = strip(n)
    if n0.get('kind') == 'UnaryOperator' and n0.get('opcode') == '-' and strip(n0['inner'][0]).get('kind') == 'IntegerLiteral':
        return True
    return False


def raw_type(n):
    """the operand's own char type before promotion: 's8', 'u8' or None"""
    n0 = n
    while n0.get('kind') in ('ImplicitCastExpr', 'ParenExpr') and n0.get('inner'):
        if n0.get('kind') == 'ImplicitCastExpr' and n0.get('castKind') not in ('IntegralCast', 'LValueToRValue', 'NoOp'):
            break
        n0 = n0['inner'][0]
    t = n0.get('type', {}).get('desugaredQualType', n0.get('type', {}).get('qualType', ''))
    if t == 'unsigned char' or (t == 'char' and CHAR_UNSIGNED):
        return 'u8'
    if t in ('char', 'signed char'):
        return 's8'
    return None


def const_outside_s8(n):
    n0 = strip(n)
    if n0.get('kind') in ('IntegerLiteral', 'CharacterLiteral'):
        v = int(n0.get('value', '0'))
        return v > 127 or v < -128
    return False


def is_arith(n):
    n0 = strip(n)
    return n0.get('kind') == 'BinaryOperator' and n0['opcode'] in ('+', '-', '*', '<<')


def walk(n, out, src_of):
    k = n.get('kind')
    if k == 'BinaryOperator' and n.get('opcode') in CMP:
        a, b = n['inner'][0], n['inner'][1]
        ta, tb = dtype_of(a), dtype_of(b)
        op = n['opcode']
        dos = arith(ta, tb, op)
        reason = None
        fixes = []      # (operand, the cast that gives DOS's result)
        if op == '>>':
            if ta == 'u16' and is_arith(a):
                reason = 'unsigned 16-bit arithmetic shifted right'
                fixes.append((a, 'uint16'))
            elif ta == 's16' and is_arith(a):
                reason = '16-bit arithmetic shifted right without narrowing'
                fixes.append((a, 'int16'))
        elif dos == 'u16':
            # one side unsigned: differs if the other can be negative or either is arithmetic
            for x, y, tx, ty in ((a, b, ta, tb), (b, a, tb, ta)):
                if tx == 'u16' and (ty == 's16' and not nonneg_const(y)):
                    if neg_const(y):
                        reason = 'unsigned 16-bit %s with a negative constant (DOS compares as unsigned)' % op
                    else:
                        reason = 'unsigned 16-bit %s against a signed value' % op
                    fixes.append((y, 'uint16'))
                if is_arith(x) and tx == 'u16':
                    reason = 'unsigned 16-bit arithmetic compared (%s)' % op
                    fixes.append((x, 'uint16'))
        else:
            for side in (a, b):
                if is_arith(side) and dtype_of(side) == 's16':
                    reason = '16-bit %s result used by %s without narrowing' % (strip(side)['opcode'], op)
                    fixes.append((side, 'int16'))
        if not reason and op in ('==', '!='):
            ra, rb = raw_type(a), raw_type(b)
            for x, rx, ry, y in ((a, ra, rb, b), (b, rb, ra, a)):
                if rx == 's8' and (ry == 'u8' or const_outside_s8(y)):
                    reason = 'char %s unsigned char or a constant above 7Fh (MSC compares the bytes)' % op
                    fixes.append((x, 'unsigned char'))
        if reason:
            out.append((n, reason, fixes))
    if k in ('ImplicitCastExpr', 'CStyleCastExpr') and n.get('castKind') == 'IntegralCast' and n.get('inner'):
        # 16-bit arithmetic widened to 32 bits: MSC widens the 16-bit result (zero-extended
        # when unsigned), the host widens its 32-bit int result
        to = n.get('type', {}).get('qualType', '')
        inner = n['inner'][0]
        if ('int32' in to or to in ('long', 'unsigned long')) and is_arith(inner):
            t = dtype_of(inner)
            if t in ('u16', 's16'):
                out.append((n, '16-bit %s arithmetic widened to 32 bits' % ('unsigned' if t == 'u16' else 'signed'),
                            [(inner, 'uint16' if t == 'u16' else 'int16')]))
    for c in n.get('inner', []) or []:
        if isinstance(c, dict):
            walk(c, out, src_of)


FIX = False
CHAR_UNSIGNED = False   # the file is compiled with /J


def cast_edit(node, cast):
    """(begin, end, cast) to wrap the operand's source text, or None when it comes from a
    macro (fixed by hand)"""
    rng = node.get('range', {})
    b, e = rng.get('begin', {}), rng.get('end', {})
    if any(k in b or k in e for k in ('spellingLoc', 'expansionLoc')) or 'offset' not in b or 'offset' not in e:
        return None
    return (b['offset'], e['offset'] + e.get('tokLen', 0), cast)


def main():
    global FIX, CHAR_UNSIGNED
    if '--fix' in sys.argv:
        sys.argv.remove('--fix')
        FIX = True
    files = sys.argv[1:] or sorted(os.path.join(d, f) for d in (SRC, SRC + '-TITLE') for f in os.listdir(d)
                                   if f.upper().endswith('.C'))
    total = 0
    for f in files:
        tu = os.path.splitext(os.path.basename(f))[0].upper()
        title = os.path.basename(os.path.dirname(os.path.realpath(f))).upper() == 'SRC-TITLE'
        CHAR_UNSIGNED = tu in UNSIGNED_CHAR and not title
        prog = ['-I', os.path.join(INC, 'title'), '-include', os.path.join(INC, 'compat.h'),
                '-include', os.path.join(INC, 'title', 'trename.h')] if title else \
               ['-I', os.path.join(INC, 'avatar'), '-include', os.path.join(INC, 'compat.h')]
        cmd = ['clang', '-std=gnu89', '-funsigned-char' if tu in UNSIGNED_CHAR and not title else '-fsigned-char',
               '-w', '-Wno-return-type', '-Wno-int-conversion', '-Wno-incompatible-function-pointer-types', '-Wno-incompatible-pointer-types', '-DPORT_TU_%s' % tu] + (['-DPORT_BLOCK'] if tu == 'U4_COMBC' else []) + prog + [
               '-I', os.path.join(INC, 'msc'), '-I', INC, '-fsyntax-only', '-Xclang', '-ast-dump=json', '-x', 'c', f]
        r = subprocess.run(cmd, capture_output=True, text=True, cwd=SRC)
        if r.returncode:
            print('# %s: clang failed' % f, file=sys.stderr)
            continue
        ast = json.loads(r.stdout)
        text = open(f, encoding='latin-1', newline='').read()
        lines = text.split('\n')
        sites = []
        edits = set()
        manual = []
        for d in ast.get('inner', []):
            loc = d.get('loc', {})
            if d.get('kind') == 'FunctionDecl' and d.get('inner') and loc.get('file', f).endswith(os.path.basename(f)):
                fs = []
                walk(d, fs, None)
                for n, why, fixes in fs:
                    for node, cast in fixes:
                        edit = cast_edit(node, cast)
                        if edit:
                            edits.add(edit)
                        else:
                            manual.append((n, why))
                    rng = n.get('range', {}).get('begin', {})
                    rng = rng.get('expansionLoc', rng)
                    # clang leaves out a line that did not change: the offset is always there
                    ln = text.count('\n', 0, rng['offset']) + 1 if 'offset' in rng else None
                    sites.append((ln, d.get('name'), why))
        seen = set()
        for ln, fn, why in sites:
            key = (ln, why)
            if key in seen or not ln:
                continue
            seen.add(key)
            total += 1
            print('%s:%s: %s: %s\n    %s' % (os.path.basename(f), ln, fn, why, lines[ln - 1].strip() if ln and ln <= len(lines) else ''))
        for n, why in manual:
            rng = n.get('range', {}).get('begin', {})
            rng = rng.get('expansionLoc', rng)
            print('%s:%s: (in a macro: by hand) %s' % (os.path.basename(f), text.count('\n', 0, rng.get('offset', 0)) + 1, why))
        if FIX and edits:
            ins = []
            for b, e, cast in edits:
                ins.append((e, 0, ')'))
                ins.append((b, 1, '(%s)(' % cast))
            for pos, _, t in sorted(ins, key=lambda x: (x[0], x[1]), reverse=True):
                text = text[:pos] + t + text[pos:]
            open(f, 'w', encoding='latin-1', newline='').write(text)
            print('# %s: %d casts written' % (os.path.basename(f), len(edits)), file=sys.stderr)
    print('# %d sites' % total, file=sys.stderr)


if __name__ == '__main__':
    main()
