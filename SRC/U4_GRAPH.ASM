EXTRN _exit:NEAR

EXTRN _SoundFlag:BYTE
EXTRN pKarmas:WORD

;from U4_DOS
EXTRN _dalloc:NEAR
EXTRN _dopen:NEAR
EXTRN _dread:NEAR
EXTRN _dclose:NEAR

EXTRN DrawLine:NEAR	;drawLine

EXTRN GraphicsType:WORD
EXTRN pShapes:DWORD
EXTRN pCharset:DWORD
EXTRN txt_X:WORD
EXTRN txt_Y:WORD

DGROUP	GROUP _DATA,_BSS
;########################################
_DATA	SEGMENT PUBLIC WORD 'DATA'
	ASSUME DS:DGROUP
;----------------------------------------
StrStartPic	db "start.pic",0	;D_08A4
StrStartEga	db "start.ega",0	;D_08AE
StrRune5Pic	db "rune_5.pic",0	;D_08B8
StrRune5Ega	db "rune_5.ega",0	;D_08C3
;----------------------------------------
_DATA	ENDS
;########################################
_BSS	SEGMENT PUBLIC WORD 'BSS'
;----------------------------------------
pGraphicsDriver	dw ?,?	;D_8C46 loading buffer for graphic lib
;----------------------------------------
InterleaveBuf	db 80h dup(?)	;D_8C4A
;----------------------------------------
_BSS	ENDS
;########################################
_TEXT	SEGMENT PUBLIC BYTE 'CODE'
	ASSUME CS:_TEXT
;----------------------------------------
;load graphic lib
PUBLIC LoadGraphicsDriver
LoadGraphicsDriver	PROC NEAR	;C_20C1
	PUSH	BP
	MOV	BP,SP
	SUB	SP,4
	PUSH	ES
	;--
	MOV	AX,0
	PUSH	AX
	PUSH	[BP+04]
	CALL	_dopen
	ADD	SP,4
	MOV	[BP-02],AX
	;%%
	INC	AX
	JNZ	C_20E2
C_20DB:	;--
	MOV	AX,49h
	PUSH	AX
	CALL	_exit
C_20E2:	;** inline seek? **
	MOV	BX,[BP-02]
	MOV	AL,2	;SEEK_END
	MOV	CX,0
	MOV	DX,0
	MOV	AH,42h	; move file pointer using handle
	INT	21h
	;** **
	MOV	[BP-04],AX
	;%%
	PUSH	AX
	CALL	_dalloc
	ADD	SP,2
	MOV	pGraphicsDriver,AX
	MOV	pGraphicsDriver+2,DX
	;** inline seek? **
	MOV	BX,[BP-02]
	MOV	AL,0	;SEEK_SET
	MOV	CX,0
	MOV	DX,0
	MOV	AH,42h	; move file pointer using handle
	INT	21h
	;** **
	PUSH	[BP-04]
	PUSH	pGraphicsDriver+2
	PUSH	pGraphicsDriver
	PUSH	[BP-02]
	CALL	_dread
	ADD	SP,8
	;%%
	CMP	AX,[BP-04]
	JNZ	C_20DB
	;--
	PUSH	[BP-02]
	CALL	_dclose
	ADD	SP,2
	;--
	POP	ES
	ADD	SP,4
	POP	BP
	RET	2
LoadGraphicsDriver	ENDP	;C_20C1
;----------------------------------------
;draw game screen frame
PUBLIC DrawGameFrame
DrawGameFrame PROC NEAR	;C_213B
	CALL    _Gra_ClearScreen
	;-- BX := (GraphicsType == 1)?"start.pic":"start.ega"
	LEA     BX,[StrStartPic]	;"start.pic"
	CMP     GraphicsType,1
	JZ      C_214D
	LEA     BX,[StrStartEga]	;"start.ega"
C_214D:	;-- Gra_LoadPicture(BX, 0, 0)
	MOV     AX,0
	PUSH    AX
	PUSH    AX
	PUSH	BX
	CALL	_Gra_LoadPicture	;display image
	ADD	SP,6
	;--
	RET
