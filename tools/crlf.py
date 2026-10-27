#!/usr/bin/env python3
"""Give files CRLF line endings (the DOS sources use them; MASM 3.00 needs them)."""
import sys
for p in sys.argv[1:]:
    b = open(p, 'rb').read()
    n = b.replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
    if n != b:
        open(p, 'wb').write(n)
