PUBLIC _SoundFlag

EXTRN u_rand_a:NEAR
EXTRN speed_info:WORD

DGROUP	GROUP _DATA
;########################################
_DATA	SEGMENT PUBLIC WORD 'DATA'
	ASSUME DS:DGROUP
;----------------------------------------
_SoundFlag	db 1	;D_06A6
SoundTable	dw OFFSET _TEXT:Snd_Step	;D_06A7
	dw OFFSET _TEXT:Snd_Blocked
	dw OFFSET _TEXT:Snd_Error
	dw OFFSET _TEXT:Snd_Shoot
	dw OFFSET _TEXT:Snd_Swing
	dw OFFSET _TEXT:Snd_FoeAttack
	dw OFFSET _TEXT:Snd_Hit
	dw OFFSET _TEXT:Snd_Hurt
	dw OFFSET _TEXT:Snd_Failed
	dw OFFSET _TEXT:Snd_Magic
	dw OFFSET _TEXT:Snd_Cast
	dw OFFSET _TEXT:Snd_Whirlpool
	dw OFFSET _TEXT:Snd_Twister
	db 0 ;padding ?
;----------------------------------------
_DATA	ENDS
;########################################
_TEXT	SEGMENT PUBLIC BYTE 'CODE'
	ASSUME CS:_TEXT
;----------------------------------------C_1D47
PUBLIC _sound
_sound	PROC NEAR
	PUSH	BP
	MOV	BP,SP
	;--
	MOV	BL,[BP+04]
	MOV	CL,[BP+06]
	MOV	CH,[BP+08]
	CMP	_SoundFlag,0
	JZ	C_1D67
	CMP	BL,0Dh
	JNB	C_1D67
	MOV	BH,0
	SHL	BX,1
	CALL	SoundTable[BX]
C_1D67:	;--
	POP	BP
	RET
_sound	ENDP
;----------------------------------------
Snd_Step	PROC NEAR	;C_1D69
	;-- backup port 61h
	IN	AL,61h
	MOV	AH,AL
	;--
	AND	AL,NOT 3
	XOR	AL,2
	OUT	61h,AL
	MOV	DI,32h
C_1D76:
	DEC	DI
	JNZ	C_1D76
	XOR	AL,2
	OUT	61h,AL
	;-- restore port 61h
	MOV	AL,AH
	OUT	61h,AL
	;--
	RET
Snd_Step	ENDP	;C_1D69
;----------------------------------------C_1D82
;same as beep for title.exe
Snd_Blocked	PROC NEAR
	MOV	AX,0CAh
	MUL	speed_info
	MOV	DX,AX
	;-- backup port 61h
	IN	AL,61h
	MOV	AH,AL
	;--
	AND	AL,NOT 3
	MOV	CX,10h
C_1D94:
	OUT	61h,AL
	XOR	AL,2
	MOV	BX,DX
C_1D9A:
	DEC	BX
	PUSHF
	PUSH	AX
	POP	AX
	POPF
	JNZ	C_1D9A
	LOOP	C_1D94
	;-- restore port 61h
	MOV	AL,AH
	OUT	61h,AL
	;--
	RET
Snd_Blocked	ENDP
;----------------------------------------
Snd_Error	PROC NEAR	;C_1DA8
	MOV	AX,0E0h
	MUL	speed_info
	MOV	DX,AX
	;-- backup port 61h
	IN	AL,61h
	MOV	AH,AL
	;--
	AND	AL,NOT 3
	MOV	CX,30h
C_1DBA:
	OUT	61h,AL
	XOR	AL,2
	MOV	BX,DX
C_1DC0:
	DEC	BX
	JNZ	C_1DC0
	LOOP	C_1DBA
	;-- restore port 61h
	MOV	AL,AH
	OUT	61h,AL
	;--
	CALL	Snd_Blocked
	;--
	RET
Snd_Error	ENDP	;C_1DA8
;----------------------------------------
Snd_Magic	PROC NEAR	;C_1DCD
	PUSH	BP
	MOV	BP,SP
	SUB	SP,4
	PUSH	speed_info
	;--
	SHR	speed_info,1
	ADC	speed_info,0
	MOV	AX,1Bh
	MUL	speed_info
	MOV	[BP-04],AX
	MOV	AX,speed_info
	MUL	CL
	MOV	DX,AX
	MOV	CX,speed_info
	;-- backup port 61h
	IN	AL,61h
	MOV	[BP-02],AX
	;--
	AND	AL,NOT 3