DrawGameFrame ENDP	;C_213B
;----------------------------------------C_215A
PUBLIC _Gra_Line
_Gra_Line PROC NEAR
	PUSH	BP
	MOV	BP,SP
	;-- other line ? --
	MOV	AX,[BP+04]
	CMP	AX,[BP+08]
	JZ	C_217E
	MOV	AX,[BP+06]
	CMP	AX,[BP+0Ah]
	JZ	C_217E
	PUSH	[BP+04]
	PUSH	[BP+06]
	PUSH	[BP+08]
	PUSH	[BP+0Ah]
	CALL	DrawLine	;drawLine
	POP	BP
	RET
C_217E:	;-- straight line --
	SUB	SP,+02
	PUSH	SI
	PUSH	DI
	;--
	MOV	AX,[BP+04]
	CMP	AX,[BP+08]
	JNZ	C_21C4
	;----
	;
	;----
	MOV	AX,[BP+0Ah]
	SUB	AX,[BP+06]
	CWD
	MOV	SI,DX
	TEST	AX,0FFFFh
	JLE	C_219C
	MOV	SI,1
C_219C:	;--
	PUSH	[BP+0Ch]
	PUSH	[BP+04]
	PUSH	[BP+06]
	MOV	AX,[BP+0Ah]
	SUB	AX,[BP+06]
	JGE	C_21AF
	NEG	AX
C_21AF:	INC	AX
	MOV	[BP-02],AX
C_21B3:	;--
	CALL	_Gra_DotOr
	;--
	ADD	[BP-0Ch],SI
	DEC	WORD PTR [BP-02]
	JNZ	C_21B3
	ADD	SP,6
	;----
	;
	;----
	JMP	C_21FA	;(+nop)
	;----
C_21C4:	;
	;----
	MOV	AX,[BP+08]
	SUB	AX,[BP+04]
	CWD
	MOV	SI,DX
	TEST	AX,0FFFFh
	JLE	C_21D5
	MOV	SI,1
C_21D5:	;--
	PUSH	[BP+0Ch]
	PUSH	[BP+04]
	PUSH	[BP+06]
	MOV	AX,[BP+08]
	SUB	AX,[BP+04]
	JGE	C_21E8
	NEG	AX
C_21E8:	INC	AX
	MOV	[BP-02],AX
C_21EC:	;--
	CALL	_Gra_DotOr
	;--
	ADD	[BP-0Ah],SI
	DEC	WORD PTR [BP-02]
	JNZ	C_21EC
	ADD	SP,6
	;----
C_21FA:	;
	;----
	POP	DI
	POP	SI
	ADD	SP,2
	POP	BP
	RET
_Gra_Line ENDP
;----------------------------------------
; handler to extern graphic library
;----------------------------------------
;----C_2201
PUBLIC _Gra_ActiveBox	;active character square
_Gra_ActiveBox PROC NEAR
	MOV	DX,0*2
	JMP	SHORT CallDriver
_Gra_ActiveBox ENDP
;----C_2206
PUBLIC _Gra_ZoomTileRow
_Gra_ZoomTileRow PROC NEAR
	MOV	DX,1*2
	JMP	SHORT CallDriver
_Gra_ZoomTileRow ENDP
;----C_220B
PUBLIC _Gra_FillTiles
_Gra_FillTiles PROC NEAR
	MOV	DX,2*2
	JMP	SHORT CallDriver
_Gra_FillTiles ENDP
;----C_2210
PUBLIC _Gra_ShiftDown
_Gra_ShiftDown PROC NEAR
	POP	AX
	PUSH	WORD PTR _SoundFlag
	PUSH	AX
	MOV	DX,3*2
	JMP	SHORT CallDriver
_Gra_ShiftDown  ENDP
;----C_221B
PUBLIC _Gra_ShiftUp
_Gra_ShiftUp PROC NEAR
	POP	AX
	PUSH	WORD PTR _SoundFlag
	PUSH	AX
	MOV	DX,4*2
	JMP	SHORT CallDriver
