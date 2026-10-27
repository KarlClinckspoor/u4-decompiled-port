# Ultima IV, native port (branch `portable`)

The game's own C compiled for Windows and Linux on SDL2: `AVATAR.EXE`'s (`SRC/`: the game)
and `TITLE.EXE`'s (`SRC-TITLE/`: the opening, the menu, the gypsy's character creation).
These are the same files that still build byte-identical programs for DOS. The DOS pieces
they need are rewritten in C under `port/`:

- the assembly modules;
- the graphics drivers: `EGA.DRV` and `TITLE.EXE`'s own EGA routines;
- the PC speaker effects;
- DOS files, the clock and the keyboard.

On top of that there is a settings screen over the game (F11), and every session is recorded
so that it replays exactly.

You need your own copy of Ultima IV. The free GOG version works. The port reads the game's
files from its folder and never writes there.

## Building

The port lives on the `portable` branch:

    git checkout portable

The game's C must be compiled with **clang**. MSC 4.00's pascal calls evaluate their
arguments left to right, the game's random numbers depend on that order, and clang keeps it.

Linux, and Windows' WSL (Debian/Ubuntu: `sudo apt install cmake ninja-build clang libsdl2-dev`):

    cmake -S port -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang
    cmake --build build
    ./build/u4 --data "/path/to/Ultima 4"

On Windows, use MSYS2's **CLANG64** shell (`pacman -S mingw-w64-clang-x86_64-{clang,cmake,ninja,SDL2}`):

    cmake -S port -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build

Then copy `SDL2.dll` (from the MSYS2 `clang64/bin` folder) next to `build/u4.exe`.

You can also cross-compile for Windows from Linux. You need `apt install clang lld mingw-w64`
and SDL2's `SDL2-devel-2.x-mingw.tar.gz` from its GitHub releases:

    cmake -S port -B build-win -G Ninja -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_TOOLCHAIN_FILE=$PWD/port/cmake/mingw64.cmake \
          -DSDL2_DIR=/path/to/SDL2-2.x/x86_64-w64-mingw32/lib/cmake/SDL2
    cmake --build build-win

Put `SDL2.dll` (from `SDL2-2.x/x86_64-w64-mingw32/bin`) next to `u4.exe`.

The DOS build is unchanged. `python3 tools/dosbuild.py --tools DIR` builds both programs with
Microsoft C 4.00 and MASM in a headless DOSBox and checks that they are identical to the
reference.

## Playing

The first time, run `u4 --data FOLDER`, where FOLDER holds `WORLD.MAP`, `SHAPES.EGA`, `EGA.DRV`
and so on. The port remembers it. Without `--data` the port also looks in the current folder,
in `U4_DATA`, and in the usual GOG folders. If nothing is found it asks for the folder.

- **Home folder**: holds your settings (`u4port.cfg`), your saved game (`PARTY.SAV`,
  `MONSTERS.SAV`, `OUTMONST.SAV`, `DNGMAP.SAV`), recordings and backups.
  - It is `%APPDATA%\u4port` on Windows and `~/.local/share/u4port` on Linux. `--home DIR`
    chooses another.
  - The game writes its files here. Any file it has not written is read from the game folder.
- **The title**: as `ULTIMA.COM` does in DOS, the port starts with `TITLE.EXE`:
  - Lord British's signature, the logo and the view;
  - then its menu: (R)eturn to the view, (J)ourney Onward, (I)nitiate New Game.
  - A new game goes through the gypsy's questions. When it asks for the PLAYER disk or the
    PROGRAM disk, press **C**: the game folder is drive C.
  - Journey Onward starts the game.
  - With the setting "Title at start" off (or `--game`), the port goes straight to the game
    when the saved game has a party.