C_1DFC:
	MOV	AH,30h
C_1DFE:
	MOV	BX,DX
C_1E00:
	DEC	BX
	JNZ	C_1E00
	OUT	61h,AL
	XOR	AL,2
	MOV	BX,CX
C_1E09:
	DEC	BX
	JNZ	C_1E09
	OUT	61h,AL
	XOR	AL,2
	DEC	AH
	JNZ	C_1DFE
	SUB	DX,speed_info
	ADD	CX,speed_info
	CMP	CX,[BP-04]
	JNZ	C_1DFC
C_1E21:
	MOV	AH,30h
C_1E23:
	MOV	BX,DX
C_1E25:
	DEC	BX
	JNZ	C_1E25
	OUT	61h,AL
	XOR	AL,2
	MOV	BX,CX
C_1E2E:
	DEC	BX
	JNZ	C_1E2E
	OUT	61h,AL
	XOR	AL,2
	DEC	AH
	JNZ	C_1E23
	ADD	DX,speed_info
	SUB	CX,speed_info
	CMP	CX,0
	JNZ	C_1E21
	;-- restore port 61h
	MOV	AX,[BP-02]
	OUT	61h,AL
	;--
	POP	speed_info
	MOV	SP,BP
	POP	BP
	RET
Snd_Magic	ENDP	;C_1DCD
;----------------------------------------
Snd_Hurt	PROC NEAR	;C_1E53
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	BH,0FFh
C_1E5A:
	PUSH	AX
	CALL	u_rand_a
	AND	AL,7Fh
	OR	AL,40h
	MUL	BYTE PTR speed_info
	MOV	DX,AX
	SHR	DX,1
	db	83h,0cah,1	;OR	DX,+01
	POP	AX
C_1E6E:
	NOP
	NOP
	DEC	DX
	JNZ	C_1E6E
	OUT	61h,AL
	XOR	AL,2
	DEC	BH
	JNZ	C_1E5A
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Hurt	ENDP	;C_1E53
;----------------------------------------
Snd_Hit	PROC NEAR	;C_1E7F
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	BH,0FFh
C_1E86:
	PUSH	AX
	CALL	u_rand_a
	AND	AL,7Fh
	OR	AL,1
	MOV	AH,0
	MOV	CX,AX
	SHL	CX,1
	ADD	CX,AX
	MOV	AX,speed_info
	MUL	CX
	MOV	CX,AX
	SHR	CX,1
	SHR	CX,1
	db	83h,0c9h,1	;OR	CX,+01
	POP	AX
C_1EA5:
	LOOP	C_1EA5
	OUT	61h,AL
	XOR	AL,2
	DEC	BH
	JNZ	C_1E86
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Hit	ENDP	;C_1E7F
;----------------------------------------
Snd_Shoot	PROC NEAR	;C_1EB3
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	BL,5
C_1EBA:
	OUT	61h,AL
	XOR	AL,2
	MOV	CL,BL
	PUSH	AX
	MOV	AX,speed_info
	MUL	CL
	MOV	CX,AX
	SHR	CX,1
	POP	AX
C_1ECB:
	DEC	CX
	NOP
	NOP
	JNZ	C_1ECB
	INC	BL
	JNZ	C_1EBA
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Shoot	ENDP	;C_1EB3
;----------------------------------------
; uncalled ?
Snd_Unused	PROC NEAR	;C_1ED8
	MOV	AX,0A0h
	MUL	speed_info
	MOV	BX,AX
	;-- backup port 61h
	IN	AL,61h
	MOV	AH,AL
	;--
	AND	AL,NOT 3
	MOV	CH,0
C_1EE9:
	MOV	CX,BX
C_1EEB:
	DEC	CX
	JNZ	C_1EEB
	OUT	61h,AL
	XOR	AL,2
	SUB	BX,speed_info
	JNZ	C_1EE9
	;-- restore port 61h
	MOV	AL,AH
	OUT	61h,AL
	;--
	RET
