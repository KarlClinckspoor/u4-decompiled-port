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