- **F11**: the settings screen. The game is paused while it is open.
  - Display: fullscreen (also Alt+Enter), window scale, 4:3 aspect, whole-number scaling.
  - Sound: the PC speaker effects and their volume. The game's own V command still turns
    them off.
  - Game:
    - the game folder;
    - recording on/off;
    - the title at start;
    - the PC's speed (below);
    - show this screen at start;
    - New game: your saved game moves to `backups/` and the title starts, so you can
      Initiate a new one.
  - Replay: the recordings, to watch. While one plays you can change its speed or take over.
- Saving is the game's own: (Q)uit and save. Closing the window does not save.

### The PC's speed

The DOS game has no clock. It runs as fast as the PC allows and scales its animations by the
speed it measures at start-up (`speed_info`: 1 on a 4.77 MHz PC/XT).

The port runs it on a virtual PC that is `speed_info` times an XT. Every routine that stands
for DOS code counts the time that code took on the 8088: the EGA writes, the BIOS and DOS
calls, the busy-wait loops, the disk. The game keeps pace with real time on that clock, and
DOS's clock is that clock. So the game runs as it did on such a PC.

The default is 3, about a PC/AT, which is the speed the game's tables were tuned for. Faster
values animate more smoothly but change nothing else. The speaker effects are played out
cycle by cycle from their loops, so they sound as they did.

## Recordings and replays

Every session of the game is recorded unless you turn it off. Each recording is a folder,
`recordings/YYYYMMDD-HHMMSS/` in the home folder. It holds `RECORD.U4R` and `stage/`, a copy
of the saved game the session started from. The newest five are kept. A replay copies
`stage/` into a temporary folder, so your own save is never touched, and feeds the game the
recorded inputs.

What is recorded is every value the game reads from outside:

- the date and time at start;
- `speed_info`;
- the result of each poll of the keyboard.

Nothing else is needed, because the game's time is the virtual PC's.

Every 512 reads, the recording also stores a digest of the game's state: the party, the area,
the combat and the random generator. A replay compares it, says so if it went another way,
and `u4 --replay` then exits with code 3.

The game's C is compiled as MSC saw it:

- 16-bit integers and its byte comparisons;
- packed structures;
- the DOS memory layout where the game reads past a table;
- locals that are always 0 before they are set.

So a recording made on Windows replays identically on Linux, and the other way round. Opening
the title is not recorded; a recording starts with the game.

    u4 --replay 20261027-093612           # a recording in the home folder, at its own pace
    u4 --replay DIR --speed 0 --hidden    # as fast as possible, no window
    u4 --replay DIR --continue            # play on from where the recording ends

## Options

    --data DIR          the game folder (remembered)
    --home DIR          where settings, saved games and recordings go
    --replay REC        play a recording (a folder, or a name in recordings/)
    --speed N           replay speed: 1, 2, 4, 8, or 0 for as fast as possible
    --continue          after a replay ends, keep playing (default: quit)
    --record DIR        record this session into DIR
    --no-recording      no black box recording this time
    --hidden            no window (replays, tests)
    --settings          open the settings screen before the game
    --scale N, --fullscreen, --no-aspect, --no-integer, --no-audio
    --title, --game     start with TITLE.EXE, or with the game when there is a party
    --screenshot FILE   write the screen as a BMP when the session ends (tests)
    --run-ms N          end the session after N ms (tests)
    --keys SCRIPT       type keys at given times (tests): "3000:up 3500:e 4000:esc"
    --fast              no pacing: --keys and --run-ms count the game's time (tests)
    --clock TIME        the PC's clock starts at YYYYMMDD-HHMMSS (tests)

## How it is put together

### Changes to the shared sources

`SRC/PORTABLE.H` (and its copy in `SRC-TITLE/`) is the only kind of change to the shared
sources. Every item below expands to exactly the original tokens under MSC (`M_I86`), so the
DOS build compiles to the same bytes.

- Explicit widths (`int16`, `uint16`, `int32`, `uint32`): `tools/widths.py` for the written
  types, `tools/implicit_int.py` for K&R's implicit ints.
