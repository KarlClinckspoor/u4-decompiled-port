# Findings from the readability pass

Every C file was read in full while its locals were renamed and its functions commented. This
file collects what came up: probable bugs in the original game, and odd behaviour worth knowing.
**Nothing listed here was changed in the code**, because the sources must keep compiling to the
original `AVATAR.EXE` and `TITLE.EXE`. Most items also have a comment at the spot.

All names are the new names. `tools/rename_map.json` maps them back to the old placeholder
names (`C_xxxx`, `D_xxxx`, ...), and the old address is still written in front of every renamed
definition.

## Probable bugs in AVATAR.EXE

### Utilities, files, start-up (U4_UTIL, U4_FILE, U4_MAIN, U4_INIT)
* `karma_dec`: the virtue is unsigned, so `<= 0` only catches 0. A virtue lower than the amount
  wraps round to about 65530 instead of stopping at 1.
* `DamageAllParty`: `Party._ship` is unsigned, so `(Party._ship -= 10) < 0` is never true. The
  ship never sinks there and the hull value wraps round; `EndOfTurn` then never repairs it, since
  it is not below 50.
* `PartyDeath`: after closing `File_DNG` it clears `File_TLK` instead of `File_DNG`.
* `u4_putl`: with a negative value there is no return after printing '-' and the absolute
  value, so the padding and the rest are printed as well.
* `u4_gets` (also in TITLE): for an empty or all-blank answer, the trailing-space loop reads
  `buf[-1]` and clears the bytes before the buffer as long as they are spaces.
* `Load`: the file is not closed when the read fails (the callers exit).
* `AskForDisk`: `key &= 0xf` maps drive letter P to drive 0, while A-O become 1-15, so P: cannot
  be chosen (harmless).
* `karma_inc`/`karma_dec` are declared `(char *, int)` in U4.H but defined `(U16 *, unsigned)`;
  U4_USE.C casts `&Party._hones` to `char *`.

### Use, talk, shops (U4_USE, U4_TALK, U4_SHOPS)
* `USE_Stones`: outside a dungeon altar room it prints "No place to Use them!" but still calls
  `USE_StonesAtAltar`, which usually adds "Hmm...No effect!".
* `TLK_join`: always removes NPC #31 from the map, not the one who joined. This only works because
  the companions are the last NPC of their town.
* `TLK_Say` is declared with one parameter, but most calls pass an extra number first. With the
  pascal convention this is harmless (the function still reads its text; see the comment there).
  `TLK_give` ends with `TLK_Say(6, 0)`, which prints nothing.
* `TalkToCitizen`: the "turns away / attacks" roll happens before every topic, even "bye".
* `CMD_Talk`: calls `FindLocalNpcAt` before its bounds check (harmless).
* `WeaponShopDeal` / `ArmourShopDeal`: price × count is computed in 16 bits and wraps: 14 wands
  cost 5000 × 14 = 70000, which becomes 4464 gp.
* `WeaponShopSell` / `ArmourShopSell`: if the first item asked for is not owned, the loop test
  reads `answer` before anything was stored in it. The prompts also accept 'P' and 'H', so mystic
  weapons and armour can be sold (3500 / 4500 gp).
* `HealerAskWho`: `AskChara` returns -2 for '0', but `HealerCure`, `HealerHeal` and
  `HealerResurrect` only test for -1, and then use `Party.chara[-2]`.
* `PubAle`: when no extra gold is given, the test checks `money` instead of `extra`, so the
  "could not hep ya" message is always shown.
* `SHP_food`: the pack count is a `char` holding `AskInt(3)` (up to 999): 200 reads as -56
  ("Too bad") and 300 as 44.
* Not bugs, but surprising: `SHP_reagent` gives +2 Honesty, Justice and Honor even after
  underpaying (on top of the penalty); sleeping at an inn also cures poison ('P' becomes 'S' then
  'G').

### Moving about, NPCs, map (U4_MAP, U4_NPC, U4_EXPLO, U4_ANIM)
* `NpcMoveInTown`: the bounds check uses `||` where `&&` was meant, so a move is only refused
  when it is off the map on both axes.
* `NpcStepOutdoors`: in the first random retry the `(char)` cast is on the sign instead of the
  random number, so dy is never -1 and the retry is biased southward.
* `LoadMapChunk` frees the loaded chunk before checking whether it is the one requested, so the
  same chunk is read again every time.
