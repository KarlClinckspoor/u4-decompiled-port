# Ultima IV: the game's mechanics, read from its source

This is how Ultima IV (the PC version, `AVATAR.EXE` and `TITLE.EXE`) actually works: time,
conversation, quest progression, character stats and combat. It is read from the decompiled C
in this repository, not from the manual or from play. Where the two disagree, this is what the
program does.

**Conventions**

- **Locations** are given as `FILE.C:Function`, in `SRC/` (the game) or `SRC-TITLE/` (the title
  and character creation).
- **Random numbers.** `rand` means `u_rand_a()`, a byte from 0 to 255. The source has four
  helpers, and the difference matters:
  - `U4_RND1(m)` is `rand & m`, a **bit mask**. `U4_RND1(7)` is a uniform 0-7. A mask like
    `0x77` only gives values whose masked-out bits are 0.
  - `U4_RND3(n)` is `rand % n`.
  - `U4_RND4(n)` is the same, but 0 when n is 0.
  - `U4_RND2(m)` is `u_rand_b() & m`, a second generator.

  Probabilities below are exact over the 256 values of `rand`.
- **Marks.**
  - **[inferred]** marks a conclusion drawn from how several pieces of code interact, rather than
    read from one line.
  - **[data]** marks something read from the game's data files (GOG version: `PARTY.NEW`, the
    `.TLK` and `.ULT` files).
  - **(FINDINGS)** means the probable bug is already listed in [FINDINGS.md](../FINDINGS.md).
  - The others, collected in [section 6](#6-surprises-and-probable-bugs), came up while writing
    this.
- **Indexes.** Virtues, classes, virtue towns and shrines share one index:

  | # | Virtue | Class | Town | Mantra |
  |---|---|---|---|---|
  | 0 | Honesty | Mage | Moonglow | ahm |
  | 1 | Compassion | Bard | Britain | mu |
  | 2 | Valor | Fighter | Jhelom | ra |
  | 3 | Justice | Druid | Yew | beh |
  | 4 | Sacrifice | Tinker | Minoc | cah |
  | 5 | Honor | Paladin | Trinsic | summ |
  | 6 | Spirituality | Ranger | Skara Brae | om |
  | 7 | Humility | Shepherd | Magincia | lum |

Contents: [1. Time](#1-time) · [2. Conversation](#2-conversation) ·
[3. Quest progression](#3-quest-progression) · [4. Stats](#4-stats) · [5. Combat](#5-combat) ·
[6. Surprises and probable bugs](#6-surprises-and-probable-bugs)

---

## 1. Time

### 1.1 Two clocks

The game keeps two unrelated kinds of time:

- **Turns.** Each command ends with `EndOfTurn` (`U4_MAIN.C`), `DNG_EndOfTurn` (`U4_DNG.C`) or
  the end of a combat round. Turns drive:
  - food, poison and MP;
  - monsters moving and appearing;
  - spell durations and torches;
  - the move counter `Party._moves`.
- **Redraws.** `RefreshView` (`U4_ANIM.C`) runs on every keyboard poll, including all the time
  the game waits for a key. Redraws drive:
  - **the moons and the moongates, wind changes and balloon drift**;
  - every animation.

  Each of these counters is scaled by `speed_info`. That value is measured at start-up
  (`LOW.ASM`): iterations of a calibration loop per timer tick, / 1000, from 1 to 32, +1 on CGA.
  It keeps the real-time speed roughly constant on any PC.

So waiting at the prompt outdoors moves the moons and the wind but not the world. Every poll
also advances the random generator (`UTIL2.ASM:u_kbhit`), so random outcomes depend on how long
you wait between keys.

### 1.2 What a turn is

- **Almost everything costs a full turn.** In the main loop (`U4_MAIN.C:main`), only an unknown
  key ("Bad command!") is free. Ztats, Volume, Locate, New order and a cancelled or failed
  command all end the turn. A whole conversation is one turn.
- **Doing nothing passes.** The prompt waits with `u_delay(25, 1)`. With no key within about
  25 seconds of real time, the command is Space: "Pass", one turn. The same holds in dungeons
  and for each character in combat.
- **A sleeping party passes turns as fast as the loop runs**, printing "Zzzzz", with no delay.
- **Dungeons** (`U4_DNG.C:DNG_main`): turning left or right is free. Commands that do not exist
  there ("Not Here!") cost a turn.
- **Galloping** (Yell toggles it, `U4_EXPLO.C:CMD_Yell`): one arrow key moves the horse twice
  for one turn.
- **Ships:** turning to a new heading costs a turn.
- **Combat** (`U4_COMBA.C:COM_main`):
  - Each conscious member gets one command per round. A bad key asks again, for free.
  - Then the foes act.
  - Then the round's bookkeeping runs (1.3).
- **Tile effects come before the command.** The effect of the tile under the party
  (`U4_EVT.C:TileEffects`) runs at the top of every loop. A free "Bad command" keypress on a
  swamp, a field or a bridge therefore rolls its poison, damage or trolls again.

### 1.3 What happens at the end of a turn

| | World and towns (`EndOfTurn`) | Dungeon (`DNG_EndOfTurn`) | Combat |
|---|---|---|---|
| Ship hull +1 (1 in 4, up to 50) | yes, aboard or not | no | no |
| A sleeper wakes (1 in 8 each) | yes | yes | after each own turn slot |
| Poison: −2 HP | yes | yes | **no** |
| Food | 1/100 per member | same | 1/100 per member turn |
| Starving: −2 HP to each living member | yes | yes | no |
| MP +1 per living member | yes | yes | once per round |
| Monsters move and appear | yes, not while flying | yes | the foes act |
| `Party._moves` +1, spell duration −1 | yes | yes | once per round |
| Light −1 | no | yes | no |
| An open door closes | after 4 more turns (towns) | no | no |

- **Food** (`U4_UTIL.C:food_dec`):
  - Food is a `long` in hundredths; the status line shows `food / 100`. One unit of food lasts
    100 turns for one person, 12.5 turns for eight.
  - **Dead members still eat**, because the cost is `Party._members`.
  - At 0 the game prints "Starving!!!".
  - The cap is 9999 food (`U4_SHOPS.C:SHP_food`), bought in packs of 25.
- **Poison** never wears off and can kill: −2 HP each turn outside combat. Cures:
  - the Cure spell;
  - a healer;
  - Lord British;
  - a curing fountain;
  - a night at an inn.

  **Camping does not cure it.**
- **MP** (`U4_UTIL.C:MP_recover`) comes back at +1 per turn, up to the class maximum (4.4).
- **HP never comes back by itself.** HP comes only from camping, inns, spells, fountains, Lord
  British and level-ups.
- **Hull:** `if(Party._ship < 50 && U4_RND1(3) == 0) Party._ship++`, so about one point per 4
  turns. It repairs only up to 50; above that (the Wheel sets 99) it never changes by itself.
- **Doors** (`U4_EXPLO.C:CMD_Open`, `AutoCloseDoor`): only one door can be open at a time, and
  it shuts at the end of the 4th turn after the one that opened it [inferred: even with the
  party in the doorway].

### 1.4 Effects that last

There is **one** effect slot (`spell_effect`, with `spell_turns`). A new effect replaces the old
one. It is counted down once per world or dungeon turn, or per combat round.

| Effect | From | Lasts | What it does |
|---|---|---|---|
| Jinx (J) | spell | 10 | a foe moving onto another foe hits it for 0-63 (`U4_AI.C:CheckJinx`) |
| Negate (N) | spell | 10 | **every** spell fails, the party's own included; foes' magic bolts and the Balron/Reaper sleep are blocked |
| Negate (N) | a Zorn | 2 | renewed every round the Zorn acts, overwriting e.g. Protection |
| Protection (P) | spell | 10 | half of the foes' melee blows miss before the armour roll |
| Quickness (Q) | spell | 10 | after each turn of a conscious member, 1 in 2 of another turn (it can repeat) |
| Horn (1) | the Silver Horn | 10 | no daemons at the Abyss approach |

**Dungeon light** shares a word with the balloon's "flying" flag (`Party._fly_light`).
- A torch (`U4_GET.C:CMD_Ignite`) and the Light spell add 100 turns each, with no cap.
- `DNG_EndOfTurn` takes 1 per turn. At 0 the game prints "It's Dark!".
- The Winds trap (0x80) puts it out, and leaving the dungeon sets it to 0.

### 1.5 The moons and the moongates

Read from `U4_ANIM.C:UpdateMoons`, `UpdateOpenGate` and `U4_MAP.C:CheckMoongate`.

**Speed.**
- The moons advance only while the **outdoor** view is drawn. `UpdateMoons` is called from
  `DrawOutdoors`, so they stand still in towns, dungeons, combat and shrines.
- A sub-step happens every `speed_info/2 + 1` redraws, and four sub-steps make a step.
- **Trammel** changes phase every 16 steps. **Felucca** changes every 5-6 steps, so it goes round
  three times per Trammel cycle. The pair repeats every 128 steps.
- [inferred] With keys typed ahead there is about one redraw per turn, so a Trammel phase lasts
  roughly 64 turns at low `speed_info`. Waiting makes the phases pass faster per turn.

**Phases are not saved.**
- The counters start at 0 when the program starts and are never loaded from `PARTY.SAV`. The
  saved `Party._trammel` and `_felucca` are only ever written.
- So **every restore starts with both moons new**. Searches that need "both moons new"
  (mandrake, nightshade, the Skull, the Black Stone) therefore always work right after loading.

**The open gate** is the one of **Trammel's** phase (`MoongateX/Y`, `U4_MAIN2.C`):

| Phase | Gate (x, y) | Near |
|---|---|---|
| 0 | 224, 133 | Moonglow |
| 1 | 96, 102 | Britain |
| 2 | 38, 224 | Jhelom |
| 3 | 50, 37 | Yew |
| 4 | 166, 19 | Minoc |
| 5 | 104, 194 | Trinsic |
| 6 | 23, 126 | Skara Brae |
| 7 | 187, 167 | Magincia |

**Opening and closing.** The gate grows over the first step of a phase and shrinks over the
last. It is never absent; it only dwindles to its smallest frame between phases. Only the full
gate (tile 0x43) can be walked onto. [inferred] So a gate that is opening or closing blocks
you.

**Going through** works on foot or on horseback only (`CheckMoongate`):
- When both moons are at phase 4, you go to the **Shrine of Spirituality**. That means the
  Minoc gate at the right moment, about 21 of every 512 sub-steps [inferred].
- Otherwise you arrive at the gate of **Felucca's** phase at the moment you step in.

The Gate spell goes to the gate you name (1-8), whatever the moons.

### 1.6 Wind, sailing and the balloon

- **The wind changes** (`U4_ANIM.C:UpdateWind`) on redraws, outdoors and in towns:
  - each check has a 1 in 64 chance (`!U4_RND1(0xfc)`), with one check every `speed_info`
    redraws;
  - the wind then turns 90° left or right, never staying the same and never reversing at once;
  - it starts from the north on every load, and the Winds spell sets it.
  - `WindDir` is where the wind comes **from**.
- **Sailing** (`U4_MAP.C:WindAllows`) has no random roll; it uses the move counter:
  - into the wind: a move succeeds when `_moves & 3 == 0`, 1 turn in 4;
  - with the wind behind: 3 turns in 4 (FINDINGS: probably meant to be every turn);
  - across the wind: always.
  - A failed move ("Slow progress!") still costs the turn. Pirate ships obey the same rule.
- **The balloon** (`U4_ANIM.C:DriftBalloon`) drifts one square downwind every `speed_info + 1`
  redraws. This happens in real time, even while you wait.
  - Each drift adds a move but **no turn**: no food, no monsters. While flying, `EndOfTurn` also
    skips monsters, vortices and tile effects.
  - It lands only on grass (`U4_EXPLO.C:CMD_Descend`).

### 1.7 Monsters and people over time

**Outdoors**, in `U4_NPC.C`.

Appearing (`SpawnMonsters`):
- There is a 1 in 16 chance per turn, into slots 0-3, on a random square at least 6 squares away
  on **both** axes.
- **On land** (grass, scrub, forest, hills) the monster is `TIL_ORC + 4 × (rand & rand & mask)`.
  - The mask grows with the move counter: 3 below 10 000 moves, 7 below 30 000, 15 after.
  - Each mask bit is set 1 time in 4, so the low entries dominate:

    | Moves | Monsters (chance) |
    |---|---|
    | < 10 000 | orc 56 %, skeleton 19 %, rogue 19 %, python 6 % |
    | < 30 000 | orc 42 %; skeleton, rogue, ettin 14 % each; python, headless, cyclops 5 % each; wisp 1.6 % |
    | ≥ 30 000 | orc 32 %; skeleton, rogue, ettin, evil mage 10.5 % each; python, headless, cyclops, liche, lava lizard, daemon 3.5 % each; wisp, zorn, hydra, dragon 1.2 % each; balron 0.4 % |

  - **The difficulty clock is `_moves`.** Combat rounds and balloon drift also advance it.
- **At sea**, 1 in 8 for each slot:
  - pirate ship 2/8;
  - nixie, squid, serpent, seahorse, whirlpool, twister 1/8 each.

Movement (`MoveWorldNpcs`):
- A monster leaving the 32×32 window is deleted.
- Any monster next to the party (Manhattan distance < 2) starts a combat.
- Serpents, lava lizards, hydras, dragons and balrons within 4 squares breathe fire half the
  time. A hit makes each member take 10-24 HP 1 time in 2 (`U4_UTIL.C:DamageAllParty`); a ship
  takes 10 hull instead.
- Pirate ships fire broadsides.
- Whirlpools, twisters and, 3 times in 4, non-evil creatures move at random. The rest step
  toward the party.
- A whirlpool on the party moves it to (127, 78) in Lock Lake; a twister damages it four times
  (`U4_COMBB.C:VortexContacts`).

Scripted events:
- 8 pirate ships at (221, 224).
- 8 daemons when walking south near the Abyss, unless the Horn is active.
- 1 in 8 "Bridge Trolls!" per turn on a bridge.

**Dungeons** (`U4_AI.C:DNG_SpawnMonsters`, `DNG_MoveMonsters`):
- Each level keeps up to two wandering monsters, picked every turn on an empty corridor.
- Level z draws from four kinds starting at z: rat, bat, spider, ghost, slime, troll, gremlin,
  (mimic: never), reaper, insects, gazer.
- They **wander and never chase**. They can walk onto the party, which starts a combat.
- Monsters placed in the dungeon's map file never move [inferred: they get no slot].
- The rooms hold the strong fixed groups.

**Towns** have **no schedules and no day or night**. Each person has a fixed movement type from
the `.ULT` file (`U4_NPC.C:MoveTownNpcs`):
- standing;
- wandering (a step 1 turn in 2);
- following;
- following and attacking (0xFF). Guards and Lord British switch to this after you attack in
  town, as does an angered citizen (2.4).

### 1.8 Resting and cooldowns

All cooldowns are keyed to `Party._moves`, and most are **blocks**, not intervals. "Once per
100-move block" means `_moves / 100` must differ from the last time. So moves 199 and 200 are
in different blocks.

| What | Rule | Source |
|---|---|---|
| Camp (Hole up) | on foot, outdoors or in a dungeon; waits 10 s; ambush 1 in 8 (orc, skeleton, rogue, python, insects, slime, troll or wisp, with the awake members put to sleep). Otherwise once per 100-move block, each living member: wakes, **+99 to +218 HP** (`HP_inc(U4_RND1(119)) + HP_inc(99)`), **MP to its maximum**. Poison stays. | `U4_HOLE.C:CMD_HoleUp` |
| Inn | living members wake cured with **+100 to +198 HP** (even values), but **MP only +1**; then 1 in 8 a rogue ambush (Skara Brae: 1 in 4 the ghost Isaac). One turn. | `U4_SHOPS.C:InnNight` |
| Meditation | once per 100-move block, **shared with Hawkwind** | `U4_SHRIN.C`, `U4_SHOPS.C:SHP_hawkwind` |
| Humility answer, gift to a beggar | the karma gain at most once per 16-move block (`_moves >> 4`), **one timer shared by every question and every gift** | `U4_TALK.C` |
| Mandrake and nightshade | both moons new and `(_moves & 0xF0) != _lastFound`; 2-9 each time. [inferred] The test compares bits 4-7 only, so the same block comes round again every 256 moves. | `U4_SRCH.C:FindMandrake` |

### 1.9 No quest clock

There is no calendar, no day or night, no daily event and no time limit. `_moves` is shown
when you Quit and in the victory text ("in N turns"). Otherwise it only sets monster
difficulty, sailing and the cooldowns above.

---

## 2. Conversation

### 2.1 Talking to someone

Read from `U4_TALK.C:CMD_Talk` and `TalkToCitizen`.

**When you can talk:**
- Talk reaches the adjacent square in a direction. Across a shop counter (letter tiles), it
  reaches one square further.
- It does not work outdoors or from the balloon.
- Someone with no talk record says "Funny, no response!".
- Lord British has his own routine (2.7), and merchants open their shop. The shop is found by
  the row of its sign only.

**The talk record.** Each town's `.TLK` file holds 16 records of 288 bytes. A person's record
number is stored in the town's `.ULT` file.

| Part of the record | Meaning |
|---|---|
| byte 0 | the topic after which the yes/no question is asked (an index into the topic list) |
| byte 1 | nonzero: the question is a humility question |
| byte 2 | the turn-away chance, out of 256 |
| 12 strings | name, pronoun, description, job, health, answers to keywords 1 and 2, the question, the answers to Yes and No, keyword 1, keyword 2 |

When you meet someone, they say their name half the time (`U4_RND1(1)`).

### 2.2 How keywords match

- You type up to 10 characters. An empty line ends the talk.
- The topics are tried **in this order**, and the first match wins: bye, name, look, job, health,
  keyword 1, keyword 2, join, give.
- The comparison is `strnicmp(topic, input, 4)`: case is ignored and at most four letters
  count.
  - **Words of four letters or more:** the first four letters are enough ("heal" means health,
    "names" means name).
  - **Shorter topics must be typed exactly.** The comparison includes the topic's end, so "bye",
    "job" and three-letter keywords ("BEG", "OAK", ...) match only the exact word: "jobs" fails.
- Nothing matched: "That I cannot help thee with."
- [data] Two personal keywords can never be reached, because they are "HEAL" and "health" comes
  first: Michelle in Serpent's Hold and Calabrini in Moonglow.

### 2.3 The yes/no question

Read from `U4_TALK.C:TLK_Question`.

- **When it is asked:** after the topic named by byte 0, **every time** that topic is asked.
- **The answer:** only the first letter of the reply counts. An empty answer leaves without
  effect.
- **Humility questions** (byte 1 set). [data] Every one of them is a "brag" question ("Art thou
  proud?" and the like), 21 people in all.
  - **Yes: Humility −5**, every time, with no limit.
  - **No: Humility +10**, at most once per 16-move block.
- **The shared timer.** Every answered question, humility or not, resets the 16-move timer, and
  gifts to beggars use the same one. So answering an ordinary question uses up the next humility
  bonus.
- **Other questions** only choose between two answers.
- [data] Three questions are never asked:
  - Shamino's (Skara Brae) and Charm's (Cove): their byte 0 is 0, "after bye".
  - Alkerion's (Minoc): his byte 0 is 10, which is no topic.

### 2.4 Turning away and attacking

Before each line you type is looked up, `bye` and unknown words included (FINDINGS), the game
rolls:

```c
if((kw = u_rand_a()) < TalkData[2]) {
    if(TalkData[2] - kw >= 0x40) /* "On guard! Fool!" -> the person attacks */
    else                         /* "<pronoun> turns away!" */
    return;
}
```

With T the record's byte 2, **each line** ends the talk with chance T/256. The person attacks
with chance max(0, T − 63)/256, by switching to "follow and attack" (1.7).

[data] Most people have T = 0. The values above 0:

| T | People |
|---|---|
| 16 | Damon (Minoc), Slysam and Ragnar (Buccaneer's Den) |
| 20 | the Lycaeum guard |
| 32 | nine people, e.g. Joe and Cricket (Britain) |
| 50 | the reaper in Lord British's castle |
| 64 | three people; 0.4 % attack |
| 96 | Vorpal (Yew); 12.9 % attack |
| 128 | Brigant (Buccaneer's Den); 25 % turn away and 25 % attack, per line |

### 2.5 Joining the party

`U4_TALK.C:TLK_join` checks these conditions in order:

1. **Who.** It must be talk record 1 of one of the **eight virtue towns**, and not the town of
   your own class (that companion is you).
2. **Virtue.** The town's virtue must be **40 or more, or 0**. Partial Avatarhood is *not*
   needed.
3. **Experience.** The Avatar's **max HP must be at least 100 × (the new party size)**. Max HP is
   100 × level, so **the party can never be larger than the Avatar's level**. The eighth member
   needs level 8, which means 6400 XP.

[data] The companions are Mariah, Iolo, Geoffrey, Jaana, Julia, Dupre, Shamino and Katrina.
Joining costs and gives no karma. Nobody can be dismissed.

### 2.6 Giving gold

`U4_TALK.C:TLK_give`:

- **Who:** only beggars (tile 0x58) accept.
- **Amount:** at most two digits, so 1-99 gold.
- **Effect: Compassion +2, whatever the amount** (1 gold is enough), at most once per 16-move
  block on the shared timer.
- **Giving never raises Sacrifice**, whatever the shrine's vision says.

### 2.7 Lord British

Read from `U4_LB.C:TalkToLB`.

- **The first visit** is only his introduction: no healing, no levels.
- **Every later visit:**
  - he revives a dead Avatar;
  - he heals everyone not dead to full HP, status Good;
  - he raises levels (4.4). **This is the only way to gain a level.**
- **"health"**, answered No, heals the party again, as often as you like.
- **"help"** gives the **first** hint that applies:
  1. fewer than 1000 moves: know thyself, stay near the towns;
  2. alone: build a party;
  3. no rune: find the runes, shrines and mantras;
  4. "no virtue achieved yet": visit Hawkwind. **This test is broken** (FINDINGS): it ANDs the
     eight values bit by bit. The starting values (50 = 0x32, 65 = 0x41, ...) share no bit, so
     the hint is skipped at the start;
  5. no stone: find the stones;
  6. not all eight virtues achieved: seek elevation in all eight;
  7. bell, book or candle missing;
  8. a key part missing: the Key of Three Parts and the Word of Passage;
  9. otherwise: ready for the Abyss, "go only with a party of eight". This does not check party
     size or the Word of Passage.

### 2.8 Hawkwind the seer

Hawkwind is reached by talking across the counter on row 0x19 of Lord British's castle, a
special case in `U4_TALK.C:EnterShop`. The routine is `U4_SHOPS.C:SHP_hawkwind`.

- **When he speaks:** only if the Avatar is conscious.
- **What you ask:** a virtue name. Only its first four letters count.
- **What he says** depends on your value v in that virtue:

  | v | Answer |
  |---|---|
  | 0 | you are a partial Avatar in it |
  | below 20, 20-39, 40-59, 60-98 | one of four messages |
  | 99 (or a wrapped value, 3.1) | "Go to the Shrine and meditate for three Cycles!" |

- **Leaving him gives Spirituality +3**, even if you asked nothing. This happens at most once per
  100-move block, **on the meditation timer**. A visit therefore blocks meditation until the next
  block, and the other way round.

### 2.9 Pubs

Read from `U4_SHOPS.C:SHP_pub` and `PubAle`.

- **Ale:** at most 3 per visit. Paying 3 gold or more lets you ask about a topic.
- **Topics and prices:** black stone 20, sextant 30, white stone 10, mandrake 40, skull 99,
  nightshade 25. You type the whole phrase.
- **Which pub answers what:** a pub answers only the topics from its own position in that list
  onward.
  - Britain answers all of them; Jhelom from the sextant on.
  - Trinsic from the white stone; Paws from the mandrake.
  - Buccaneer's Den only the skull and nightshade; Vesper only nightshade.
- **Paying for the answer:** the gold already paid, the ale included, counts toward the price.
  If you give up, it is kept.
- There is no karma effect.

### 2.10 The gypsy

The character creation questions are a knock-out between virtues. Their effect on the stats is
in 4.2.

---

## 3. Quest progression

### 3.1 How virtues are stored

The eight virtues are 16-bit **unsigned** numbers in the party record. They start at 50 and
normally range from 1 to 99. **0 means partial Avatarhood in that virtue.** Two functions in
`U4_UTIL.C` change them:

```c
karma_inc: if(*karma) { *karma += amount; if(*karma > 99) *karma = 99; }
karma_dec: if(*karma == 0) { *karma = 99; /* "Thou hast lost an Eighth!" */ }
           *karma -= amount; if(*karma <= 0) *karma = 1;
```

- **An achieved virtue (0) never goes up.**
- **Any loss on an achieved virtue** costs the eighth: the virtue restarts at 99 − amount.
- **Gains cap at 99.**
- **The wrap (FINDINGS).** Because the value is unsigned, `<= 0` only catches exactly 0. A loss
  bigger than the value wraps it to about 65 530. The next gain then clamps it to **99**, exactly
  the value the shrine needs. Two ways to exploit it, read from the code but not tried in play:
  - **Reagent shops.** Underpaying costs Honesty, Justice and Honor `shortBy/3` each (at least
    4). That is immediately followed by the +2 for buying. An order of 891 gold paid with 1 gold
    therefore sets all three to 99.
  - **Bragging.** Keep answering Yes to a humility question until Humility wraps, three more
    times after that, then answer No once: Humility becomes 99.

### 3.2 Everything that changes a virtue

This covers all 49 call sites of `karma_inc` and `karma_dec`. The only other writes are
character creation and the shrine.

| Action | Virtue | Change | Condition | Source |
|---|---|---|---|---|
| Attack someone outside combat | Compassion, Justice, Honor | −5 each | the target is non-evil, **or** you are in a town or castle (then the guards and Lord British turn hostile) | `U4_COMBA.C:CMD_Attack` |
| A non-evil foe flees the combat map | Compassion, Justice | +1 each | per foe | `U4_AI.C:FoeFlees` |
| A party member is killed by a foe | Sacrifice | +1 | | `U4_AI.C:FoeHitsChara` |
| A member leaves the combat map | Valor, Sacrifice | −2 each | normal combat, evil foe, **and that member at full HP** | `U4_COMBB.C:COM_CharaExits` |
| The party leaves the field against a non-evil foe | Compassion, Justice | +2 each | not a dungeon room | `U4_COMBC.C:CombatEnd` |
| The party leaves the field against an evil foe ("Battle is lost!") | Valor | −2 | not a dungeon room | `U4_COMBC.C:CombatEnd` |
| Victory over an evil foe | Valor | +0 or +1 (`U4_RND1(1)`) | | `U4_COMBC.C:CombatEnd` |
| Open a chest that belongs to a town map | Honesty, Justice, Honor | −1 each | not monster loot, not in a dungeon | `U4_GET.C:OpenChest` |
| Underpay a reagent shop | Honesty, Justice, Honor | −`shortBy/3` (at least 4) each | | `U4_SHOPS.C:SHP_reagent` |
| Buy reagents | Honesty, Justice, Honor | +2 each | also after underpaying (FINDINGS) | `U4_SHOPS.C:SHP_reagent` |
| Give blood at a healer (offered at 400+ HP) | Sacrifice | Yes +5 (−100 HP), No −5 | | `U4_SHOPS.C:HealerBlood` |
| Leave Hawkwind | Spirituality | +3 | once per 100-move block | `U4_SHOPS.C:SHP_hawkwind` |
| Meditate without elevation | Spirituality | +3 per cycle | once per 100-move block | `U4_SHRIN.C:EnterShrine` |
| Wrong mantra | Spirituality | −3 | | `U4_SHRIN.C:EnterShrine` |
| Elevation | the shrine's virtue | set to 0 | 3.3 | `U4_SHRIN.C:EnterShrine` |
| Any successful Search on the surface | Honor | +5 | each find | `U4_SRCH.C:SearchFound` |
| Find a dungeon stone | Honor | +5 | | `U4_SRCH2.C:FindStone` |
| Humility question, Yes | Humility | −5 | no limit | `U4_TALK.C:TLK_Question` |
| Humility question, No | Humility | +10 | once per 16-move block | `U4_TALK.C:TLK_Question` |
| Give gold to a beggar | Compassion | +2 | once per 16-move block (same timer) | `U4_TALK.C:TLK_give` |
| Use the Skull anywhere but the Abyss | all eight | −5 each | the Skull is lost | `U4_USE.C:USE_Skull` |
| Use the Skull at the Abyss entrance | all eight | +10 each | the Skull is destroyed | `U4_USE.C:USE_Skull` |

What the shrines preach but the code does not check:
- killing a non-evil creature once combat has started;
- attacking a fleeing foe;
- giving your last gold (no Sacrifice);
- fleeing while hurt (only fleeing at full HP costs);
- anything in a dungeon room.

Nothing in conversation changes Honesty.

### 3.3 Shrines and partial Avatarhood

`U4_SHRIN.C:EnterShrine` checks these, in order:

1. **Entry:** you need the shrine's **rune**.
2. **The virtue:** you name it in full (case ignored). A wrong name gives "unable to focus",
   with no cost.
3. **Cycles:** you choose 1-3.
4. **Cooldown:** once per 100-move block, shared with Hawkwind. The slot is used up from here on,
   even if the meditation then fails.
5. **The mantra:** each cycle takes about 16 seconds of real time, then asks for it (1.1's
   table). A wrong mantra costs Spirituality −3 and ends the meditation.
6. **The result:**
   - **Elevation** comes with **3 cycles and the virtue at exactly 99**. The virtue becomes 0,
     and a rune picture is shown; the eight spell I-N-F-I-N-I-T-Y. No Spirituality is gained.
   - **Otherwise** you get a vision and Spirituality +3 per cycle. The 3-cycle visions give the
     order of the Abyss items (bell, book, candle) and where the White and Black stones are.

Nothing else is required: no stone, no companion. The Shrine of Spirituality is reached only
by moongate, when both moons are full (1.5).

### 3.4 Items and where they come from

`Party.mItems` bits hold the skull, the cast skull, candle, book, bell, the three key parts,
horn and wheel, plus the bell, book and candle "used" bits. `mRunes` and `mStones` have one bit
per rune and per stone. **No bit is ever cleared, except the Skull's.**

**Searching** (`U4_SRCH.C:SearchSpots`) needs the exact square. Every find gives Honor +5; the
XP goes to the first member.

| Where | x, y (hex) | What | Condition | XP |
|---|---|---|---|---|
| World | B6,36 · 64,A5 | Mandrake ×2-9 | both moons new; cooldown (1.8) | – |
| World | 2E,95 · CD,2C | Nightshade ×2-9 | same | – |
| World | B0,D0 | Bell of Courage | once | 400 |
| World | 2D,AD | Silver Horn | once | 400 |
| World | 60,D7 | Wheel of the H.M.S. Cape | once | 400 |
| World | C5,F5 | Skull of Mondain | both moons new, not held, never cast | 400 |
| World | E0,85 | Black Stone | both moons new (the Moonglow gate's square) | 200 |
| World | 40,50 | White Stone | once (the shrine hints at the Serpent's Spine) | 200 |
| Lycaeum | 06,06 | Book of Truth | once | 400 |
| Cove | 16,01 | Candle of Love | once | 400 |
| Lycaeum | 16,03 | Telescope (view a town's map) | – | – |
| Empath Abbey | 16,04 | Mystic Armour ×8 | none owned and all eight virtues 0 | 400 |
| Serpent's Hold | 08,0F | Mystic Weapons ×8 | the same | 400 |
| Moonglow 08,06 · Britain 19,01 · Jhelom 1E,1E · Yew 0D,06 · Minoc 1C,1E · Trinsic 02,1D · LB's castle 11,08 · Paws 1D,1D | | the runes of Honesty … Humility | once each | 100 |

**Dungeon stones** come from searching an altar (0xB0) once (`U4_SRCH2.C`): +5 Honor, 200 XP.

| Dungeon | Stone |
|---|---|
| Deceit | Blue |
| Despise | Yellow |
| Destard | Red |
| Wrong | Green |
| Covetous | Orange |
| Shame | Purple |

Hythloth and the Abyss have none.

**Dungeon orbs** raise stats (4.4).

**Using items** (`U4_USE.C`):
- **Bell, book and candle:** only at the Abyss entrance (233, 233), and only in that order.
  Each sets its "used" bit; none is consumed.
- **Horn:** blocks the daemons for 10 turns.
- **Wheel:** aboard a ship with its hull at exactly 50, it sets the hull to 99. It can be used
  again.
- **Skull:** see 3.2.
  - Used anywhere else, it also wipes every creature on the map except Lord British. It is then
    lost without being marked as cast, so **it can be found again**.
  - Used at the entrance, it is marked as cast.
- **Key:** works by itself in the Codex chamber.

**The Key of Three Parts** comes from the altar rooms (room 15 of the dungeons). Which part a
room gives depends only on the dungeon's x position:

| x | Part | Stones to name (any order, all owned) |
|---|---|---|
| 3 | Love | White, Orange, Green, Yellow |
| ≤ 2 | Truth | White, Purple, Green, Blue |
| other | Courage | White, Purple, Orange, Red |

The active member must stand at (5, 5) of the room. Each part is given once, and the stones are
not used up. The White stone is needed for all three parts and the Black for none. The rooms'
exits lead into other dungeons (`U4_COMBC.C:AltarRoomExits`).

### 3.5 The Abyss and the Codex

**Entering** (`U4_EXPLO.C:EnterAbyss`): you need all six bits, bell, book and candle each
**owned and used**. The used bits are never cleared, so the entrance stays open once opened.

**The altar on each level** (`U4_USE.C:USE_Stones`): you name the virtue and then the stone. A
wrong answer does nothing and costs nothing. A right one turns the altar into a ladder down.

| Level | Asked: the virtue of | Answer | Stone |
|---|---|---|---|
| 1 | Truth | Honesty | Blue |
| 2 | Love | Compassion | Yellow |
| 3 | Courage | Valor | Red |
| 4 | Truth and Love | Justice | Green |
| 5 | Love and Courage | Sacrifice | Orange |
| 6 | Courage and Truth | Honor | Purple |
| 7 | Truth, Love and Courage | Spirituality | White |
| 8 | none of the three | Humility | Black, which opens the Codex chamber |

**The Codex chamber** (`U4_END.C:EndGame`): the Avatar goes on alone. The checks, in order:

1. **All three key parts**, otherwise you are sent back to the surface.
2. **The Word of Passage**, **veramocor**: three tries.
3. **A party of eight.** The size is counted before the Avatar goes on, so dead members count.
4. **All eight virtues at 0.** The Skull's fate is **never checked**: casting it is optional.
5. **Eleven questions**, three tries each:
   - the eight virtues, in order;
   - then Truth, Love and Courage.

   Failing one sends you back to the surface, to a place that depends on the question.
6. **The last question:** **infinity**.
7. **Victory** prints the number of turns, then the program stops in an endless loop (FINDINGS).

Being sent back costs nothing: no karma, items or flags.

### 3.6 The shortest path the code allows

1. **The runes:** find them.
2. **The virtues:** bring each to exactly 99, then meditate three cycles at its shrine.
3. **The items:**
   - bell, book and candle;
   - the six dungeon stones, the White stone and the Black stone.
4. **The key:** the three key parts from the altar rooms.
5. **The party:** level the Avatar to 8 (6400 XP) and recruit all seven companions. Each needs
   the virtue of their town at 40 or more (or 0).
6. **The Abyss:** at its entrance, ring the bell, read the book, light the candle and Enter.
   Then the eight altars and the Codex.

---

## 4. Stats

### 4.1 The character record

`struct tChara` in `U4.H`, eight of them in the party record. Slot 0 is the Avatar.

| Field | Range | Notes |
|---|---|---|
| HP, max HP | 0..max; max = 100 × level (100-800) | only Lord British changes max HP |
| XP | 0..9999 | |
| STR, DEX, INT | 15-28 at creation, at most 50 | |
| MP | 0..class maximum (≤ 99) | |
| weapon, armour | 0-15, 0-7 | the item held is not in the party's stock |
| sex, class | | |
| status | **G**ood, **P**oisoned, **S**leeping, **D**ead | alive = G/P/S; able to act = G/P |

**There is no level field.** The level is max HP / 100.

### 4.2 Character creation

Read from `SRC-TITLE/TITLE_1.C:GypsyReading` and `CreateParty`.

**The knock-out.**
- STR, DEX and INT start at 15, and every virtue at 50.
- The gypsy asks seven questions as a knock-out between pairs of virtues: four, then two between
  the winners, then the final.
- Each choice gives the chosen virtue +5 and its stat bonus:

  | Virtue | STR | DEX | INT |
  |---|---|---|---|
  | Honesty | | | +3 |
  | Compassion | | +3 | |
  | Valor | +3 | | |
  | Justice | | +1 | +1 |
  | Sacrifice | +1 | +1 | |
  | Honor | +1 | | +1 |
  | Spirituality | +1 | +1 | +1 |
  | Humility | | | |

**The result.**
- **Virtues:** always one at 65 (the winner, which sets your class and town), one at 60, two at
  55 and four at 50.
- **Stats:** the winner's bonus counts three times, the runner-up's twice and the semi-finalists'
  once. STR + DEX + INT ends between 57 and 66.
- **What else the Avatar starts with** is the class's template in `PARTY.NEW` [data]: its HP,
  XP, MP, weapon and armour. A Shepherd starts at level 1 with 100 HP, a Paladin at level 3 with
  a sword and chain mail.
- **The party** starts with 300.99 food, 200 gold, 2 torches, 3 ginseng and 4 garlic [data].

### 4.3 The companions [data]

| Name | Class | Level (max HP) | XP | STR | DEX | INT | Weapon | Armour |
|---|---|---|---|---|---|---|---|---|
| Mariah | Mage | 2 (200) | 125 | 9 | 12 | 20 | Staff | Cloth |
| Iolo | Bard | 3 (300) | 240 | 16 | 19 | 13 | Sling | Cloth |
| Geoffrey | Fighter | 3 (300) | 205 | 20 | 15 | 11 | Axe | Leather |
| Jaana | Druid | 2 (200) | 175 | 17 | 16 | 13 | Dagger | Cloth |
| Julia | Tinker | 2 (200) | 110 | 15 | 16 | 12 | Mace | Leather |
| Dupre | Paladin | 3 (300) | 325 | 17 | 14 | 17 | Sword | Chain |
| Shamino | Ranger | 2 (200) | 150 | 16 | 15 | 15 | Sword | Leather |
| Katrina | Shepherd | 1 (100) | 5 | 11 | 12 | 10 | Staff | Cloth |

### 4.4 Levels, HP and MP

**Level** (`U4_LB.C:LB_LevelUp`):
```c
for(newMax = limit = 100; limit <= XP; newMax += 100, limit <<= 1);
```

| XP | 0 | 100 | 200 | 400 | 800 | 1600 | 3200 | 6400 |
|---|---|---|---|---|---|---|---|---|
| Level | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |

**Gaining a level** happens only when visiting Lord British (2.7). For each member whose level
has risen:
- max HP and HP go to the new maximum;
- the status becomes Good, which **revives the dead** (FINDINGS);
- STR, DEX and INT each gain **1-8** (`U4_RND1(7) + 1`), capped at 50.

The stats are raised **once per visit**, however many levels were gained.

**XP** goes to the member who made the kill: the monster's base HP / 16 + 1, so 4-16 (5.7).
Quest finds give XP to the first member. Kills by fields or Jinx give none; missile spells and
Tremor give the XP to the caster.

**Maximum MP** (`U4_UTIL.C:MP_recover`), capped at 99:

| Class | Max MP | At INT 50 |
|---|---|---|
| Mage | 2 × INT | 99 |
| Bard, Paladin, Ranger | INT | 50 |
| Druid | INT + INT/2 | 75 |
| Tinker | INT / 2 | 25 |
| Fighter, Shepherd | 0 | 0 |

INT matters **only** here: there is no spell failure roll (5.10).

**Dungeon orbs** (`U4_SRCH2.C:TouchBall`) give +5 (cap 50) to the stats of the dungeon, and
damage the one who touches it:

| Dungeons | Stats | Damage |
|---|---|---|
| Deceit, Despise, Destard | INT, DEX, STR respectively | 200 |
| Wrong, Covetous, Shame | DEX+INT, STR+DEX, STR+INT respectively | 400 |
| Hythloth | all three | 600 |

The damage can kill.

### 4.5 Who can use what

From `U4_LOCAT.C:ClassMasks`. Hands and skin are always allowed.

| Weapon | Classes |
|---|---|
| Staff, Dagger, Sling, Flaming Oil, Mystic Sword | all |
| Mace | all but Mage |
| Axe, Sword | Bard, Fighter, Tinker, Paladin, Ranger, Shepherd |
| Bow, Crossbow | Bard, Fighter, Druid, Tinker, Paladin, Ranger |
| Halberd | Fighter, Tinker, Paladin |
| Magic Axe | Tinker, Paladin |
| Magic Sword | Fighter, Tinker, Paladin, Ranger |
| Magic Bow | Bard, Druid, Tinker, Paladin, Ranger |
| Magic Wand | Mage, Bard, Druid |

| Armour | Classes |
|---|---|
| Cloth, Mystic Robe | all |
| Leather | all but Mage |
| Chain Mail, Plate Mail | Fighter, Tinker, Paladin |
| Magic Chain | Fighter, Paladin |
| Magic Plate | Paladin |

---

## 5. Combat

### 5.1 How a combat starts

| Trigger | Foe | Source |
|---|---|---|
| You Attack an adjacent creature | it (in town, the guards and Lord British turn hostile) | `U4_COMBA.C:CMD_Attack` |
| A monster ends its move next to you, outdoors | it | `U4_NPC.C:MoveWorldNpcs` |
| A hostile townsperson reaches you | them | `U4_NPC.C:MoveTownNpcs` |
| A bridge, 1 in 8 per turn | trolls | `U4_EVT.C:TileEffects` |
| A dungeon square holding a monster | that monster | `U4_DNG.C:DNG_CheckEncounter` |
| A dungeon room | its fixed group | `U4_COMBC.C:EnterDungeonRoom` |
| Camp, 1 in 8 | a random weak group, the awake put to sleep | `U4_HOLE.C:CMD_HoleUp` |
| An inn, 1 in 8 | rogues | `U4_SHOPS.C:InnNight` |

**The creature is removed from the map when the combat starts**
(`U4_COMBC.C:StartCombat`). So it is gone whatever the outcome, fleeing included [inferred].

The battlefield is an 11×11 `.CON` map chosen by the terrain under you and under the foe: ship
or shore, marsh, brush, forest, hill, brick, bridge, grass, or one of seven dungeon maps.

### 5.2 How many foes

Read from `U4_COMBC.C:PlaceFoes`.

- **Against a town guard:** twice the party size in guards.
- **Against another townsperson:** that one person.
- **Against monsters:** **1 to min(8, 2 × party size), uniformly.** The table of group sizes is
  never used, because of a bug (FINDINGS).
- **Partners.** Each foe but one may be replaced by the monster's partner (12.1 %) or the
  partner's partner (3.1 %). Partners are table entries, e.g. orc → troll → ettin → daemon →
  balron.
- **Foe HP** is `(base >> 1) | rand % base`. It can exceed the base, and the base is in 5.7.

Foe HP is a single byte, **unsigned** in the DOS program. The files involved are compiled with
`/J` or cast to `unsigned char`. A build that ignores `/J` would get foes with 128+ HP wrong.
The native port keeps it.

### 5.3 A combat round

1. **The party acts.** Each member on the field who can act (G or P), in order, gets one
   command: move, Attack, Cast, Get a chest, Ready a weapon, Use, Volume, Ztats or Pass. A member
   who waits 25 seconds passes.
2. **After each member's slot:**
   - the tile under the member takes effect (5.8);
   - the dead are removed from the field;
   - under Quickness, 1 in 2 of another turn;
   - a sleeper wakes 1 in 8;
   - 1/100 food is eaten.
3. **The foes act**, from slot 15 down to 0 (5.6).
4. **End of the round:**
   - MP +1;
   - `_moves` +1;
   - the spell effect is counted down.

**Slow terrain** can waste a move (`U4_MAP.C:IsSlowTerrain`):

| Terrain | Chance of wasting the move |
|---|---|
| Swamp | 1 in 8 |
| Scrub, forest | 1 in 4 |
| Hills, fire field | 1 in 2 |

**Leaving:**
- Stepping off the edge takes a member off the field, alive.
- In a dungeon room, everyone must use the same exit, and **the room never ends by itself**: you
  have to walk out.
- The combat ends when no member is on the field, or (outside rooms) no foe is left.

### 5.4 The party attacks

Read from `U4_COMBA.C:TryDamage` and `COM_DoDamage`.

```c
if(Party._loc == LOC_ABYSS && weapon < WPN_MAGIC_AXE)       -> miss
if(dex < 40 && U4_RND1(0xff) > dex + 0x80)                  -> miss
damage = rand % min(str + WeaponDamage[weapon], 255);
```

**To hit:** with DEX 40 or more the party **never misses**. Below 40, the miss chance is
(127 − DEX)/256:

| DEX | 15 | 20 | 25 | 30 | 39 | 40+ |
|---|---|---|---|---|---|---|
| Miss | 43.8 % | 41.8 % | 39.8 % | 37.9 % | 34.4 % | 0 % |

The foe's kind has no defence value.

**Damage:**
- It runs from 0 to STR + weapon − 1 (at most 254), so a blow can do 0.
- **A foe dies only when the damage exceeds its HP.** A foe left at exactly 0 survives.
- Lord British cannot be hurt.
- **In the Abyss**, every weapon below the magic axe always misses.

**The message after a blow that does not kill:**
1. "Fleeing!" below **24 HP**, a fixed number (FINDINGS);
2. otherwise "Critical!" below a quarter of the base;
3. then "Heavily", "Lightly" or "Barely Wounded".

**Weapons** (`U4_COMBC.C:WeaponDamage`, `WeaponIsRanged`):

| Weapon | Value | Kind | Damage at STR 15 / 50 | Price |
|---|---|---|---|---|
| Hands | 8 | melee | 0-22 / 0-57 | – |
| Staff | 16 | melee | 0-30 / 0-65 | 20 |
| Dagger | 24 | melee; thrown if nobody is adjacent | 0-38 / 0-73 | 2 |
| Sling | 32 | ranged | 0-46 / 0-81 | 25 |
| Mace | 40 | melee | 0-54 / 0-89 | 100 |
| Axe | 48 | melee | 0-62 / 0-97 | 225 |
| Sword | 64 | melee | 0-78 / 0-113 | 300 |
| Bow | 40 | ranged | 0-54 / 0-89 | 250 |
| Crossbow | 56 | ranged | 0-70 / 0-105 | 600 |
| Flaming Oil | 64 | thrown 0-9 squares; leaves fire | 0-78 / 0-113 | 5 |
| Halberd | 96 | **only the square two away** | 0-110 / 0-145 | 350 |
| Magic Axe | 96 | ranged; comes back | 0-110 / 0-145 | 1500 |
| Magic Sword | 128 | melee | 0-142 / 0-177 | 2500 |
| Magic Bow | 80 | ranged | 0-94 / 0-129 | 2000 |
| Magic Wand | 160 | ranged | 0-174 / 0-209 | 5000 |
| Mystic Sword | 255 | melee | 0-254 | – |

How the weapons behave (`U4_COMBA.C:COM_Attack`, `COM_Shoot`):
- **Attacks** go in the four straight directions only.
- **Ranged weapons** fly up to 11 squares to the first foe. Walls, doors, rocks, mountains and
  energy fields stop them; water and the other fields do not.
- **The halberd** steps once before its check, so it hits only the square two away, **never an
  adjacent foe**.
- **Daggers and oil are used up** when thrown.
  - If no spare is left, the game says "Last one!" and switches the member to Hands **before**
    the blow. So the last one does bare-hand damage, and the last oil leaves no fire.

### 5.5 Armour and the foes' attacks

Read from `U4_AI.C:FoeMelee`, `FoeHitsChara` and `FoeShoot`.

**Melee:**
```c
if((spell_effect == 'P' && U4_RND1(1)) || ArmorDefense[armor] >= rand) -> miss
damage = U4_RND4(base >> 2);  damage = (damage >> 4) * 10 + damage % 10;
```

| Armour | Defence | Chance the foe's blow hits | Price |
|---|---|---|---|
| Skin | 96 | 62.1 % | – |
| Cloth | 128 | 49.6 % | 50 |
| Leather | 144 | 43.4 % | 200 |
| Chain Mail | 160 | 37.1 % | 600 |
| Plate Mail | 176 | 30.9 % | 2000 |
| Magic Chain | 192 | 24.6 % | 4000 |
| Magic Plate | 208 | 18.4 % | 7000 |
| Mystic Robe | 248 | 2.7 % | – |

Protection halves these.

**Damage:**
- It depends only on the monster's base HP, never on its strength or current HP.
- The second line is a broken decimal conversion (FINDINGS). It squeezes the damage hard:

  | Base HP | Maximum damage |
  |---|---|
  | up to 64 | 9 |
  | up to 128 | 19 |
  | up to 192 | 29 |
  | above 192 | 39 |

- **A member dies only when the damage exceeds their HP**, so a member can be alive at 0 HP.

**Missiles** (1 in 4 per turn, for monsters that have one) fly toward the nearest member, even
diagonally. **Armour does not count against them.**
- **Poison and sleep missiles** do no damage. They poison or put to sleep a member in good
  status, 1 time in 2.
- **The others** (energy, fire, lava, magic bolt, rocks, missile, flash) do the melee damage
  above.

### 5.6 How foes behave

Read from `U4_AI.C:COM_FoesTurn` and `FoeAct`. Each foe, in turn:

1. **Asleep:** it wakes 1 in 8; otherwise it does nothing. Damage does not wake it.
2. **Specials:** a Zorn casts Negate on the party for 2 rounds. A Wisp teleports, 1 time in 8.
3. **Target:** the nearest member.
4. **Action:**
   1. A **Mimic** more than 4 squares away stays a chest.
   2. **1 in 4:** the foe shoots its missile, if it has one.
   3. Otherwise **1 in 4:** a **Balron or Reaper** casts sleep. Each member in good status falls
      asleep 1 time in 2.
   4. **Below 24 HP:** it flees, and leaving the map counts as fleeing (3.2).
   5. **Next to its target:** a Gremlin steals 25 food, a Rogue steals 0-63 gold 1 time in 4.
      Then it strikes.
   6. **Otherwise:** it moves toward the target. Reapers and Mimics never move.
5. **The field under the foe** takes effect:
   - **fire or lava:** 0-127 damage, except to lava lizards, daemons, hydras, dragons and balrons;
   - **poison field:** 0-127 damage;
   - **sleep field:** a foe falls asleep with chance (HP + 1)/256. So **healthy foes fall asleep
     more often**, the reverse of the Sleep spell.

There is **no** splitting, regeneration, invisibility or summoning in the code. The special
abilities are the ones listed in 5.7.

### 5.7 Monsters

From `U4_COMBC.C:MonsterMaxHP`, `MonsterPartner` and `U4_AI.C`.

| Column | Meaning |
|---|---|
| Base | the monster's base HP |
| HP | the range of `(base >> 1) \| rand % base` |
| XP | base / 16 + 1 |
| Melee | the damage range after the decimal conversion |
| Move | **W** walks, **S** swims, **F** flies, **P** passes through walls, – never moves |
| Good | non-evil (`IsNonEvil`): it matters for karma |

| Monster | Base | HP | XP | Melee | Missile | Move | Notes |
|---|---|---|---|---|---|---|---|
| Pirate ship | 255 | 127 or 255 | 16 | 0-39 | – | – | fought as rogues |
| Nixie | 64 | 32-63 | 5 | 0-9 | missile | S | |
| Squid | 96 | 48-127 | 7 | 0-19 | energy | S | |
| Sea Serpent | 128 | 64-127 | 9 | 0-19 | flash | S | breathes outdoors |
| Seahorse | 96 | 48-127 | 7 | 0-19 | magic bolt | S | Good |
| Rat | 48 | 24-63 | 4 | 0-9 | – | W | Good |
| Bat | 48 | 24-63 | 4 | 0-9 | – | F | Good |
| Spider | 64 | 32-63 | 5 | 0-9 | poison | W | Good |
| Ghost | 80 | 40-111 | 6 | 0-19 | – | P | undead |
| Slime | 48 | 24-63 | 4 | 0-9 | – | W | |
| Troll | 96 | 48-127 | 7 | 0-19 | missile | W | |
| Gremlin | 48 | 24-63 | 4 | 0-9 | – | W | steals food |
| Mimic | 192 | 96-255 | 13 | 0-29 | poison | – | looks like a chest |
| Reaper | 255 | 127 or 255 | 16 | 0-39 | poison, energy, fire or sleep | – | casts sleep |
| Insects | 48 | 24-63 | 4 | 0-9 | – | F | Good |
| Gazer | 240 | 120-255 | 16 | 0-39 | sleep | W | |
| Phantom | 128 | 64-127 | 9 | 0-19 | – | W | undead |
| Orc | 80 | 40-111 | 6 | 0-19 | – | W | |
| Skeleton | 48 | 24-63 | 4 | 0-9 | – | W | undead |
| Rogue | 80 | 40-111 | 6 | 0-19 | – | W | steals gold |
| Python | 48 | 24-63 | 4 | 0-9 | poison | W | Good |
| Ettin | 112 | 56-127 | 8 | 0-19 | rocks | W | |
| Headless | 64 | 32-63 | 5 | 0-9 | – | W | |
| Cyclops | 128 | 64-127 | 9 | 0-19 | rocks | W | |
| Wisp | 64 | 32-63 | 5 | 0-9 | – | W | teleports |
| Mage | 176 | 88-255 | 12 | 0-29 | magic bolt | W | |
| Liche | 192 | 96-255 | 13 | 0-29 | magic bolt | W | undead |
| Lava Lizard | 96 | 48-127 | 7 | 0-19 | lava | W | fire-proof; breathes outdoors |
| Zorn | 240 | 120-255 | 16 | 0-39 | – | P | Negate |
| Daemon | 112 | 56-127 | 8 | 0-19 | magic bolt | F | fire-proof |
| Hydra | 208 | 104-255 | 14 | 0-39 | flash | W | fire-proof; breathes outdoors |
| Dragon | 224 | 112-255 | 15 | 0-39 | flash | F | fire-proof; breathes outdoors |
| Balron | 255 | 127 or 255 | 16 | 0-39 | poison, energy, fire or sleep | F | fire-proof; immune to Sleep; casts sleep |

The whirlpool and twister are never fought. A base of 255 gives HP of 127 or 255 only.

**People fought in towns** read their HP past the end of the monster table (FINDINGS:
`MonsterIndex`). So a beggar has a base of 196 (HP 98-255) and a ranger 32 (HP 16-31, already
"fleeing"). Lord British cannot be hurt.

### 5.8 Fields, traps and chests

The table shows what each hazard does in each place. Read from `U4_EVT.C:TileEffects` and
`U4_UTIL.C:DamageAllParty`.

| Hazard | In combat (the active member) | Outdoors and in towns (each member) | Dungeon |
|---|---|---|---|
| Swamp, poison field | **always** poisoned (good status) | poisoned 1 in 8 | as outdoors |
| Sleep field | always asleep | asleep 1 in 4 | 1 in 4 |
| Fire field, lava | 16-47 HP | 10-24 HP, 1 in 2 | 10-24 HP, 1 in 2 |
| Energy field | cannot be entered | – | cannot be entered |
| Traps 0x80-0x8F | – | – | winds put out the light; rocks and pits do 10-24 HP, 1 in 2 |

**Chests** (`U4_GET.C:ChestTrap`):
- Half are not trapped, 37.5 % hold acid (0-29 HP) and 12.5 % poison. Sleep and bomb traps never
  occur (FINDINGS).
- The opener avoids the trap when `rand % 100 <= DEX + 25`: 48 % at DEX 15, 81 % at DEX 50.
- The Open spell avoids traps outside combat.
- A chest holds 10-96 gold.

### 5.9 Death and healing

**One member dies** when damage exceeds their HP: status D, HP 0. They stay in the party and
keep eating. Ways back:

| Way | Effect | Source |
|---|---|---|
| Healer | 300 gold; **revived at 0 HP** | `U4_SHOPS.C:HealerResurrect` |
| Resurrect spell | **revived at 0 HP** | `U4_SPELL.C` |
| Lord British | revives the Avatar only, at full HP; a level-up revives anyone at full HP (FINDINGS) | `U4_LB.C` |

[inferred] A member revived at 0 HP dies again from any damage.

**The whole party dies** (`U4_UTIL.C:PartyDeath`). After "All is Dark…":
- **Where:** you wake in Lord British's castle with full HP.
- **What is lost:**
  - **The spare** weapons and armour are lost. Each member **keeps what they hold and wear**.
  - Gold becomes 200 and food 200.
  - Spells, the light, the balloon and any ship or horse are gone.
- **What is kept:** XP, levels, stats, karma, reagents, mixtures, quest items and the move
  counter.

**Healers** (`U4_SHOPS.C`):

| Service | Cost | Effect |
|---|---|---|
| Cure | 100 gold, **free with less than 100** | poison cured |
| Heal | 200 gold | HP to full |
| Resurrect | 300 gold | revived at 0 HP |

Camp, inns and Lord British are in 1.8 and 2.7.

### 5.10 Magic

**Casting** (`U4_SPELL.C:CMD_Cast`):
1. **The mixture** must be ready. It is **used up immediately**.
2. **MP:** with too few, "M.P. too low!", and the mixture is lost (FINDINGS).
3. **The spell's checks:** where it can be cast, and its target.
4. **Spending:** the MP is spent, then the spell fails if Negate is on.

**There is no random failure and no INT check.** A spell cast on the wrong target (Cure on
someone healthy) still costs the mixture and the MP.

**Mixing** (`U4_MIX.C:CMD_Mix`):
- The **set** of reagents must equal the recipe; adding one twice wastes it but still works.
- A wrong set loses the reagents. Escape returns them.
- Choosing a reagent you have none of loses the ones already added (FINDINGS).

Reagent abbreviations: Ash = sulfurous ash, Gin = ginseng, Gar = garlic, Silk = spider silk,
Moss = blood moss, Pearl = black pearl, Shade = nightshade, Man = mandrake.

| | Spell | MP | Reagents | Where | Effect |
|---|---|---|---|---|---|
| A | Awaken | 5 | Gin, Gar | anywhere | wakes a member |
| B | Blink | 15 | Silk, Moss | outdoors, on foot or horse | jumps as far as possible in a direction |
| C | Cure | 5 | Gin, Gar | anywhere | cures poison |
| D | Dispel | 20 | Ash, Gar, Pearl | not from the balloon | removes a field |
| E | Energy | 10 | Ash, Silk, Pearl | dungeon, combat | creates a poison, lightning, fire or sleep field |
| F | Fireball | 15 | Ash, Pearl | combat | missile, 24-127 |
| G | Gate | 40 | Ash, Pearl, Man | outdoors, on foot or horse | to a moongate |
| H | Heal | 10 | Gin, Silk | anywhere | +75-99 HP |
| I | Iceball | 20 | Pearl, Man | combat | missile, 32-255 |
| J | Jinx | 30 | Pearl, Shade, Man | anywhere | 10 rounds: foes hurt each other |
| K | Kill | 25 | Pearl, Shade | combat | missile, 232 damage |
| L | Light | 5 | Ash | dungeon | +100 turns of light |
| M | Magic Missile | 5 | Ash, Pearl | combat | missile, 16-63 |
| N | Negate | 20 | Ash, Gar, Man | anywhere | 10 rounds: no magic at all, the party's included |
| O | Open | 5 | Ash, Moss | not from the balloon | opens a chest safely |
| P | Protection | 15 | Ash, Gin, Gar | anywhere | 10 rounds: half the foes' blows miss |
| Q | Quickness | 20 | Ash, Gin, Moss | anywhere | 10 rounds: extra turns |
| R | Resurrect | 45 | Ash, Gin, Gar, Silk, Moss, Man | not in combat | revives, **at 0 HP** |
| S | Sleep | 15 | Gin, Silk | combat | each foe (not undead, not Balron) sleeps with chance (255 − HP)/256 |
| T | Tremor | 30 | Ash, Moss, Man | combat | each foe below 192 HP: killed 1 in 2, set to 23 HP 1 in 4 (which can heal it, FINDINGS) |
| U | Undead | 15 | Ash, Gar | combat | each undead, 1 in 2: down to 23 HP (it flees) |
| V | View | 15 | Shade, Man | anywhere | the map |
| W | Winds | 10 | Ash, Moss | outdoors | sets the wind |
| X | X-it | 15 | Ash, Silk, Moss | dungeon | leaves the dungeon |
| Y | Y-up | 10 | Silk, Moss | dungeon, not the Abyss | up a level |
| Z | Z-down | 5 | Silk, Moss | dungeon, not the Abyss | down a level |

Several recipes are shared, and the mixture is for the spell you named while mixing:
- Awaken = Cure;
- Fireball = Magic Missile;
- Heal = Sleep;
- Open = Winds;
- Blink = Y-up = Z-down.

Missile spells hit the first foe in line and give XP. Tremor and Undead set 23 HP, just under
the 24 HP at which foes flee.

---

## 6. Surprises and probable bugs

These came up while writing this document and are not in [FINDINGS.md](../FINDINGS.md). They
are read from the code; the ones marked [inferred] combine several lines. None has been tried in
the running game.

**Time**
1. **The moons restart at "both new" on every load.** Their counters are never read back from
   the save (1.5).
2. **Free keypresses still trigger tile effects.** A "Bad command" costs no time, but the tile
   effects (swamp, fields, bridge trolls) run again (1.2).
3. **Dead members eat.** Poison and starvation do no damage during combat (1.3).
4. **Balloon drift and combat rounds** advance the move counter, and with it monster difficulty,
   without eating food (1.6).
5. **The hull** repairs while you are not aboard, but never in dungeons (1.3).
6. **Dungeon monsters:** pre-placed ones never move, and wanderers can step onto the party [inferred] (1.7).
7. **The reagent cooldown** repeats every 256 moves [inferred] (1.8).
8. **An inn night restores 1 MP**, although the comment says the MP is recovered (1.8).
9. **Moongates:** one that is not fully open blocks movement [inferred] (1.5).

**Conversation and quest**

10. **Shared timers.** One timer covers every yes/no question and every gift, so an ordinary
    question uses up the next Humility or Compassion bonus (2.3). Hawkwind and the shrines share
    the meditation timer (2.8).
11. **Unreachable text** [data]. Two "HEAL" keywords are shadowed by "health", and three
    questions are never asked (2.2, 2.3).
12. **Two ways to reach 99** through the `karma_dec` wrap: underpaying reagent shops, and
    bragging (3.1).
13. **The Skull of Mondain** can be found again after a wrong use, and casting it into the Abyss
    is never required (3.4, 3.5).

**Combat**

14. **The halberd** cannot hit an adjacent foe (5.4).
15. **The last dagger or oil** does bare-hand damage, and the last oil makes no fire (5.4).
16. **The bat's partner** is lava lizard tile 0xE9 instead of 0xE8. [inferred] Such a lizard
    cannot spit lava and is not fire-proof.
17. **Balrons' and Reapers' missiles** include sleep fields, which the code comment leaves out
    (5.6).
18. **Sleep fields** put healthy foes to sleep more often than weak ones, the reverse of the
    Sleep spell (5.6).
19. **A party death** keeps each member's held weapon and worn armour, and loses only the spares
    (5.9).
20. **Alive at 0 HP.** A member can be alive at 0 HP: after damage exactly equal to their HP, or
    after either kind of resurrection (5.5, 5.9).
21. **Combat swamps** always poison, without the 1-in-8 roll used elsewhere (5.8).
