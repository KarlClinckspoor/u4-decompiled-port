/*
 * Ultima IV - Quest Of The Avatar
 * (c) Copyright 1987 Lord British
 * reverse-coded by Ergonomy Joe in 2012
 */

#include "u4.h"

#include <string.h>

/*words Lord British answers to (only the first 4 letters count), "" ends the list.
  0-2 are handled apart, the others are answered with LBAnswers[index - 3]*/
char * /*D_6FF0*/LBKeywords[28] = {
	/*D_639A*/"bye",
	/*D_639E*/"help",
	/*D_63A3*/"health",
	/*D_63AA*/"name",
	/*D_63AF*/"look",
	/*D_63B4*/"job",
	/*D_63B8*/"truth",
	/*D_63BE*/"love",
	/*D_63C3*/"courage",
	/*D_63CB*/"honesty",
	/*D_63D3*/"compassion",
	/*D_63DE*/"valor",
	/*D_63E4*/"justice",
	/*D_63EC*/"sacrifice",
	/*D_63F6*/"honor",
	/*D_63FC*/"spirituality",
	/*D_6409*/"humility",
	/*D_6412*/"pride",
	/*D_6418*/"avatar",
	/*D_641F*/"quest",
	/*D_6425*/"britannia",
	/*D_642F*/"ankh",
	/*D_6434*/"abyss",
	/*D_643A*/"mondain",
	/*D_6442*/"minax",
	/*D_6448*/"exodus",
	/*D_644F*/"virtue",
	/*D_6456*/""
};

char * /*D_7028*/LBAnswers[24] = {
	/*D_6457*/"\n\n\nHe says:\nMy name is\nLord British,\nSovereign of\nall Britannia!\n",
	/*D_6499*/"\n\n\n\n\nThou see the\nKing with the\nRoyal Sceptre.\n",
	/*D_64C9*/"\n\n\nHe says:\nI rule all\nBritannia, and\nshall do my best\nto help thee!\n",
	/*D_650F*/"He says:\nMany truths can\nbe learned at\nthe Lycaeum.  It\nlies on the\nnorthwestern\nshore of Verity\nIsle!\n",
	/*D_6577*/"He says:\nLook for the\nmeaning of Love\nat Empath Abbey.\nThe Abbey sits\non the western\nedge of the Deep\nForest!\n",
	/*D_65E6*/"\n\nHe says:\nSerpent's Castle\non the Isle of\nDeeds is where\nCourage should\nbe sought!\n",
	/*D_663B*/"\nHe says:\nThe fair towne\nof Moonglow on\nVerity Isle is\nwhere the virtue\nof Honesty\nthrives!\n",
	/*D_6698*/"\n\nHe says:\nThe bards in the\ntowne of Britain\nare well versed\nin the virtue of\nCompassion!\n",
	/*D_66F3*/"\n\nHe says:\nMany valiant\nfighters come\nfrom Jhelom\nin the Valarian\nIsles!\n",
	/*D_673D*/"\n\n\nHe says:\nIn the city of\nYew, in the Deep\nForest, Justice\nis served!\n",
	/*D_6785*/"\nHe says:\nMinoc, towne of\nself-sacrifice,\nlies on the\neastern shores\nof Lost Hope\nBay!\n",
	/*D_67DD*/"\nHe says:\nThe Paladins who\nstrive for Honor\nare oft seen in\nTrinsic, north\nof the Cape of\nHeroes!\n",
	/*D_6840*/"\nHe says:\nIn Skara Brae\nthe Spiritual\npath is taught.\nFind it on an\nisle near\nSpiritwood!\n",
	/*D_689B*/"\n\n\nHe says:\nHumility is the\nfoundation of\nVirtue!  The\nruins of proud\nMagincia are a\ntestimony unto\nthe Virtue of\nHumility!\n\nFind the Ruins\nof Magincia far\noff the shores\nof Britannia,\non a small isle\nin the vast\nOcean!\n",
	/*D_6978*/"\n\n\nHe says:\nOf the eight\ncombinations of\nTruth, Love and\nCourage, that\nwhich contains\nneither Truth,\nLove nor Courage\nis Pride.\n\nPride being not\na Virtue must be\nshunned in favor\nof Humility, the\nVirtue which is\nthe antithesis\nof Pride!\n",
	/*D_6A66*/"\n\n\n\n\n\nLord British\nsays:\nTo be an Avatar\nis to be the\nembodiment of\nthe Eight\nVirtues.\n\n\nIt is to live a\nlife constantly\nand forever in\nthe Quest to\nbetter thyself\nand the world in\nwhich we live.\n",
	/*D_6B2B*/"\n\n\nLord British\nsays:\nThe Quest of\nthe Avatar is\nto know and\nbecome the\nembodiment of\nthe Eight\nVirtues of\nGoodness!\nIt is known that\nall who take on\nthis Quest must\nprove themselves\nby conquering\nthe Abyss and\nViewing the\nCodex of\nUltimate Wisdom!\n",
	/*D_6C25*/"\n\n\n\n\n\n\nHe says:\nEven though the\nGreat Evil Lords\nhave been routed\nevil yet remains\nin Britannia.\n\n\n\n\n\nIf but one soul\ncould complete\nthe Quest of the\nAvatar, our\npeople would\nhave a new hope,\na new goal for\nlife.\n\nThere would be a\nshining example\nthat there is\nmore to life\nthan the endless\nstruggle for\npossessions\nand gold!\n",
	/*D_6D6C*/"He says:\nThe Ankh is the\nsymbol of one\nwho strives for\nVirtue.  Keep it\nwith thee at all\ntimes for by\nthis mark thou\nshalt be known!\n",
	/*D_6DF2*/"\n\n\n\n\n\nHe says:\nThe Great\nStygian Abyss\nis the darkest\npocket of evil\nremaining in\nBritannia!\n\n\n\n\n\n\n\nIt is said that\nin the deepest\nrecesses of the\nAbyss is the\nChamber of the\nCodex!\n\n\nIt is also said\nthat only one of\nhighest Virtue\nmay enter this\nChamber, one\nsuch as an\nAvatar!!!\n",
	/*D_6F0C*/"\n\n\n\n\n\nHe says:\nMondain is dead!\n",
	/*D_6F2D*/"\n\n\n\n\n\nHe says:\nMinax is dead!\n",
	/*D_6F4C*/"\n\n\n\n\n\nHe says:\nExodus is dead!\n",
	/*D_6F6C*/"\nHe says:\nThe Eight\nVirtues of the\nAvatar are:\nHonesty,\nCompassion,\nValor,\nJustice,\nSacrifice,\nHonor,\nSpirituality,\nand Humility!\n"
};

