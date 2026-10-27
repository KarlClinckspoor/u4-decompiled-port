DGROUP	GROUP XOB,XO,XOE,c_common

XOB	SEGMENT PUBLIC WORD 'BSS'
XOB	ENDS
XO	SEGMENT PUBLIC WORD 'BSS'
XO	ENDS
XOE	SEGMENT PUBLIC WORD 'BSS'
XOE	ENDS

PUBLIC MonstersShown
PUBLIC Party
PUBLIC GraphicsType
PUBLIC pShapes
PUBLIC pAnim
PUBLIC TwoDrives
PUBLIC pCharset
PUBLIC txt_X
PUBLIC txt_Y
PUBLIC pTitle

;actually, the next segment should be
;generated as a COMDEF record

c_common	SEGMENT PUBLIC PARA 'BSS'
	ASSUME DS:DGROUP

MonstersShown	dw ?	;D_6E80 2 monster callback flag
Party	db 1F6h dup(?)	;D_6E82
GraphicsType	dw ?	;D_7078 graphic adapter
pShapes	dd ?	;D_707A
pAnim	dd ?	;D_707E
TwoDrives	dw ?	;D_7082
pCharset	dd ?	;D_7084
txt_X	dw ?	;D_7088
txt_Y	dw ?	;D_708A
pTitle	dd ?	;D_708C

c_common	ENDS

END
