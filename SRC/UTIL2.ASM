EXTRN u4_puts:NEAR
EXTRN u4_putc:NEAR
EXTRN Gra_PutChar:NEAR
EXTRN RefreshView:NEAR

EXTRN MSG_DSKERR:BYTE
EXTRN MSG_SYSERR:BYTE

EXTRN speed_info:WORD

EXTRN Party:BYTE

EXTRN txt_X:WORD
EXTRN txt_Y:WORD

PUBLIC cursor_delay


DGROUP	GROUP _DATA,_BSS
;########################################
_DATA	SEGMENT PUBLIC WORD 'DATA'
	ASSUME DS:DGROUP
;----------------------------------------
cursor_delay	dw 0	;D_0672
cursor_delay_old	dw 0	;D_0674
cursor_enabled	dw 0FFFFh	;D_0676 display cursor flag
;----------------------------------------
_DATA	ENDS
;########################################
_BSS	SEGMENT PUBLIC WORD 'BSS'
;----------------------------------------
;random seeds
PUBLIC seed_128
seed_128	db 10h dup(?)	;D_8732
;-- struct tArea --
PUBLIC Area
Area	db 500h dup(?)	;D_8742
;--
cursor_state	db ?	;D_8C42 cursor state {0, 1, 2, 3}
cursor_count	dw ?	;D_8C43 cursor update counter
	db ?	;padding?
;----------------------------------------
_BSS	ENDS
;########################################
_TEXT	SEGMENT PUBLIC BYTE 'CODE'
	ASSUME CS:_TEXT
;----------------------------------------C_16CD
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
	JB	C_1701
	;-- --
	SUB	SI,60
	;--
	TEST	WORD PTR [BP+06],0FFFFh
	JZ	C_1710	;cannot interrupt
	CALL	_u_kbhit
	TEST	AX,0FFFFh
	JNZ	C_176C	;key has been pressed
C_1701:	;--
	TEST	WORD PTR [BP+06],0FFFFh
	JZ	C_1710	;cannot interrupt
	CALL	_u_kbhit
	TEST	AX,0FFFFh
	JNZ	C_175D	;key has been pressed
C_1710:	;--
	CALL	RefreshView
	;============
	;== cursor ==
	;============
	MOV	AX,[BP+06]
	AND	AX,cursor_enabled
	JZ	C_1751
	;-- update shape
	MOV	AX,cursor_delay_old
	CMP	AX,cursor_delay
	JNZ	C_173D
	DEC	cursor_count
	JNZ	C_1751
	DEC	cursor_state
	AND	cursor_state,3
	;-- display
	MOV	AL,cursor_state
	ADD	AL,1Ch
	PUSH	AX
	CALL	Gra_PutChar
C_173D:	;-- reload cursor counter
	MOV	AX,1
	ADD	AX,cursor_delay
	MUL	speed_info
	MOV	cursor_count,AX
	;--
	MOV	AX,cursor_delay
	MOV	cursor_delay_old,AX
	;=======
C_1751:	;==   ==
	;=======
	MOV	AH,2Ch	;get time
	INT	21h
	MOV	BL,DH	;seconds
	MOV	BH,0
	CMP	SI,BX
	JNZ	C_1701
C_175D:	;--
	MOV	AX,[BP+06]
	AND	AX,cursor_enabled
	JZ	C_176C
	;-- delete cursor --
	MOV	AL,' '
	PUSH	AX
	CALL	Gra_PutChar
C_176C:	;--
	POP	SI
	MOV	SP,BP
	POP	BP
	RET
_u_delay	ENDP
;----------------------------------------C_1771
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
C_1784:	ADC	AL,[DI]
	STOSB
	LOOP	C_1784
	ADC	[DI],AL
	;--
	MOV	CX,10h
	MOV	DI,OFFSET DGROUP:seed_128 + 10h
C_1791:	DEC	DI
	INC	BYTE PTR [DI]
	LOOPZ	C_1791
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
;----------------------------------------C_17A2
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
;----------------------------------------C_17C5
;increment Party._tick
;returns:
; 1 if key is pressed
; 0 if not
PUBLIC _u_kbhit
_u_kbhit	PROC NEAR
	CALL	u_rand_a
	;-- increment (unsigned long)Party._tick
	INC	WORD PTR Party
	JNZ	C_17D2
	INC	WORD PTR Party+2
