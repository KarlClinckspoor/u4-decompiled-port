Low-level graphic functions are implemented in 3 differents files: CGA.DRV, EGA.DRV, TANDY.DRV.
At runtime, depending on the running configuration, only one is loaded in memory and its functions addressed through some "hooks" placed at the beginning at the file. Some kind of extra-simple DLL.
Only AVATAR.EXE uses this system. TITLE.EXE has the functions for all three types of devices linked into it.
Since this is not the most interesting part of the job (and since I love CGA), I only reversed CGA.DRV.

## Building

`doit.bat` is the original recipe (MASM 5.10, LINK, EXE2BIN). `python3 tools/dosbuild.py --tools DIR --drv` builds
the driver with the tools the rest of the project uses instead: MASM 3.00 and the LINK of Microsoft C 4.00 in
DOSBox, then strips the EXE header as EXE2BIN does (the driver has no relocations). The result is checked against
the `CGA` line of `tools/build.sha1`, the SHA-1 of the shipped `CGA.DRV`: it is identical.

MASM 3.00 never uses the short sign-extended immediate form (opcode 83h) for `AND`, `OR` and `XOR`, and MASM 5
does. The shipped driver has that form in five instructions (`AND BX,1` in the tile zoom, and the four
`XOR WORD PTR ...,0FFFFh` that invert the view and a status line), so the source spells those five as `db`
with the instruction in a comment, as it already did for one `XOR` that has the long form. Assembled as plain
instructions with MASM 3.00, the driver is 5 bytes longer (3,318 bytes instead of 3,313) and every later offset
moves.

## EGA.DRV and TANDY.DRV (not reversed yet)

A survey made on 2026-11-23, before any work on them:

| | CGA | EGA | TANDY |
|---|---|---|---|
| Size (bytes) | 3,313 | 4,730 | 3,484 |
| Tables and data before the first hook | 486 | 631 | 503 |
| Code: bytes / about how many instructions | 2,827 / 1,174 | 4,099 / 1,708 | 2,981 / 1,283 |
| Code in 12-byte runs shared with CGA.DRV or TITLE's GRAPH.ASM | - | 1,766 (43%) | 1,151 (39%) |
| Short-form (83h) AND/OR/XOR that MASM 3.00 would need as `db` | 5 | about 3 | about 1 |

- The hook table has the same layout in all three (25 hooks, slot 7 empty), so the hook names and the
  source skeleton (header table, `random`, `_sgn`, `_abs`, the DOS helpers) carry over from `CGA_DRV.ASM`.
- TANDY is close to CGA hook by hook (ShiftUp/ShiftDown 0.90-0.93, RevealCodex 1.00, DrawQuad 0.84,
  AnimFlags 0.91 by instruction structure). EGA is mostly different (0.2-0.4) and planar (writes through
  port DX), apart from ShiftUp/ShiftDown, DrawQuad and the helpers.
- TITLE.EXE links EGA and Tandy versions of several routines (`SRC-TITLE/GRAPH.ASM`): Dot, LoadPicture,
  Init, AnimFlow, AnimFlags, PutChar, ClearScreen. Several driver hooks match them closely (EGA AnimFlags
  0.97, AnimFlow 0.92, PutChar 0.89; TANDY AnimFlow 0.94, AnimFlags 0.93).
- Method: disassemble the shipped driver into MASM 3.00 source, extend `dosbuild.py --drv` to all three
  drivers, iterate until it is byte-identical, then name by the hook table and by analogy with
  `CGA_DRV.ASM` and `GRAPH.ASM`.
- Estimate: TANDY about one agent session (2-3 hours); EGA one to two (3-5 hours); `--drv` for three
  drivers about 30 minutes. Risks: MASM 3.00's encoding choices and telling data from code; the byte
  check catches both at once.
