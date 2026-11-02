# Lessons learned: the native port

The port (this branch, `portable`) compiles the game's own C, the same files that still build
byte-identical `AVATAR.EXE` and `TITLE.EXE` for DOS, for 64-bit Windows and Linux on SDL2.
Everything DOS provided is rewritten in C under `port/`: the assembly modules, the EGA drivers,
the PC speaker, DOS files, the clock and the keyboard. How it is built and used is in
[PORT.md](PORT.md). This file is about what the work taught. The lessons of the readable
rewrite it builds on are in [../LESSONS-DECOMPILATION.md](../LESSONS-DECOMPILATION.md).

## 1. One codebase works if the shared sources only change in neutral ways

The aim was the approach of underworld-exhumed: the original and the port in one tree, with no
fork of the game's C. That held, under one rule: **every change to `SRC/` and `SRC-TITLE/`
compiles to the same bytes under MSC 4.00**, and `tools/dosbuild.py` proves it after each
change. In practice that meant:

- **`PORTABLE.H` macros** that expand to exactly the original tokens under MSC (`M_I86`):
  - `int16`, `uint16`, `int32` and `uint32`;
  - `RND2_CARRY`, for the carry flag `u_rand_b` reads (§5);
  - `BUSY_PASS`, the body of a busy-wait loop, where the port counts time;
  - `LOCAL_AT`, a read before a local buffer.
- **Conversions by script, not by hand.**
  - `tools/widths.py` gave every integer type an explicit width.
  - `tools/implicit_int.py` used clang's AST to find K&R's implicit ints (return types,
    undeclared parameters) and wrote `int16`.
- **Declarations that match their definitions.** Where `U4.H` disagreed with the code, the
  header was corrected (for example `karma_inc`/`karma_dec`, `MonsterIndex`).
- **Generated prototypes for the host only** (`tools/mkprotos.py` → `portprot.h`). The original
  declares most functions with empty parameter lists and calls some before declaring them.
  MSC 4.00 passes every argument as a 16-bit word, so that works in DOS. On a 64-bit host, an
  argument passed without a prototype is not converted to the parameter's type, and an
  implicit declaration takes an int result.
- **Everything else on the host side** lives in a force-included `compat.h`:
  - memory-model and calling-convention keywords defined away;
  - packed structures (`/Zp`);
  - `exit` and `main` routed to the port;
  - MSC's library extras.

**Lesson:** a single tree is cheaper to maintain than a fork. It is only safe while the
original toolchain can veto every shared change, so keep that check one command away.

## 2. The 16-bit semantics are the real port, and they are invisible

The code compiles on a modern host almost at once. It then plays a subtly different game,
because MSC 4.00 and a modern compiler disagree about arithmetic in ways no warning shows:

- **Unsigned-preserving promotions.** MSC follows the pre-ANSI rule: `unsigned char` promotes
  to `unsigned int`. So an unsigned char plus an int is *unsigned 16-bit* arithmetic. The host
  promotes both to signed 32-bit int. The disassembly gives it away by comparing with `jc`/`jnc`.
- **Hex constants 8000h-FFFFh** are `unsigned int` in MSC and `int` on the host.
- **Byte comparisons.** MSC compares a char with an unsigned char, or with a constant above 7Fh,
  byte by byte for `==` and `!=`.
- **`/J` files.** Some files are compiled with unsigned `char`, so the port compiles them with
  `-funsigned-char`, file by file, as `SRC/MAKEFILE` does.
- **16-bit results widened.** File offsets computed in 16 bits and then passed as a long: the
  world-map chunk, the dungeon room and the citizen's talk record (`dlseek`).

`port/tools/intaudit.py` walks clang's AST and lists every site where the value can differ.
Each site got either the cast that reproduces MSC's result or a note that no difference is
possible. Each cast was proved neutral by the DOS build.

The audit tool needed its own debugging:

- **Its first version lost sites**, because it derived line numbers from file offsets.
- **It knew neither the unsigned-preserving rule nor the `/J` files**, so a whole class of
  sites only appeared in a second round.

**Lesson:** treat the audit as code under test. Check it on cases you already know before
trusting a clean report.

## 3. A matching DOS build does not prove a cast is right

The proof that a cast is *neutral* is easy: the DOS build matches. The proof that it is
*correct* on the host needs the disassembly. `StealGold` tests `gold - amount >= 0`. A cast was
added to reproduce a 16-bit wrap of the subtraction, and the DOS build stayed identical. But
MSC compiles that test as `gold >= amount`, with no subtraction to wrap, so the cast described
something the original never did. It was reverted, with a comment.

**Lesson:** for every cast added for its meaning, find the instruction in the original
program that it is meant to reproduce.

## 4. Evaluation order is behaviour

MSC's pascal calls (`/Gc`) evaluate their arguments left to right. The game makes several
random draws inside one argument list, so the order decides the game. C leaves the order
unspecified. In practice clang goes left to right on these targets, and GCC on x86-64 often
does not. The port therefore requires clang (CMake warns otherwise), and `U4_RND1` computes its
operand before the call, as MSC does.