/*
 * Pause between pages of Lord British's speech: wait up to 10 seconds or until a key
 * is pressed (the key is eaten).
 */
int16 /*C_E1FC*/LB_Pause() {
	u_kbflush();
	u_delay(10, 1);
	if(u_kbhit())
		u_kbread();
}

/*
 * Answer to "help": a hint chosen by how far the game has gone (moves, party size,
 * runes, partial Avatarhoods, stones, Bell/Book/Candle, the three-part key).
 */
int16 /*C_E21E*/LB_Help() {
	u4_puts(/*D_7058*/"He says: ");
	if(Party._moves < 1000) {
		u4_puts(/*D_7062*/"To survive in this hostile land thou must first know thyself! Seek ye to master thy weapons and thy magical ability!\n"); LB_Pause();
		u4_puts(/*D_70D8*/"\nTake great care in these thy first travels in Britannia.\n"); LB_Pause();
		u4_puts(/*D_7113*/"\nUntil thou dost well know thyself, travel not far from the safety of the townes!\n");
	} else
	if(Party._members == 1) {
		u4_puts(/*D_7166*/"Travel not the open lands alone. There are many worthy people in the diverse townes whom it would be wise to ask to Join thee!\n"); LB_Pause();
		u4_puts(/*D_71E6*/"\nBuild thy party unto eight travellers, for only a true leader can win the Quest!\n");
	} else
	if(Party.mRunes == 0) {
		u4_puts(/*D_7239*/"Learn ye the paths of virtue. Seek to gain entry unto the eight shrines!\n"); LB_Pause();
		u4_puts(/*D_7283*/"\nFind ye the Runes, needed for entry into each shrine, and learn each chant or \"Mantra\" used to focus thy meditations.\n"); LB_Pause();
		u4_puts(/*D_72FB*/"\nWithin the Shrines thou shalt learn of the deeds which show thy inner virtue or vice!\n"); LB_Pause();
		u4_puts(/*D_7353*/"\nChoose thy path wisely for all thy deeds of good and evil are remembered and can return to hinder thee!\n");
	} else
	/*meant as "no partial Avatarhood yet" (no virtue at 0); but the virtues are combined
	  with a bitwise & and not &&, so it also fails when they merely share no bit*/
	if(
		Party._hones & Party._compa &
		Party._valor & Party._justi &
		Party._sacri & Party._honor &
		Party._spiri & Party._humil
	) {
		u4_puts(/*D_73BD*/"Visit the Seer Hawkwind often and use his wisdom to help thee prove thy virtue.\n"); LB_Pause();
		u4_puts(/*D_740E*/"\nWhen thou art ready, Hawkwind will advise thee to seek the Elevation unto partial Avatarhood in a virtue.\n"); LB_Pause();
		u4_puts(/*D_747A*/"\nSeek ye to become a partial Avatar in all eight virtues, for only then shalt thou be ready to seek the codex!\n");
	} else
	if(Party.mStones == 0) {
		u4_puts(/*D_74EA*/"Go ye now into the depths of the dungeons. Therein recover the 8 colored stones from the altar pedestals in the halls of the dungeons.\n"); LB_Pause();
		u4_puts(/*D_7572*/"\nFind the uses of these stones for they can help thee in the Abyss!\n");
	} else
	/*not yet Avatar in all eight virtues*/
	if(
		Party._hones | Party._compa |
		Party._valor | Party._justi |
		Party._sacri | Party._honor |
		Party._spiri | Party._humil
	) {
		u4_puts(/*D_75B7*/"Thou art doing very well indeed on the path to Avatarhood! Strive ye to achieve the Elevation in all eight virtues!\n");
	} else
	if(!((Party.mItems >> ST_BELL) & (Party.mItems >> ST_BOOK) & (Party.mItems >> ST_CANDLE) & 1)) {
		u4_puts(/*D_762C*/"Find ye the Bell, Book and Candle!  With these three things, one may enter the Great Stygian Abyss!\n");
	} else
	if(!((Party.mItems >> ST_KEY_C) & (Party.mItems >> ST_KEY_L) & (Party.mItems >> ST_KEY_T) & 1)) {
		u4_puts(/*D_7691*/"Before thou dost enter the Abyss thou shalt need the Key of Three Parts, and the Word of Passage.\n"); LB_Pause();
		u4_puts(/*D_76F4*/"\nThen might thou enter the Chamber of the Codex of Ultimate Wisdom!\n");
	} else {
		u4_puts(/*D_7739*/"Thou dost now seem ready to make the final journey into the dark Abyss! Go only with a party of eight!\n"); LB_Pause();
		u4_puts(/*D_77A1*/"\nGood Luck, and may the powers of good watch over thee on this thy most perilous endeavor!\n"); LB_Pause();
		u4_puts(/*D_77FD*/"\nThe hearts and souls of all Britannia go with thee now. Take care, my friend.\n");
	}
}

