EXTRN u4_puts:NEAR
EXTRN u4_putc:NEAR
EXTRN Gra_PutChar:NEAR
EXTRN AnimateMonsters:NEAR

EXTRN MSG_DSKERR:BYTE
EXTRN MSG_SYSERR:BYTE

EXTRN speed_info:WORD

EXTRN Party:BYTE

EXTRN txt_X:WORD
EXTRN txt_Y:WORD

PUBLIC cursor_delay
PUBLIC cursor_enabled

DGROUP	GROUP _DATA,_BSS
;########################################
_DATA	SEGMENT PUBLIC WORD 'DATA'
	ASSUME DS:DGROUP
;----------------------------------------
cursor_delay	dw 0	;D_31BC
cursor_delay_old	dw 0
cursor_enabled	dw 0FFFFh	;D_31C0 display cursor flag
;----------------------------------------
_DATA	ENDS
;########################################
_BSS	SEGMENT PUBLIC WORD 'BSS'
;----------------------------------------
;random seeds
PUBLIC seed_128
seed_128	db 10h dup(?)	;D_6966
;-- struct tArea --
PUBLIC Area
Area	db 500h dup(?)	;D_6976
;--
cursor_state	db ?	;D_6E76 cursor state {0, 1, 2, 3}
cursor_count	dw ?	;D_6E77 cursor update counter
	db ?	;padding?
;----------------------------------------
_BSS	ENDS
;########################################
_TEXT	SEGMENT PUBLIC BYTE 'CODE'
	ASSUME CS:_TEXT
;----------------------------------------C_3149
;[bp+04] wait delay in seconds
;[bp+06] allow interruption/prompt
PUBLIC _u_delay
_u_delay	PROC NEAR
	PUSH	BP
	MOV	BP,SP
	PUSH	SI
	;--
	MOV	cursor_count,1
	;--
	MOV	AX,cursor_delay
	MOV	cursor_delay_old,AX
	;--
	MOV	AH,2Ch	;get time
	INT	21h
	MOV	DL,DH	;seconds
	MOV	DH,0
	MOV	SI,DX
	ADD	SI,[BP+04]
	CMP	SI,60
	JB	C_317D
	;-- --
	SUB	SI,60
	;--
	TEST	WORD PTR [BP+06],0FFFFh
	JZ	C_318C	;cannot interrupt
	CALL	u_kbhit
	TEST	AX,0FFFFh
	JNZ	C_31E8
C_317D:	;--
	TEST	WORD PTR [BP+06],0FFFFh
	JZ	C_318C	;cannot interrupt
	CALL	u_kbhit
	TEST	AX,0FFFFh
	JNZ	C_31D9
C_318C:	;--
	CALL	AnimateMonsters
	;============
	;== cursor ==
	;============
	MOV	AX,[BP+06]
	AND	AX,cursor_enabled
	JZ	C_31CD
	;-- update shape
	MOV	AX,cursor_delay_old
	CMP	AX,cursor_delay
	JNZ	C_31B9
	DEC	cursor_count
	JNZ	C_31CD
	DEC	cursor_state
	AND	cursor_state,3
	;-- display
	MOV	AL,cursor_state
	ADD	AL,1Ch
	PUSH	AX
	CALL	Gra_PutChar
C_31B9:	;-- reload cursor counter
	MOV	AX,1
	ADD	AX,cursor_delay
	MUL	speed_info
	MOV	cursor_count,AX
	;--
	MOV	AX,cursor_delay
	MOV	cursor_delay_old,AX
	;=======
C_31CD:	;==   ==
	;=======
	MOV	AH,2Ch	;get time
	INT	21h
	MOV	BL,DH	;seconds
	MOV	BH,0
	CMP	SI,BX
	JNZ	C_317D
C_31D9:	;--
	MOV	AX,[BP+06]
	AND	AX,cursor_enabled
	JZ	C_31E8
	;-- delete cursor --
	MOV	AL,' '
	PUSH	AX
	CALL	Gra_PutChar
C_31E8:	;--
	POP	SI
	MOV	SP,BP
	POP	BP
	RET
