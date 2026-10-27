# Graphics diff tests

Two tests run the original 16-bit graphics code in unicorn and compare it with the C,
byte for byte (four 64 KB planes, palette, shapes, charset, picture buffers, random
states, speaker toggles, file reads, exit codes):

| test | original | C |
|---|---|---|
| `ega_diff.py` | `EGA.DRV` (AVATAR's driver, 25 entries) | `gfx/ega.c` |
| `tgfx_diff.py` | `TITLE_0.EXE`'s GRAPH.ASM / INFLATE.ASM (EGA) | `gfx/tgfx.c` |

Needs Python 3.9+ with `unicorn` 2.x and gcc. Each test builds its shim
(`ega_shim.c` / `tgfx_shim.c` + `shim_hooks.c`) into a shared library in a scratch
directory (default `$TMPDIR/u4gfx_tests`, `--build DIR` to change).

    python3 tests/ega_diff.py                 # about 20 s
    python3 tests/tgfx_diff.py                # about 90 s

Both print a summary per entry and exit 0 when everything is identical, 1 otherwise
(the first 20 differences are printed; `-v` prints all).

Options: `--cases N` (random cases per entry, default 200), `--seed S`,
`--data DIR` (the game files: `EGA.DRV`, the pictures, `SHAPES.EGZ`; default
`/home/claude/u4data`); `tgfx_diff.py` also takes `--exe TITLE_0.EXE` (the unpacked
build of SRC-TITLE, default `/home/claude/scratch/r1/TITLE_0.EXE`) and `--map` (its link
map, default: the `.MAP` next to it).

`ega_model.py` is the shared emulation: the EGA (planes, latches, sequencer and graphics
controller, write modes 0-2, ports 3DAh and 61h, INT 10h mode/palette) and a little DOS
(INT 21h open/read/lseek/close/alloc/free/exit serving the data directory).