Snd_Unused	ENDP	;C_1ED8
;----------------------------------------
Snd_Swing	PROC NEAR	;C_1EFD
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	BL,0
C_1F04:
	OUT	61h,AL
	XOR	AL,02
	MOV	CL,BL
	PUSH	AX
	MOV	AX,speed_info
	MUL	CL
	MOV	CX,AX
	SHR	CX,1
	POP	AX
C_1F15:
	DEC	CX
	NOP
	NOP
	JNZ	C_1F15
	DEC	BL
	JS	C_1F04
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Swing	ENDP	;C_1EFD
;----------------------------------------
Snd_Failed	PROC NEAR	;C_1F22
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	BL,80h
C_1F29:
	OUT	61h,AL
	XOR	AL,02
	MOV	CL,BL
	PUSH	AX
	MOV	AX,speed_info
	MUL	CL
	SHR	AX,1
	ADC	AX,0
	MOV	CX,AX
	POP	AX
C_1F3D:
	DEC	CX
	NOP
	NOP
	JNZ	C_1F3D
	DEC	BL
	JNZ	C_1F29
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Failed	ENDP	;C_1F22
;----------------------------------------
Snd_FoeAttack	PROC NEAR	;C_1F4A
	;-- backup port 61h
	IN	AL,61h
	MOV	AH,AL
	;--
	AND	AL,NOT 3
	MOV	BL,80h
C_1F52:
	OUT	61h,AL
	XOR	AL,02
	MOV	CL,BL
	PUSH	AX
	MOV	AX,speed_info
	MUL	CL
	SHR	AX,1
	MOV	CX,AX
	POP	AX
C_1F63:
	DEC	CX
	NOP
	NOP
	NOP
	NOP
	JNZ	C_1F63
	INC	BL
	JNZ	C_1F52
	;-- restore port 61h
	MOV	AL,AH
	OUT	61h,AL
	;--
	RET
Snd_FoeAttack	ENDP	;C_1F4A
;----------------------------------------
Snd_Cast	PROC NEAR	;C_1F73
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
C_1F78:
	PUSH	AX
	CALL	u_rand_a
	AND	AL,3Fh
	ADD	AL,40h
	MOV	DL,AL
	MOV	AX,speed_info
	MUL	DL
	SHL	AX,1
	MOV	DX,AX
	POP	AX
	MOV	CH,28h
C_1F8E:
	MOV	BX,DX
C_1F90:
	DEC	BX
	JNZ	C_1F90
	OUT	61h,AL
	XOR	AL,2
	DEC	CH
	JNZ	C_1F8E
	DEC	CL
	JNZ	C_1F78
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Cast	ENDP	;C_1F73
;----------------------------------------
Snd_Whirlpool	PROC NEAR	;C_1FA3
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	CX,40h
C_1FAB:
	MOV	AH,14h
C_1FAD:
	MOV	BX,CX
	XCHG	BX,AX
	MUL	speed_info
	XCHG	BX,AX
C_1FB5:
	NOP
	DEC	BX
	JNZ	C_1FB5
	OUT	61h,AL
	XOR	AL,2
	DEC	AH
	JNZ	C_1FAD
	INC	CX
	CMP	CX,0C0h
	JB	C_1FAB
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Whirlpool	ENDP	;C_1FA3
;----------------------------------------
Snd_Twister	PROC NEAR	;C_1FCC
	;-- backup port 61h
	IN	AL,61h
	PUSH	AX
	;--
	AND	AL,NOT 3
	MOV	CX,0C0h
C_1FD4:
	MOV	AH,14h
C_1FD6:
	MOV	BX,CX
	XCHG	BX,AX
	MUL	speed_info
	XCHG	BX,AX
C_1FDE:
	NOP
	DEC	BX
	JNZ	C_1FDE
	OUT	61h,AL
	XOR	AL,2
	DEC	AH
	JNZ	C_1FD6
	DEC	CX
	CMP	CX,40h
	JA	C_1FD4
	;-- restore port 61h
	POP	AX
	OUT	61h,AL
	;--
	RET
Snd_Twister	ENDP	;C_1FCC
;----------------------------------------
_TEXT	ENDS
;########################################
END