- `RND2_CARRY`: the carry flag `u_rand_b` adds, which comes from the caller's last
  comparison.
- `BUSY_PASS`: the body of the busy-wait loops.
- `LOCAL_AT`: a read before a local buffer.

Beyond `PORTABLE.H`, a few other edits were made and checked byte-identical with
`tools/dosbuild.py`:

- `port/tools/intaudit.py` found the comparisons whose result depends on 16-bit
  arithmetic or on MSC's byte comparisons. Each now has the cast that gives MSC's result on
  any host.
- The declarations of a few functions now match their definitions.
- Two data sections are marked `PORT_BLOCK`.

### The port's own files

- `port/include/compat.h`, force-included in the game's files on the host:
  - the memory-model and calling-convention keywords go away;
  - every struct is packed (`/Zp`);
  - `exit` and `main` go to the port;
  - MSC's library extras are provided.

  The files SRC/MAKEFILE compiles with `/J` are compiled with `-funsigned-char`.
  `port/include/*/portprot.h` (from `tools/mkprotos.py`) declares every function with its
  real parameters.
- Data layout:
  - The uninitialised globals of the assembly modules are one block per program in DOS's
    order (`port/sys/bss_*.c`, from `port/tools/gen_bss.py`): `LoadCombatMap` reads
    `(char *)&Fighters - (char *)&Combat` bytes into `Combat`.
  - The `PORT_BLOCK` sections keep DOS's exact layout as packed blocks
    (`port/tools/gen_block.py`). These are the monster tables, which `MonsterIndex` reads
    past for people, and `TITLE.EXE`'s `DATA.C`.
  - `port/tests/title_data_check.py` checks that `DATA.C` holds `DATA.ASM`'s bytes.
- `port/gfx/ega.c` is `EGA.DRV` in C and `port/gfx/tgfx.c` is `TITLE.EXE`'s
  `GRAPH.ASM`/`INFLATE.ASM` in C. Both work on a model of the EGA's planes, latches and
  write modes (`ega_hw.h`).
- `port/sys/`:
  - `util2.c` (UTIL2: delay and cursor, random generators, keyboard), `low.c` (LOW),
    `u4dos.c` (U4_DOS), `graph.c` and `tgraph.c` (the driver stubs and interleaving),
    `sound.c` (U4_SOUND, BEEP), `msc.c` (MSC's DOS calls);
  - `vclock.c`: the virtual PC's clock, DOS's clock, the keyboard buffer, pacing;
  - `portmain.c`: what `ULTIMA.COM` does;
  - `session.c`: the home folder, recordings, replays;
  - `dosfs.c`: the DOS file system over the game folder.

  `util2.c` and `low.c` are built twice, once for each program.
- `TITLE.EXE` and `AVATAR.EXE` live in one executable: `TITLE.EXE`'s names are renamed `T_*`
  (`port/include/title/trename.h`, from `port/tools/gen_title.py`).
- `port/replay/` is the recording format, `port/ui/` the settings screen and settings file,
  and `port/platform/` holds `plat.h` and its SDL2 backend.
- Not ported:
  - the CGA and Tandy drivers (the port's PC has an EGA);
  - the copy protection's floppy read, whose result the game ignores.

## Tests

- `python3 port/tests/ega_diff.py`: `gfx/ega.c` against `EGA.DRV` run in unicorn
  (`pip install unicorn`). It covers every entry on thousands of random cases and compares
  the four planes byte for byte.
- `python3 port/tests/tgfx_diff.py`: `gfx/tgfx.c` against `TITLE.EXE`'s routines in unicorn,
  every picture and `SHAPES.EGZ` included.
- `python3 port/tests/title_data_check.py`: `DATA.C` against `DATA.ASM`.
- Replays: make a recording with `--keys` (and `--fast --clock`) and replay it with
  `--hidden --speed 0`. It must report "all identical", on Linux and on Windows alike.