_u_delay	ENDP
;----------------------------------------C_31ED
PUBLIC u_rand_a
u_rand_a	PROC NEAR
	PUSHF
	PUSH	CX
	PUSH	DI
	PUSH	ES
	;--
	MOV	AX,DS
	MOV	ES,AX
	STD
	;--
	MOV	CX,0Eh
	MOV	DI,OFFSET DGROUP:seed_128 + 0Eh
	CLC
	MOV	AL,[DI+01]
C_3200:	ADC	AL,[DI]
	STOSB
	LOOP	C_3200
	ADC	[DI],AL
	;--
	MOV	CX,10h
	MOV	DI,OFFSET DGROUP:seed_128 + 10h
C_320D:	DEC	DI
	INC	BYTE PTR [DI]
	LOOPZ	C_320D
	;--
	POP	ES
	POP	DI
	POP	CX
	POPF
	;--
	MOV	AL,seed_128[0]
	MOV	AH,0
	TEST	AL,0FFh
	;--
	RET
u_rand_a	ENDP
;----------------------------------------C_321E
PUBLIC u_rand_b
u_rand_b	PROC NEAR
	MOV	AL,seed_128[3]
	ADC	AL,seed_128[2]
	MOV	seed_128[2],AL
	XOR	seed_128[1],AL
	MOV	AL,seed_128[1]
	ADC	AL,seed_128[0]
	ROR	AL,1
	MOV	seed_128[0],AL
	MOV	seed_128[3],AL
	MOV	AH,0
	TEST	AX,0FFFFh
	RET
u_rand_b	ENDP
;----------------------------------------C_3241
;increment Party._tick
;returns:
; 1 if key is pressed
; 0 if not
PUBLIC u_kbhit
u_kbhit	PROC NEAR
	CALL	u_rand_a
	;-- increment (unsigned long)Party._tick
	INC	WORD PTR Party
	JNZ	C_324E
	INC	WORD PTR Party+2
C_324E:	;--
	MOV	AH,1	;get keyboard status
	INT	16h
	JZ	C_3267	;-- no key pressed
	CMP	AX,2E03h	;<CTRL C> key code ?
	JZ	C_3263	;remove <CTRL C>
	TEST	AX,0FFFFh
	JZ	C_3263	;remove <CTRL C>
	;-- returns 1
	MOV	AX,1
	JMP	SHORT C_326D
C_3263:	;-- remove <CTRL C>
	MOV	AH,0	;wait for keypress and read character
	INT	16h
C_3267:	;--
	CALL	AnimateMonsters
	;-- returns 0
	MOV	AX,0
C_326D:	;-- --
	OR	AX,AX
	;--
	RET
u_kbhit	ENDP
;----------------------------------------C_3270
;get keyboard scancode
;wait if necessary
PUBLIC u_kbread
u_kbread	PROC NEAR
	;-- wait a very long time,
	;-- and allow key interrupt
	MOV	AX,8081h
	PUSH	AX
	PUSH	AX
	CALL	_u_delay
	ADD	SP,4
	;--
	MOV	AH,0	;wait for keypress and read character
	INT	16h
	;--
	RET
u_kbread	ENDP
;----------------------------------------C_3280
;wait while key pressed
PUBLIC u_kbflush
u_kbflush	PROC NEAR
	;-- allow interrupts
	STI
	NOP
	NOP
	NOP
	CLI
	;--
	CALL	u_kbhit
	JZ	C_328F
	CALL	u_kbread
	JMP	u_kbflush
C_328F:	;--
	RET
u_kbflush	ENDP
;----------------------------------------
;get current default drive
PUBLIC u_getdrive
u_getdrive	PROC NEAR	;C_3290
	MOV	AH,19h	;get current default drive
	INT	21h
	INC	AL
	MOV	AH,0
	;--
	RET
u_getdrive	ENDP	;C_3290
;----------------------------------------
;select disk
PUBLIC u_setdrive
u_setdrive	PROC NEAR	;C_3299
	PUSH	BP
	MOV	BP,SP
	;--
	MOV	DL,[BP+04]
	DEC	DL
	MOV	AH,0Eh	;select disk
	INT	21h
	MOV	AH,0
	;--
	POP	BP
	RET	2