/*
 * Look for word in LBKeywords (only the first 4 letters are compared, ignoring case).
 * Returns its index, or -1 if not found.
 */
int16 /*C_E37E*/LB_FindKeyword(word)
char *word;
{
	int16 kw;

	for(kw = 0; LBKeywords[kw][0]; kw++) {
		if(strnicmp(word, LBKeywords[kw], 4) == 0)
			break;
	}
	if(LBKeywords[kw][0] == 0)
		return -1;
	return kw;
}

/*
 * Print one of Lord British's answers, pausing after every 13 lines.
 */
int16 /*C_E3D2*/LB_Speak(text)
register char *text;
{
	register int16 lines;

	lines = 0;
	while(*text) {
		if(*text == '\n') {
			if(lines++ == 12) {
				LB_Pause();
				lines = 0;
			}
		}
		u4_putc(*text);
		text++;
	}
}

/*
 * Heal every character that is not dead: full hit points, status Good.
 */
int16 /*C_E408*/HealParty() {
	int16 chara;

	for(chara = Party._members - 1; chara >= 0; chara --) {
		if(Party.chara[chara]._stat != 'D') {
			Party.chara[chara]._stat = 'G';
			Party.chara[chara]._HP[0] = Party.chara[chara]._HP[1];
		}
	}
}

/*
 * Answer to "health": Lord British asks if you are well; on No he heals the party.
 */
int16 /*C_E442*/LB_Health() {
	char answer;

	u4_puts(/*D_784D*/"\n\n\n\n\n\nHe says: I am well, thank ye.\n\nHe asks: Art thou well?\x12\x12\b");
	answer = AskY_N();
	if(answer == 'Y') {
		u4_puts(/*D_788D*/"\nHe says: That is good.\n");
	} else if(answer =='N') {
		u4_puts(/*D_78A6*/"\nHe says: Let me heal thy wounds!\n");
		/*heal party*/
		sound(SND_CAST, 10);
		Gra_InvertView(); sound(SND_MAGIC, 0xc0); Gra_InvertView();
		HealParty();
		dspl_Stats();
	}
}

