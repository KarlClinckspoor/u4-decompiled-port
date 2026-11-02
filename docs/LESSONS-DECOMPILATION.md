# Lessons learned: making the decompilation readable

ergonomy_joe's decompilation (branch `original`) rebuilds `AVATAR.EXE` and `TITLE.EXE` with
Microsoft C 4.00 and MASM. Its names are addresses (`C_2F7E`, `D_8742`, `bp06`). This branch
(`master`) gives every function, global, structure member, local and parameter a real name,
comments every function and puts named constants where literals were. The generated code does
not change: both programs still build to the same bytes.

This file records what that work taught: how to keep a rewrite byte-exact, the traps of this
compiler, and what reading every line of the game turned up. The port to Windows and Linux has
its own notes on the `portable` branch, in `docs/port/LESSONS-PORT.md`.

## 1. Make the original toolchain the referee

Nothing in this rewrite is trusted until the original compiler agrees.
`tools/dosbuild.py --tools DIR` does that check:

1. It copies the sources and Microsoft C 4.00, MASM 3.00 and LINK 3.51 to a scratch drive. It
   reads the compiler straight from the eight floppy images of `Microsoft C Compiler 4.0.zip`.
2. It builds both programs in a headless DOSBox, as `SRC/MAKEFILE` and `SRC-TITLE/DOIT.BAT` do.
3. It compares the SHA-1 of the linked programs (before EXEPACK) with `tools/build.sha1`.

A full build takes about 20 seconds. That made it possible to build after **every file**,
not after every batch. When a build differs, the change that broke it is one file old.

**Lesson:** settle the reference first, automate it, and make it fast enough to run all the
time. Every later rule in this file was found because a build stopped matching.

## 2. In Microsoft C 4.00, the names of locals are part of the code

The surprise of the project. MSC 4.00 keeps a function's local symbols in a hash table of 11
buckets. The bucket is the sum of the name's characters modulo 11. Locals get their stack slots
in table order:

- bucket 0 comes first, nearest BP;
- within a bucket, the last declared comes first.