C_17D2:	;--
	MOV	AH,1	;get keyboard status
	INT	16h
	JZ	C_17EB	;-- no key pressed
	CMP	AX,2E03h	;<CTRL C> key code ?
	JZ	C_17E7	;remove <CTRL C>
	TEST	AX,0FFFFh
	JZ	C_17E7	;remove <CTRL C>
	;-- returns 1
	MOV	AX,1
	JMP	SHORT C_17F1
C_17E7:	;-- remove <CTRL C>
	MOV	AH,0	;wait for keypress and read character
	INT	16h
C_17EB:	;--
	CALL	RefreshView
	;-- returns 0
	MOV	AX,0
C_17F1:	;-- --
	OR	AX,AX
	;--
	RET
_u_kbhit	ENDP
;----------------------------------------C_17F4
;get keyboard scancode
;wait if necessary
PUBLIC _u_kbread
_u_kbread	PROC NEAR
	;-- wait a very long time,
	;-- and allow key interrupt
	MOV	AX,8081h
	PUSH	AX
	PUSH	AX
	CALL	_u_delay
	ADD	SP,4
	;--
	MOV	AH,0
	INT	16h
	;--
	RET
_u_kbread	ENDP
;----------------------------------------C_1804
;wait while key pressed
PUBLIC _u_kbflush
_u_kbflush	PROC NEAR
	;-- allow interrupts
	STI
	NOP
	NOP
	NOP
	CLI
	;--
	CALL	_u_kbhit
	JZ	C_1813
	CALL	_u_kbread
	JMP	_u_kbflush
C_1813:	;--
	RET
_u_kbflush	ENDP
;----------------------------------------
;get current default drive
PUBLIC u_getdrive
u_getdrive	PROC NEAR	;C_1814
	MOV	AH,19h	;get current default drive
	INT	21h
	INC	AL
	MOV	AH,0
	;--
	RET
u_getdrive	ENDP	;C_1814
;----------------------------------------
;select disk
PUBLIC u_setdrive
u_setdrive	PROC NEAR	;C_181D
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
u_setdrive	ENDP	;C_181D
;----------------------------------------
;test file
PUBLIC u_fexist
u_fexist	PROC NEAR	;C_182F
	PUSH	BP
	MOV	BP,SP
	;--
	MOV	DX,[BP+04]
	MOV	AL,0
	MOV	AH,3Dh	;open file
	INT	21h
	JNB	C_1842
	;--
	MOV	AX,0
	JMP	SHORT C_184B
C_1842:	;--
	MOV	BX,AX
	MOV	AH,3Eh	;close file
	INT	21h
	;--
	MOV	AX,1
C_184B:	;--
	POP	BP
	RET	2
u_fexist	ENDP	;C_182F
;----------------------------------------
;piracy check function ?
;succes => 0
;failure => -1
PUBLIC u_diskcheck
u_diskcheck	PROC NEAR	;C_184F
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
	JBE	C_1865
	MOV	AL,0
C_1865:	MOV	DL,AL	;drive
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
	JB	C_1887
	;-- one track success
	DEC	CL
	JZ	C_1890	;finished
	MOV	SI,4	;4 tries
	JMP	nextSector
C_1887:	;-- failed this sector
	DEC	SI
	JNZ	nextSector
	;-- failure
	MOV	BX,0FFFFh
	JMP	C_1893	;(+nop)
C_1890:	;-- success
	MOV	BX,0
C_1893:	;--
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
u_diskcheck	ENDP	;C_184F
;----------------------------------------
;set critical error handler
PUBLIC u_setint24
u_setint24	PROC NEAR	;C_18A2
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
u_setint24	ENDP	;C_18A2
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
	JNZ	C_190C
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
C_18F0:	;--
	MOV	AX,' '
	PUSH	AX
	CALL	u4_putc
	;--
	DEC	SI
	JNZ	C_18F0
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
C_190C:	;-- display fatal error
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