The calling convention also hid a mismatch. `TLK_Say` is declared with one parameter and
mostly called with two. With pascal calls in DOS this is harmless, and the comment at the
function explains why. On the host the extra parameter had to be declared (`#ifndef M_I86`).

**Lesson:** list the calling conventions the original relies on, and what each one quietly
tolerates. Every tolerance is a difference on the host.

## 5. Flags leak across the C/assembly boundary

`u_rand_b`'s first `ADC` adds the carry flag *the caller left*, which comes from the C
compiler's last comparison before the call. The C source shows nothing of this. Finding it took
the disassembly of every call site:

1. At each site, work out what CF holds on every path that reaches the call.
2. Write it into the source as `RND2_CARRY(condition)`, which is empty under MSC.

AVATAR's four call sites were handled first. TITLE's `AnimateViewTiles` has the same pattern
and was missed. That changed the title's random state, and with it the gypsy's cards. An
independent review of the assembly replacements found it.

**Lesson:** when C calls assembly, check what the assembly reads besides its arguments: flags,
registers, the stack above its frame.

## 6. Have someone who did not write it review it

The independent review of the assembly replacements found what the author was blind to:

- **The missing carry site** (§5).
- **A miscounted speaker.** `OUT 61h` only moves the cone when bit 1 changes, and each routine
  ends by restoring the port. The model flipped on every `OUT` and missed the final edge.
- **A timing estimate about 130 times too high.** TITLE's refresh is a countdown costing about
  150 cycles, not a redraw at 20000. In the title, the cursor and the corner monsters ran about
  25 times too slowly.

**Lesson:** brief the reviewer with the code and the original, not with your conclusions, and
ask for a list of differences, not for confirmation.

## 7. Memory layout is behaviour too

The game reads and writes past the ends of its tables, on purpose or not:

- `LoadCombatMap` reads `(char *)&Fighters - (char *)&Combat` bytes into `Combat`;
- `CMD_HoleUp` clears 32 entries of a 16-entry array, running into the next one;
- `MonsterIndex` reads past the monster tables for people.

So the layout of DOS memory had to be kept where the game depends on it:

- **The assembly modules' uninitialised globals** are one block per program, in DOS's order
  (`port/tools/gen_bss.py` → `port/sys/bss_*.c`). Objects that hold pointers grow on the host.
  Every run of non-pointer objects stays exactly as contiguous as in DOS.
- **Two data sections** keep DOS's exact layout as packed blocks (`PORT_BLOCK`, generated by
  `port/tools/gen_block.py`). These are the monster tables and `TITLE.EXE`'s `DATA.C`.
  `port/tests/title_data_check.py` checks the latter against `DATA.ASM`.
- **The generator checks its assumptions.** For example, a `jmp_buf` is 18 bytes in DOS, 200
  with glibc and 256 with MinGW. The block reserves 512 bytes and a static assert checks it.

The bug this caught late: `TITLE.EXE`'s `patch_tandy` is a far pointer that no header
declares. The generator kept its DOS size, the port stored an 8-byte host pointer there, and
the store ran over `seed_128`. The crash came later, when the pointer was freed.

**Lesson:** anything the headers do not declare is invisible to tools built on the headers.
List such objects explicitly, and make the generator fail rather than guess.

## 8. Uninitialised locals have to be made deterministic

Some locals are read before they are set: the sell loop's `answer`, and `foe` in `COM_Shoot`
with oil at range 0. In DOS they hold whatever the stack held. The port compiles the game's C
with `-ftrivial-auto-var-init=zero` and `-fwrapv`. Every build on every host then plays the
same game, which is what makes recordings portable (§11).

## 9. Rewrite graphics against a model of the hardware, and diff against the original

`EGA.DRV` and `TITLE.EXE`'s graphics were rewritten in C against a model of the EGA
(`ega_hw.h`): four planes, latches, the sequencer and graphics controller, and write modes 0-2.
Drawing to pixels directly would have been simpler, but the driver's quirks live in that
hardware. Pixel code would have to fake them.

The checking is what made it trustworthy:

- **The original runs in unicorn.** The tests emulate the EGA through memory-mapped I/O and
  provide a little DOS. They run the original 16-bit code and the C on thousands of random
  cases, plus every real picture, and compare the four planes byte for byte:
  - `ega_diff.py`: 3248 cases, about 20 s;
  - `tgfx_diff.py`: 1953 cases, about 90 s.
- **The harness itself was tested.** Bugs were planted in scratch copies of the C. Every one
  that changes behaviour was caught.
- **Two real bugs came out.**
  - `ega_init` never loaded the palette registers.
  - `FillTiles` with a width of 0 must draw 256 tiles: byte counters decremented after the
    work mean 0 = 256. The same is true of several of the title's counters.