* `WindAllows` slows sailing with the wind behind (3 moves in 4) as well as into the wind (1 in 4).
* `CMD_Open` and `CMD_Jimmy` only work east or west.
* `LoadTown` takes the map from its parameter but the .TLK file from `Party._loc`.

### Combat (U4_COMBA, U4_COMBB, U4_COMBC, U4_AI, U4_SPELL)
* `MonsterIndex`: for people (tiles below the monsters) the index runs past `MonsterMaxHP` into
  `MonsterPartner`, so most townsfolk get the wrong hit points (the comment there lists them).
* `PlaceFoes`: for monsters, the count is overwritten by `U4_RND1(7)` before the group-size
  lookup, which then reads `MonsterGroupSize[MonsterIndex(0)]` (padding, 0). Monster group sizes
  are never used: usually 2-8 foes, one time in 8 a single foe.
* `FreeNpcSlot`: operator precedence, `si >= (SavedMode == MOD_OUTDOORS) ? 8 : 0`. Outdoors the
  search reaches monster slots 1-7 and returns slot 0 when the list is full.
* `CombatEnd` / `CombatSpoils` test `SavedMode != MOD_COM_CAMP`, but `SavedMode` never holds
  that value (`CurMode` was probably meant): victories in camp and inn ambushes also leave spoils,
  in the inn case at the stale `CombatFoeX/Y`.
* `EnterDungeonRoom`: `if(room._010)` tests an array's address, which is always true (harmless:
  empty slots hold tile 0).
* `TwisterContact` resets `hit_tile` without ever setting it (copied from `WhirlpoolContact`).
* `CMD_HoleUp` clears 32 entries of the 16-entry `Fighters._tile`, which also clears `_gtile`.
* `DNG_SpawnMonsters` uses `Party._x != x && Party._y != y`, which excludes the party's whole row
  and column; `||` (only the party's square) was probably meant.
* `FoeHitsChara`: the damage goes through `(d >> 4) * 10 + d % 10`, a broken BCD conversion
  (`d & 0xf` would make it a real one).
* `FoeMelee` returns 1 only when the character is hit and survives (the value is unused).
* `FoeCanMoveTo` reads `Combat_MAP(y, x)` before checking that the square is on the map.
* `COM_Shoot`: with oil at range 0 the foe variable is used uninitialised.
* `CMD_Attack` compares an NPC's tile with `TIL_SHALLOWS`, which never matches an NPC.
* `SPL_Energy`: in a dungeon, when the square ahead is not empty it prints "Failed!" but still
  puts the field there (no `return`).
* `SPL_Tremor`: a foe that survives may have its HP set to 23, which heals foes that had fewer.
* `CMD_Cast` uses up the mixture before checking the MP.
* `COM_DoDamage` prints "Fleeing!" below an absolute 24 HP, whatever the foe's maximum.

### Dungeons, the 3D view (U4_DNG, U4_3D, U4_PEER, U4_GET, U4_MIX)
* `DNG_Render`: `Saved_forward3 == Saved_forward3` compares the variable with itself, so a change
  three squares ahead never forces a redraw.
* `ShowDungeonMap`: the 600-byte flood-fill stack wraps to 0 when it overflows, which can lose
  pending squares or end the fill early.
* `ChestTrap`: the trap value is always 0 or 2, so only acid or poison traps occur; sleep and
  bomb traps never do.
* `OpenChest`: if no chest object is found, the search ends at slot -1 and clears it (out of
  bounds; unreachable in practice).
* `CMD_Mix`: choosing a reagent you have none of ends the mix without giving back the reagents
  already added.

### Lord British, shrines, the end (U4_LB, U4_SHRIN, U4_END)
* `LB_Help`: the "no partial Avatarhood yet" test combines the eight virtues with bitwise `&`
  instead of `&&`: it is false whenever the values share no bit, even if none is 0.
* `LB_LevelUp` sets the status of a levelled-up character to Good, which also revives a dead one.
* `Victory` ends in `while(1);`.

## Probable bugs in TITLE.EXE
* `NewGame`: the sex answer is `u_kbread() & 0xff`, so its comparisons with `KBD_ENTER` and
  `KBD_SPACE` (scan code + character) can never match; only Escape cancels.
* `Load` (FILE.C): the error paths return without freeing the buffer or closing the file (the
  callers exit).
* `RedrawTitle`: exit code 0x3B is used for two different allocation failures.
* DATA.C: two places in the view script write `V_MKU8(0,x)` where `V_MKU8_bis(0,x)` is meant
  (same byte).