_Gra_ShiftUp ENDP
;---- C_2226
;display tile map?
PUBLIC _Gra_DrawTiles
_Gra_DrawTiles PROC NEAR
	MOV	DX,5*2
	JMP	SHORT CallDriver
_Gra_DrawTiles ENDP
;----C_222B
PUBLIC _Gra_Dot
_Gra_Dot PROC NEAR
	MOV	DX,6*2
	JMP	SHORT CallDriver
_Gra_Dot ENDP
;----C_2230
PUBLIC Gra_CR
Gra_CR PROC NEAR
	MOV	txt_Y,23
	MOV	txt_X,24
	MOV	DX,8*2
	JMP	SHORT CallDriver
Gra_CR ENDP
;----C_2241
PUBLIC Gra_InvertView
Gra_InvertView PROC NEAR
	MOV	DX,9*2
	JMP	SHORT CallDriver
Gra_InvertView ENDP
;----C_2246
PUBLIC Gra_ClearView
Gra_ClearView PROC NEAR
	MOV	DX,10*2
	JMP	SHORT CallDriver
Gra_ClearView ENDP
;----C_224B
PUBLIC Gra_InvertStatusLine
Gra_InvertStatusLine PROC NEAR
	MOV	DX,11*2
	JMP	SHORT CallDriver
Gra_InvertStatusLine ENDP
;----C_2250
;(uncalled?)
Gra_InvertChar PROC NEAR
	MOV	DX,12*2
	JMP	SHORT CallDriver
Gra_InvertChar ENDP
;----
CallDriver:	;C_2255 -- create far ret
	POP	AX	;return address
	PUSH	CS
	PUSH	AX
	;--
	MOV	CX,ES	;backup ES
	LES	BX,DWORD PTR pGraphicsDriver
	ADD	BX,DX
	PUSH	ES
	MOV	AX,ES:[BX]
	ADD	AX,pGraphicsDriver
	PUSH	AX
	MOV	ES,CX	;restore ES
	db	0CBh;	RETF
;----C_226C
PUBLIC Gra_ClearStatus
Gra_ClearStatus PROC NEAR
	MOV	DX,13*2
	JMP	CallDriver
Gra_ClearStatus ENDP
;----C_2271
PUBLIC _Gra_DotOr
_Gra_DotOr PROC NEAR
	MOV	DX,14*2
	JMP	CallDriver
_Gra_DotOr ENDP
;----C_2276
PUBLIC _Gra_DotXor
_Gra_DotXor PROC NEAR
	MOV	DX,15*2
	JMP	CallDriver
_Gra_DotXor ENDP
;----C_227B
PUBLIC _Gra_LoadPicture
_Gra_LoadPicture PROC NEAR
	MOV	DX,16*2
	JMP	CallDriver
_Gra_LoadPicture ENDP
;----C_2280
PUBLIC _Gra_RevealCodex
_Gra_RevealCodex PROC NEAR
	POP	AX
	;--
	LEA	BX,[StrRune5Pic]
	CMP	GraphicsType,1
	JZ	C_2290
	LEA	BX,[StrRune5Ega]
C_2290:	PUSH	BX
	;--
	PUSH	AX
	MOV	DX,17*2
	JMP	CallDriver
_Gra_RevealCodex ENDP
;----C_2297
;draw some quad?
PUBLIC _Gra_DrawQuad
_Gra_DrawQuad PROC NEAR
	MOV	DX,18*2
	JMP	CallDriver
_Gra_DrawQuad ENDP
;----C_229C
;draw the ankh?
PUBLIC _Gra_VirtueGlyph
_Gra_VirtueGlyph PROC NEAR
	POP	AX
	MOV	BX,OFFSET DGROUP:pKarmas
	PUSH	BX
	PUSH	AX
	MOV	DX,19*2
	JMP	CallDriver
_Gra_VirtueGlyph ENDP
;----C_22A7
PUBLIC _Gra_AnimCampfire
_Gra_AnimCampfire PROC NEAR
	MOV	DX,20*2
	JMP	CallDriver