- **Quirks are kept and commented**, for example:
  - `Gra_Dot` ORs its colour, so colour 0 draws nothing;
  - `Gra_AnimFlags`' random seed is BSS nothing writes, so all five flags swap at every call.

**Lesson:** when the original code can be executed, differential testing against it beats any
amount of reading.

## 10. Time belongs to the emulated machine, not the wall clock

The DOS game has no clock. It runs as fast as the PC allows and scales its animations by
`speed_info`, a loop count it measures at start-up. Speeding it up or slowing it down by guess
would get every animation and delay slightly wrong. The port instead runs a virtual PC:

- **Each replacement routine adds the 8088 cycles its DOS code took** (`vc_cycles`): EGA writes,
  BIOS and DOS calls, busy-wait loops (`BUSY_PASS`), the disk.
- **DOS's clock reads that virtual clock**, and the game keeps pace with real time on it.
- **`speed_info` is a setting** (default 3, about a PC/AT, the speed the game's tables were
  tuned for), and is recorded.
- **The PC speaker is modelled as edges at cycle times** and rendered to audio. The effects
  therefore sound as their loops made them sound.

Three things went wrong on the way:

- **Some costs are estimates**, for example the per-mode cost of a view refresh. One of them was
  wildly off until the review (§6).
- **A real-time rate limit made test runs nondeterministic.** The `--fast` test mode first
  limited the platform pump by real time. Now it uses virtual time, and the same keys give the
  same run.
- **The restart menu looped in headless runs.** A hidden run that ends with exit code 4
  restarted forever until the restart was disabled when hidden.

**Lesson:** derive all time from one emulated clock. Then pacing, sound and replays agree with
each other, and tests can run faster than real time without changing what happens.

## 11. Record at the boundary, and replays become the best regression test

With all time virtual, the only values the game reads from outside are:

- the date and time at start;
- `speed_info`;
- the result of each keyboard poll.

Recording just those reproduces a whole session. Every 512 reads, the recording also stores a
digest of the game's state: the party, the area, the combat, the mode and the random generator.
A replay compares it and reports the first divergence.

That turned into the strongest test the port has:

- **The same build replays its own recording**, which checks determinism.
- **Another platform replays it.** A recording made on Linux was replayed by the Windows build
  under Wine: all checkpoints identical. That checks the 16-bit semantics, the memory layout
  and the evaluation order all at once.
- **Fuzzing found no memory errors.** Hours of random keys in towns, the overworld, combat
  and a dungeon ran under AddressSanitizer, and every run is itself a recording that can be
  replayed.

**Lesson:** make the boundary narrow and explicit early, and record it. Determinism then becomes
something you can test, not something you hope for.

## 12. Two DOS programs in one executable

`ULTIMA.COM` runs `TITLE.EXE`, then `AVATAR.EXE`. The port links both into one program. The two
share many names: `Party`, `txt_X`, `u4_puts`, `Gra_Init`, `low_init` and more. Hand-editing
`TITLE.EXE`'s sources would have broken the one-codebase rule. Instead:

- `port/tools/gen_title.py` collects every name `TITLE.EXE` defines, from its C (clang's AST)
  and its assembly (the `PUBLIC` lines).
- It writes `trename.h`, which renames each one to `T_<name>` and is force-included in the
  title's files.
- The runtime files both programs need (`util2.c`, `low.c`) are compiled twice, once with the
  title's names.

## 13. Build and release

- **Upper-case `.C` files.** CMake took them for C++. They get `LANGUAGE C` and `-xc`.
- **Windows from Linux.** clang with `--target x86_64-w64-mingw32`, lld, MinGW's headers and
  libgcc, and SDL2's MinGW development package (`port/cmake/mingw64.cmake`). The exe needs only
  `SDL2.dll` and system DLLs. Wine runs it well enough for headless replays.
- **A Linux binary that runs on old distributions.** A plain build on a current system needs
  glibc 2.34. The 0.1.0 release was built with `zig cc -target x86_64-linux-gnu.2.17` instead,
  which brings its own glibc headers. The binary then needs glibc 2.14 and only `libc` and
  `libSDL2-2.0.so.0`.
  - SDL2's pkg-config file adds `-I/usr/include`, which mixes the system's glibc headers into
    zig's. A small `sdl2.pc` pointing only at the SDL2 headers avoids it.
  - Remove the build's RUNPATH (`patchelf --remove-rpath`) before packaging.
- **Testing without a window.** `--hidden`, `--keys`, `--run-ms`, `--fast`, `--clock` and
  `--screenshot` drove every test. Screenshots were the only check of what is on screen.

## 14. What is still unverified

- **Nobody has played it in a real window yet.** All checks ran headless.
- **The speaker has not been listened to.** Its effects were checked by counting edges against
  the disassembly and by their lengths (a 1.4 s magic zap, a 0.05 s beep).
- **The CGA and Tandy drivers are not ported.** The port's PC has an EGA.
- **The cycle costs are estimates.** Animations and delays are as fast as the model says, which
  is close to, not exactly, a real PC.
