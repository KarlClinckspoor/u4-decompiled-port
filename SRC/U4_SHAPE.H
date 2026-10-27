/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#ifndef __U4_SHAPE_H__
#define __U4_SHAPE_H__

/*deep water*/
#define TIL_DEEP_WATER 0x00
/*medium water*/
#define TIL_WATER 0x01
/*shallow water*/
#define TIL_SHALLOWS 0x02
/*swamp*/
#define TIL_SWAMP 0x03
/*grass*/
#define TIL_GRASS 0x04
/*scrub*/
#define TIL_SCRUB 0x05
/*forest*/
#define TIL_FOREST 0x06
/*hills*/
#define TIL_HILLS 0x07
/*mountains*/
#define TIL_MOUNTAINS 0x08
/*dungeon*/
#define TIL_DUNGEON 0x09
/*town*/
#define TIL_TOWN 0x0A
/*castle*/
#define TIL_CASTLE 0x0B
/*village*/
#define TIL_VILLAGE 0x0C
/*LB castle left wing*/
#define TIL_LCB_WEST 0x0D
/*LB castle entrance*/
#define TIL_LCB_ENTRANCE 0x0E
/*LB castle right wing*/
#define TIL_LCB_EAST 0x0F

/*ship W/N/E/S*/
#define TIL_SHIP_W 0x10
#define TIL_SHIP_N 0x11
#define TIL_SHIP_E 0x12
#define TIL_SHIP_S 0x13

/*horse W/E*/
#define TIL_HORSE_W 0x14
#define TIL_HORSE_E 0x15

/*tiled floor*/
#define TIL_TILED_FLOOR 0x16
/*bridge*/
#define TIL_BRIDGE 0x17
/*balloon*/
#define TIL_BALLOON 0x18
/**/
#define TIL_BRIDGE_N 0x19
/**/
#define TIL_BRIDGE_S 0x1A
/*ladder up*/
#define TIL_LADDER_UP 0x1B
/*ladder down*/
#define TIL_LADDER_DOWN 0x1C
/*ruins*/
#define TIL_RUINS 0x1D
/*shrine*/
#define TIL_SHRINE 0x1E
/*on foot party*/
#define TIL_AVATAR 0x1F

/*====---- 2-tile animation "human" ----====*/

/*mage*/
#define TIL_MAGE 0x20
/*bard*/
#define TIL_BARD 0x22
/*fighter*/
#define TIL_FIGHTER 0x24
/*druid*/
#define TIL_DRUID 0x26
/*tinker*/
#define TIL_TINKER 0x28
/*paladin*/
#define TIL_PALADIN 0x2A
/*ranger*/
#define TIL_RANGER 0x2C
/*shepherd*/
#define TIL_SHEPHERD 0x2E

/*====---- ----====*/

#define TIL_30 0x30
#define TIL_SHORE_1 0x31
#define TIL_SHORE_2 0x32
#define TIL_SHORE_3 0x33
#define TIL_SHORE_4 0x34
#define TIL_SHIP_WHEEL 0x35
#define TIL_SHIP_MAST 0x36
#define TIL_ROCKS 0x37
/*sleep*/
#define TIL_SLEEPING 0x38
/**/
#define TIL_STONES 0x39
/*locked door*/
#define TIL_LOCKED_DOOR 0x3A
/*door*/
#define TIL_DOOR 0x3B
/*chest*/
#define TIL_CHEST 0x3C
/*ankh*/
#define TIL_ANKH 0x3D
/*brick floor*/
#define TIL_BRICK_FLOOR 0x3E
/*wood floor*/
#define TIL_WOOD_FLOOR 0x3F

/*moongate 4 phases*/
#define TIL_MOONGATE_1 0x40
#define TIL_MOONGATE_2 0x41
#define TIL_MOONGATE_3 0x42
#define TIL_MOONGATE 0x43

/*poison field*/
#define TIL_POISON_FIELD 0x44
/*energy field*/
#define TIL_ENERGY_FIELD 0x45
/*fire field*/
#define TIL_FIRE_FIELD 0x46
/*sleep field*/
#define TIL_SLEEP_FIELD 0x47

#define TIL_SHIP_RAIL 0x48
#define TIL_SECRET_DOOR 0x49
#define TIL_ALTAR 0x4A
#define TIL_CAMPFIRE 0x4B
/*lava*/
#define TIL_LAVA 0x4C
/*missile*/
#define TIL_MISSILE 0x4D
#define TIL_MAGIC_BOLT 0x4E
#define TIL_HIT_FLASH 0x4F