u_setdrive	ENDP	;C_3299
;----------------------------------------
;test file
PUBLIC u_fexist
u_fexist	PROC NEAR	;C_32AB
	PUSH	BP
	MOV	BP,SP
	;--
	MOV	DX,[BP+04]
	MOV	AL,0
	MOV	AH,3Dh	;open file
	INT	21h
	JNB	C_32BE
	;--
	MOV	AX,0
	JMP	SHORT C_32C7
C_32BE:	;--
	MOV	BX,AX
	MOV	AH,3Eh	;close file
	INT	21h
	;--
	MOV	AX,1
C_32C7:	;--
	POP	BP
	RET	2
u_fexist	ENDP	;C_32AB
;----------------------------------------
;piracy check function ?(not called)
;succes => 0
;failure => -1
PUBLIC u_diskcheck
u_diskcheck	PROC NEAR	;C_32CB
	PUSHF
	PUSH	BX
	PUSH	AX
	PUSH	CX
	PUSH	DX
	PUSH	DS
	PUSH	ES
	;--
	PUSH	BP
	MOV	BP,SP
	PUSH	DI
	PUSH	SI
	;-- set drive
	MOV	AH,19h	;get current default drive
	INT	21h
	CMP	AL,1
	JBE	C_32E1
	MOV	AL,0
C_32E1:	MOV	DL,AL	;drive
	;--
	MOV	SI,4	;4 tries
	MOV	CL,10h	;from sector 10h
nextSector:	;-- read one sector to c000:0000
	MOV	AX,(2 SHL 8) OR 1	;read 1 sector
	MOV	DH,0	;head 0
	MOV	CH,9	;track 9
	MOV	BX,0C000h
	MOV	ES,BX
	XOR	BX,BX	;to c000:0000
	INT	13h
	JB	C_3303
	;-- one track success
	DEC	CL
	JZ	C_330C	;finished
	MOV	SI,4	;4 tries
	JMP	nextSector
C_3303:	;-- failed this sector
	DEC	SI
	JNZ	nextSector
	;-- failure
	MOV	BX,0FFFFh
	JMP	C_330F	;(+nop)
C_330C:	;-- success
	MOV	BX,0
C_330F:	;--
	POP	SI
	POP	DI
	MOV	SP,BP
	POP	BP
	;--
	POP	ES
	POP	DS
	POP	DX
	POP	CX
	POP	AX
	MOV	AL,BL
	POP	BX
	POPF
	RET
u_diskcheck	ENDP	;C_32CB
;----------------------------------------
;set critical error handler
PUBLIC u_setint24
u_setint24	PROC NEAR	;C_331E
	PUSH	DS
	;--
	LEA	DX,Int24Handler
	MOV	AX,CS
	MOV	DS,AX
	MOV	AH,25h	;set new interrupt vector
	MOV	AL,24h	;Critical Error
	INT	21h
	;--
	POP	DS
	RET
u_setint24	ENDP	;C_331E
;----
Int24Handler:
	PUSH	BP
	MOV	BP,SP
	PUSH	BX
	PUSH	CX
	PUSH	DX
	PUSH	SI
	PUSH	DS
	PUSH	ES
	;--
	MOV	BX,SS
	MOV	DS,BX
	MOV	ES,BX
	TEST	AH,80h
	JNZ	C_3388
	;-- display disk error --
	PUSH	txt_Y
	PUSH	txt_X
	MOV	txt_Y,24	;18h
	MOV	txt_X,0
	;--
	LEA	AX,MSG_DSKERR
	PUSH	AX
	CALL	u4_puts
	;--
	MOV	AH,0	;wait for keypress and read character
	INT	16h
	;--
	MOV	txt_X,0
	MOV	SI,39	;27h
C_336C:	;--
	MOV	AX,' '
	PUSH	AX
	CALL	u4_putc
	;--
	DEC	SI
	JNZ	C_336C
	;--
	POP	txt_X
	POP	txt_Y
	;-- --
	POP	ES
	POP	DS
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	POP	BP
	MOV	AL,1
	IRET
C_3388:	;-- display fatal error
	LEA	AX,MSG_SYSERR
	PUSH	AX
	CALL	u4_puts
	;-- --
	POP	ES
	POP	DS
	POP	SI
	POP	DX
	POP	CX
	POP	BX
	POP	BP
	MOV	AL,2
	IRET
;----------------------------------------
_TEXT	ENDS
;########################################
END