The stack offsets are written into the instructions, so **renaming a local can move it, and
the program changes**. For example, `loc_A` hashes to bucket 5 and `loc_B` to 6. Renamed `x`
(10) and `y` (0), the same two locals swap places and `y` moves next to BP.
(`python3 tools/localorder.py --hash NAME...` prints a name's bucket.)

What this meant in practice:

- `tools/localorder.py OLD.C NEW.C` lists the allocation order of every block of every function
  under the old and the new names, and reports any block whose order changed. It ran on every
  file before the DOS build.
- A good name sometimes had to give way to one that hashes right. Matching pairs such as x/y
  cannot always sit on either side of another local. So a few functions use `incH`/`incV`,
  `spawnCol`/`spawnRow` or `row`/`col` instead of the obvious pair.
- Parameters and register variables are safe to rename: their places do not depend on the
  table.
- Uninitialised `static` variables at file scope are laid out the same way. A module with
  several of them keeps their address names and gets a readable alias:
  `#define TalkStrings D_8CCE`.

**Lesson:** when a compiler stores symbols in a hash table, identifiers can be code. Before a
mass rename, prove on a few functions that renaming alone leaves the binary unchanged. If it
does not, find out why before going on.

## 3. Rename tokens, never text

`tools/rename.py` applies `tools/rename_map.json`, one map per program. It works on a C token
stream (`tools/ctok.py`), not on regular expressions:

- **Identifiers** are replaced in code and in preprocessor lines.
- **Strings** are left alone.
- **Comments** only change for whole-word names that look technical.
- **Old addresses** stay in a comment in front of each renamed definition and declaration
  (`/*C_2F7E*/WaitKeyOrTimeout()`). The source can still be matched with a disassembly, and
  with any notes keyed to the old names.
- **Assembly** names are matched case-insensitively, with or without the leading `_` of cdecl
  names. A label at column 0 that had an address name keeps it as a comment.
- **The CGA driver** (`SRC-DRV/`) only gets the shared `Gra_*` names, because its own labels are
  its own addresses.

Each readable pass was then checked with a token diff against the previous version. The only
allowed differences were consistent identifier renames and literals replaced by constants of
the same value. That rule was kept strictly, even where the compiler would not care
(`u4_putc(8)` stayed rather than becoming `'\b'`). A strict rule keeps the check mechanical: a
diff either passes or needs a look.

**Lesson:** one map owns the global names, and every edit has a definition of "allowed"
that a script can check. Neither the build nor a reviewer has to rely on reading diffs by eye.

## 4. Splitting the work

The readable pass of about 40 C files went to five workers, each with its own set of files and
its own copy of the tree. The rules:

- **One owner per file.** No two workers ever touched the same file, so merging was copying.
- **Global names are not the workers' to change.** They come from `rename_map.json` only.
  Workers reported the global names they found misleading, and those reports were applied
  centrally in a second pass.
- **Each worker verifies its own files** with `localorder.py`, the token diff and the DOS build,
  one file at a time. The build of all the merged files was checked again at the end.
- **Give each worker its own scratch folder.** A shared scratch area got its helper scripts
  (`tokdiff.py`, `old_*.C`) overwritten by another worker.

**Lesson:** parallel work on a byte-exact tree is safe when the boundaries are files and the
check is per file.

## 5. Name in two passes

The global names came first, from the call graph and what each function visibly did. They were
good enough to read the code, but reading every function in full while commenting it showed
that some were wrong:

| first name | better name | why |
|---|---|---|
| `MoveVortices` | `VortexContacts` | nothing moves there; it applies the whirlpool's and twister's contact |
| `ShowTownMap` | `ShowAreaMap` | also used outdoors, for the gem and the View spell |
| `COM_CheckHitable` | `COM_MissileStep` | it moves the missile one square |
| `cursor_rate` | `cursor_delay` | it slows the cursor loop in `u_delay` |
| `KBD_0e7f` | `KBD_CTRL_BS` | 0E7Fh is Ctrl+Backspace |

**Lesson:** name globals early so the reading can start, and plan a second pass for them once
every function has been read. The people who read a function's whole body are the best judges
of its name, so ask them for corrections as part of the job.

## 6. Line endings and the old dialect

- MASM 3.00 needs CRLF line endings, and the sources are CRLF throughout. `tools/crlf.py` fixes
  a file. Git converting line endings in a working copy makes every file look modified. A
  `.gitattributes` saying which files are CRLF would have avoided that.
- MSC 4.00 has no `//` comments, and the sources stay ASCII.
- Several files are compiled with `/J` (char is unsigned), as `SRC/MAKEFILE` says file by file.
  The same expression can mean different things in two files.

## 7. A byte-identical build proves less than it seems

The DOS build proves that names and comments did not change the code. It does not prove that
a name is right. It also cannot tell whether a cast added for another compiler has the meaning
intended. In `StealGold`, MSC compiles `gold - amount >= 0` as `gold >= amount`. A cast meant to
reproduce a 16-bit wrap of the subtraction therefore compiled to the same bytes and still
described something MSC never computes. The port's notes cover this in more detail.

**Lesson:** for any change made for its meaning, read the disassembly. A matching hash only
says the change was neutral.

## 8. Reading everything finds bugs; record them, do not fix them

Commenting every function meant understanding it, and that turned up about 50 probable bugs in
the original game, listed in [`FINDINGS.md`](../FINDINGS.md). They fall into a few families,
worth looking for in any code of this age:

- **Unsigned tests that can never be true.** `karma_dec`'s `<= 0` on an unsigned virtue, so a
  virtue wraps to about 65530. `DamageAllParty`'s `(Party._ship -= 10) < 0`, so the ship never
  sinks there.
- **`||` for `&&` and the reverse.** `NpcMoveInTown`'s bounds check, and `DNG_SpawnMonsters`
  excluding the party's whole row and column.
- **Precedence.** `FreeNpcSlot`'s `si >= (SavedMode == MOD_OUTDOORS) ? 8 : 0`.
- **A variable compared with itself.** `DNG_Render`'s `Saved_forward3 == Saved_forward3`.
- **16-bit overflow.** 14 wands at 5000 gp cost 70000, which becomes 4464 gp.
- **Out-of-range sentinels.** `AskChara` returns -2, the healer tests -1 and uses
  `Party.chara[-2]`.
- **Reads before a buffer or before a value is stored.** `u4_gets` on an empty answer, and the
  shops' sell loop reading `answer`.

None was fixed in the code, since a fix would change the program. Each has a comment at the
spot as well as its entry in `FINDINGS.md`.

**Lesson:** keep the readable source faithful, and keep the bug list separate and exhaustive.
That list is what a later fixed version or a port needs, and what a reader of the source wants
to know.

## 9. What the headers still lack

The workers listed literals they left in place for want of a constant:

- the dungeon tile kinds (high nibble: 10h-30h ladders, 40h chest, 80h trap, A0h field, C0h
  door, D0h room, E0h secret door, F0h wall);
- `Party._loc == 0` (outdoors);
- the `GraphicsType` values (1 CGA, 2 EGA, 3 Tandy);
- reagent indexes and the stones' bits;
- `TITLE.EXE` has no tile constants at all.

These are the obvious next step for readability. Adding them is safe, since constants compile
to the same code.

## 10. Reusing tools from earlier decompilations

Several tools came from the same work on Ultima VI, which was built with Turbo C 2.0: the
tokenizer, the width conversion and the DOS build. They saved a great deal of time. Their
compiler-specific assumptions did not carry over, though, and some docstrings still say
"Turbo C". Everything about MSC 4.00 had to be found again:

- the hash-ordered locals and statics;
- the unsigned-preserving promotions;
- byte comparisons of chars;
- the `/J` files;
- the order of pascal-call arguments.

**Lesson:** reuse the machinery, but re-verify every rule that comes from the compiler on the
new one before relying on it.