_Gra_AnimCampfire ENDP
;----C_22AC
PUBLIC _Gra_Init
_Gra_Init PROC NEAR
	MOV	DX,21*2
	JMP	CallDriver
_Gra_Init ENDP
;----C_22B1
PUBLIC _Gra_ClearScreen
_Gra_ClearScreen PROC NEAR
	MOV	DX,22*2
	JMP	CallDriver
_Gra_ClearScreen ENDP
;----C_22B6
PUBLIC Gra_AnimFlow
Gra_AnimFlow PROC NEAR
	MOV	DX,23*2
	JMP	CallDriver
Gra_AnimFlow ENDP
;----C_22BB
PUBLIC Gra_AnimFlags
Gra_AnimFlags PROC NEAR
	MOV	DX,24*2
	JMP	CallDriver
Gra_AnimFlags ENDP
;----C_22C0
PUBLIC Gra_PutChar
Gra_PutChar PROC NEAR
	POP	AX
	PUSH	txt_X
	PUSH	txt_Y
	PUSH	AX
	MOV	DX,25*2
	JMP	CallDriver
Gra_PutChar ENDP
;----------------------------------------
;
;----------------------------------------
;(TITLE.EXE:_C_210C/AVATAR.EXE:C_22CF)
PUBLIC InterleaveShapes
InterleaveShapes	PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	ES
	;--
	LES	SI,pShapes
	MOV	DI,SI
	MOV	CX,100h
C_22DB:
	MOV	DH,10h
	MOV	BX,OFFSET DGROUP:InterleaveBuf
C_22E0:
	MOV	DL,8
C_22E2:
	LODS	BYTE PTR ES:[SI]
	SHL	AL,1
	RCL	BYTE PTR [BX+01h],1
	RCL	BYTE PTR [BX+00h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+21h],1
	RCL	BYTE PTR [BX+20h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+41h],1
	RCL	BYTE PTR [BX+40h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+61h],1
	RCL	BYTE PTR [BX+60h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+01h],1
	RCL	BYTE PTR [BX+00h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+21h],1
	RCL	BYTE PTR [BX+20h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+41h],1
	RCL	BYTE PTR [BX+40h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+61h],1
	RCL	BYTE PTR [BX+60h],1
	DEC	DL
	JNZ	C_22E2
	ADD	BX,2
	DEC	DH
	JNZ	C_22E0
	MOV	DX,CX
	;--
	MOV	SI,OFFSET DGROUP:InterleaveBuf
	MOV	CX,40h
	REPZ	MOVSW
	;--
	MOV	CX,DX
	MOV	SI,DI
	LOOP	C_22DB
	;--
	POP	ES
	POP	DI
	POP	SI
	RET
InterleaveShapes	ENDP
;----------------------------------------
;(TITLE.EXE:_C_20B4/AVATAR.EXE:C_2341)
PUBLIC InterleaveCharset
InterleaveCharset	PROC NEAR
	PUSH	SI
	PUSH	DI
	PUSH	ES
	;--
	LES	SI,pCharset
	MOV	DI,SI
	MOV	CX,80h
C_234D:
	MOV	DH,8
	MOV	BX,OFFSET DGROUP:InterleaveBuf
C_2352:
	MOV	DL,4
C_2354:
	LODS	BYTE PTR ES:[SI]
	SHL	AL,1
	RCL	BYTE PTR [BX+00h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+08h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+10h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+18h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+00h],1                    
	SHL	AL,1
	RCL	BYTE PTR [BX+08h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+10h],1
	SHL	AL,1
	RCL	BYTE PTR [BX+18h],1                 
	DEC	DL
	JNZ	C_2354
	INC	BX
	DEC	DH
	JNZ	C_2352
	MOV	DX,CX
	;--
	MOV	SI,OFFSET DGROUP:InterleaveBuf
	MOV	CX,10h
	REPZ	MOVSW
	;--
	MOV	CX,DX
	MOV	SI,DI
	LOOP	C_234D
	;--
	POP	ES
	POP	DI
	POP	SI
	RET
InterleaveCharset ENDP
;----------------------------------------
_TEXT	ENDS
;########################################
END