/*
 * Raise the characteristic *stat by 1 to 8 (random), capped at 50.
 */
int16 /*C_E498*/RaiseStat(stat)
int16 *stat;
{
	*stat += U4_RND1(7)+1;
	*stat = u4_min(*stat, 50);
}

/*
 * Lord British raises the level of the characters who have enough experience: the
 * level is 1 + the number of times the XP reach 100, 200, 400, 800...; max hit points
 * become 100 * level, the character is fully healed and set to Good (which also
 * brings a dead one back to life) and strength, dexterity and intelligence each get
 * +1 to +8.
 */
int16 /*C_E4C3*/LB_LevelUp() {
	int16 limit, chara, newMax;

	for(chara = 0; chara < Party._members; chara++) {
		for(newMax = limit = 100; limit <= Party.chara[chara]._XP; newMax += 100, limit <<= 1);
		if(Party.chara[chara]._HP[1] < newMax) {
			Party.chara[chara]._HP[1] = newMax;
			Party.chara[chara]._HP[0] = newMax;
			Party.chara[chara]._stat = 'G';
			RaiseStat(&(Party.chara[chara]._str));
			RaiseStat(&(Party.chara[chara]._dex));
			RaiseStat(&(Party.chara[chara]._int));
			Gra_CR();
			u4_puts(Party.chara[chara]._name);
			u4_puts(/*D_78C9*/"\nThou art now Level ");
			u4_putl(newMax / 100, 1, '0');
			Gra_CR();
			Gra_InvertView(); sound(SND_MAGIC, 0xc0); Gra_InvertView();
			dspl_Stats();
		}
	}
	u4_puts(/*D_78DE*/"What would thou ask of me?\n");
}

/*
 * Talk to Lord British: greeting (the first time: the introduction; afterwards he
 * resurrects a dead leader, heals, and raises levels), then answer questions until
 * "bye" or an empty line.
 */
int16 /*C_E59B*/TalkToLB()
{
	int16 topic;
	char question[15];

	if(Party._metLB == 0) {
		/*-- first time --*/
		Party._metLB = 1;
		u4_puts(/*D_78FA*/"\n\n\nLord British rises and says: At long last!\n");
		u4_puts(Party.chara[0]._name);
		u4_putc(',');
		u4_puts(/*D_7929*/" thou hast come!  We have waited such a long, long time...\n\n");
		LB_Pause();
		u4_puts(/*D_7966*/"\n\nLord British sits and says: A new age is upon Britannia. The great evil Lords are gone but our people lack direction and purpose in their lives...\n\n\n\
A champion of virtue is called for. Thou may be this champion, but only time shall tell.  I will aid thee any way that I can!\n");
		u4_puts(/*D_7A7C*/"How may I help thee?\n");
	} else {
		/*-- second time and after --*/
		if(Party.chara[0]._stat == 'D') {
			/*resurrection*/
			Party.chara[0]._stat = 'G';
			u4_puts(Party.chara[0]._name);
			u4_puts(/*D_7A92*/", Thou shalt live again!\n");
			/*heal party*/
			sound(SND_CAST, 20);
			Gra_InvertView(); sound(SND_MAGIC, 0xc0); Gra_InvertView();
			HealParty();
			dspl_Stats();
		}
		u4_puts(/*D_7AAC*/"\n\n\nLord British says:  Welcome ");
		u4_puts(Party.chara[0]._name);
		if(Party._members >= 3) {
			u4_puts(/*D_7ACC*/" and thy worthy Adventurers!\n");
		} else if(Party._members == 2) {
			u4_puts(/*D_7AEA*/" and thee also ");
			u4_puts(Party.chara[1]._name);
			u4_puts(/*D_7AFA*/"!\n");
		} else {
			Gra_CR();
		}
		LB_LevelUp();
	}

	for(;;) {
		u4_gets(question, 15);
		Gra_CR();
		if(question[0] == 0)
			topic = 0;
		else
			topic = LB_FindKeyword(question);
		if(topic == 0)/*"bye"*/
			break;
		switch(topic) {
			case 2: LB_Health(); break;
			case 1: LB_Help(); break;
			case -1: u4_puts(/*D_7AFD*/"\nHe says: I cannot help thee with that.\n"); break;
			default: LB_Speak(LBAnswers[topic-3]);
		}
		u4_puts(/*D_7B26*/"\nWhat else?\n");
	}

	u4_puts(/*D_7B33*/"Lord British says: Fare thee well my friend");
	if(Party._members > 1)
		u4_putc('s');
	u4_puts(/*D_7B5F*/"!\n");
}
