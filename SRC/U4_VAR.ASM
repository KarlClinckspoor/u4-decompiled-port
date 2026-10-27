DGROUP	GROUP XOB,XO,XOE,c_common

XOB	SEGMENT PUBLIC WORD 'BSS'
XOB	ENDS
XO	SEGMENT PUBLIC WORD 'BSS'
XO	ENDS
XOE	SEGMENT PUBLIC WORD 'BSS'
XOE	ENDS

PUBLIC CombatFoeY,MoongateTile,ShopId,Party
PUBLIC MapChunks,GraphicsType,tile_forward,AltarRoom
PUBLIC LineOfSight,ship_x,ship_y,tile_cur
PUBLIC OpenGateX,Redraw3D,OpenGateY,CharaAnimTile
PUBLIC CombatFoe,pShapes,GameLoopJmp,CurMode
PUBLIC spell_turns,CombatGround,Combat,Fighters
PUBLIC File_DNG,activeChara,WinPos
PUBLIC tile_east,pCharset,spell_effect,WinBlock
PUBLIC txt_X,tile_north,activeCharaY
PUBLIC hit_tile,hit_x,hit_y
PUBLIC RoomTriggerData
PUBLIC tile_west,tile_south,txt_Y,Gallop
PUBLIC CursorBlink,tile_back,SavedMode,TalkData
PUBLIC RoomExitX,File_MAP,WindDir,RoomExitY
PUBLIC File_TLK,CombatFoeTile,ViewTiles,CombatFoeX

;actually, the next segment should be
;generated as a COMDEF record

c_common	SEGMENT PUBLIC PARA 'BSS'
	ASSUME DS:DGROUP

CombatFoeY	db ?	;D_9140
MoongateTile	db ?	;D_9141
ShopId	dw ?	;D_9142
Party	db 1F6h dup(?)	;D_9144
MapChunks	dd 8*8 dup(?)	;D_933A tMap32x32 far *
GraphicsType	dw ?	;D_943A graphic adaptater type 
tile_forward	db ?	;D_943C
	db ?
AltarRoom	dw ?	;D_943E
LineOfSight	dw ?	;D_9440
ship_x	db ?	;D_9442
ship_y	db ?	;D_9443
tile_cur	db ?	;D_9444
OpenGateX	db ?	;D_9445
Redraw3D	dw ?	;D_9446
OpenGateY	db ?	;D_9448
	db ?
CharaAnimTile	db 8 dup (?)	;D_944A
CombatFoe	db ?	;D_9452
	db ?
pShapes	dd ?	;D_9454
GameLoopJmp	dw 9 dup(?)	;D_9458 jmp_buf
CurMode	dw ?	;D_946A
CombatGround	db ?	;D_946C
	db ?
spell_turns	dw ?	;D_946E
Combat	db 0C0h dup(?)	;D_9470 struct tCombat
Fighters	db 68h dup(?)	;D_9530
File_DNG	dw ?	;D_9598
activeChara	dw ?	;D_959A
WinPos	db ?,?	;D_959C
tile_east	db ?	;D_959E
	db ?
pCharset	dd ?	;D_95A0
spell_effect	db ?	;D_95A4
WinBlock	db ?,?	;D_95A5
	db ?
txt_X	dw ?
tile_north	db ?	;D_95AA
	db ?
activeCharaY	dw ?	;D_95AC
hit_tile	dw ?	;D_95AE
hit_x	db ?	;D_95B0
hit_y	db ?	;D_95B1
RoomTriggerData	db 16 dup (?)	;D_95B2
tile_west	db ?	;D_95C2
tile_south	db ?	;D_95C3
txt_Y	dw ?
Gallop	dw ?	;D_95C6
CursorBlink	dw ?	;D_95C8
tile_back	db ?	;D_95CA
	db ?
SavedMode	dw ?	;D_95CC
TalkData	db 120h dup (?)	;D_95CE
RoomExitX	db ?	;D_96EE
	db ?
File_MAP	dw ?	;D_96F0
WindDir	dw ?	;D_96F2
RoomExitY	db ?	;D_96F4
	db ?
File_TLK	dw ?	;D_96F6
CombatFoeTile	db ?	;D_96F8
ViewTiles	db 11*11 dup (?)	;D_96F9
CombatFoeX	db ?	;D_9772

c_common	ENDS

END