/*====---- 2-tile animation NPC ----====*/

/*guard*/
#define TIL_GUARD 0x50
/*merchant*/
#define TIL_MERCHANT 0x52
/*bard*/
#define TIL_BARD_NPC 0x54
/*jester*/
#define TIL_JESTER 0x56
/*beggar*/
#define TIL_BEGGAR 0x58
/*child*/
#define TIL_CHILD 0x5A
/*bull*/
#define TIL_BULL 0x5C
/*lord british*/
#define TIL_LORD_BRITISH 0x5E

/*====---- Alphabet ----====*/

#define TIL_LETTER_A 0x60
#define TIL_LETTER_B 0x61
#define TIL_LETTER_C 0x62
#define TIL_LETTER_D 0x63
#define TIL_LETTER_E 0x64
#define TIL_LETTER_F 0x65
#define TIL_LETTER_G 0x66
#define TIL_LETTER_H 0x67
#define TIL_LETTER_I 0x68
#define TIL_LETTER_J 0x69
#define TIL_LETTER_K 0x6A
#define TIL_LETTER_L 0x6B
#define TIL_LETTER_M 0x6C
#define TIL_LETTER_N 0x6D
#define TIL_LETTER_O 0x6E
#define TIL_LETTER_P 0x6F
#define TIL_LETTER_Q 0x70
#define TIL_LETTER_R 0x71
#define TIL_LETTER_S 0x72
#define TIL_LETTER_T 0x73
#define TIL_LETTER_U 0x74
#define TIL_LETTER_V 0x75
#define TIL_LETTER_W 0x76
#define TIL_LETTER_X 0x77
#define TIL_LETTER_Y 0x78
#define TIL_LETTER_Z 0x79
/*<space>*/
#define TIL_LETTER_SPACE 0x7A

#define TIL_7B 0x7B
#define TIL_7C 0x7C
#define TIL_7D 0x7D
#define TIL_BLACK 0x7E
#define TIL_WALL 0x7F

/*====---- 2-tile monsters ----====*/

/*pirate*/
#define TIL_PIRATE_W 0x80
#define TIL_PIRATE_N 0x81
/*pirate*/
#define TIL_PIRATE_E 0x82
#define TIL_PIRATE_S 0x83
/*nixie*/
#define TIL_NIXIE 0x84
/*squid*/
#define TIL_SQUID 0x86
/*serpent*/
#define TIL_SERPENT 0x88
/*seahorse*/
#define TIL_SEAHORSE 0x8A
/*whirlpool*/
#define TIL_WHIRLPOOL 0x8C
/*twister*/
#define TIL_TWISTER 0x8E
#define TIL_TWISTER_1 0x8F

/*====---- 4-tile monsters ----====*/

/*rat*/
#define TIL_RAT 0x90
/*bat*/
#define TIL_BAT 0x94
/*spider*/
#define TIL_SPIDER 0x98
/*ghost*/
#define TIL_GHOST 0x9C
/*slime*/
#define TIL_SLIME 0xA0
/*troll*/
#define TIL_TROLL 0xA4
/*gremlin*/
#define TIL_GREMLIN 0xA8
/*mimic*/
#define TIL_MIMIC 0xAC
/*reaper*/
#define TIL_REAPER 0xB0
/*insects*/
#define TIL_INSECTS 0xB4
/*gazer*/
#define TIL_GAZER 0xB8
/*phantom*/
#define TIL_PHANTOM 0xBC
/*orc*/
#define TIL_ORC 0xC0
/*skeleton*/
#define TIL_SKELETON 0xC4
/*rogue*/
#define TIL_ROGUE 0xC8
/*python*/
#define TIL_PYTHON 0xCC
/*ettin*/
#define TIL_ETTIN 0xD0
/*headless*/
#define TIL_HEADLESS 0xD4
/*cyclops*/
#define TIL_CYCLOPS 0xD8
/*wisp*/
#define TIL_WISP 0xDC
/*mage*/
#define TIL_EVIL_MAGE 0xE0
/*lyche*/
#define TIL_LICHE 0xE4
/*lava lizard*/
#define TIL_LAVA_LIZARD 0xE8
#define TIL_LAVA_LIZARD_1 0xE9
/*zorn*/
#define TIL_ZORN 0xEC
/*daemon*/
#define TIL_DAEMON 0xF0
/*hydra*/
#define TIL_HYDRA 0xF4
/*dragon*/
#define TIL_DRAGON 0xF8
/*balron*/
#define TIL_BALRON 0xFC

#endif
