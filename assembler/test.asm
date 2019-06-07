.orig x3000
	LD  R1 ARG  ;R1:=8
	JSR FAC ;R0:=FAC(R1)
	JSR PRINT_DECIMAL
	LEA R0 BYE
	PUTS
	HALT

	
FAC_RET_REG .blkw #1
FAC_ARG     .blkw #1
FAC
	ST  R7 FAC_RET_REG ;save return register
	ST  R1 FAC_ARG ;save argument
	ADD R3 R1 #-1 ;R3 is counter-1
FAC_LOOP
	BRz FAC_RETURN ;return for 
	ADD R2 R3 #0  ;R2:=R3
	JSR MUL ;R0:=MUL(R1,R2), consumes R2
	ADD R1 R0 #0 ;R1:=R0
	ADD R3 R3 #-1 ;counter--
	BR  FAC_LOOP
FAC_RETURN
	LD  R7 FAC_RET_REG ;restore return register
	LD  R1 FAC_ARG ;restore argument
	RET ;return to R6

	
MUL ;R0:=R1*R2
	AND R0 R0 #0 ;R0:=0
	ADD R2 R2 #0 ;R3:=R2, CC=sign(R2)
MUL_LOOP
	BRz MUL_RETURN ;if(CC = 0) return
	ADD R0 R0 R1 ;R0 += R1
	ADD R2 R2 #-1
	BR  MUL_LOOP
MUL_RETURN
	RET

	
DIV ;R0:=R1/R2
	NOT R2 R2
	ADD R2 R2 #1 ;R2:=-R2
	AND R0 R0 #0
DIV_LOOP
	ADD R0 R0 #1
	ADD R1 R1 R2 ;R1-=R2
	BRn DIV_REVERT
	BRz DIV_RETURN
	BRp DIV_LOOP
DIV_REVERT
	ADD R0 R0 #-1
	NOT R2 R2
	ADD R2 R2 #1 ;R2:=-R2
	ADD R1 R1 R2
DIV_RETURN
	RET
	
	
;MUL10 ;R0:=R1*10
;	LSHF R0 R1 #3 ;R0:=R1*8
;	ADD  R0 R0 R1
;	ADD  R0 R0 R1
;	RET
;	
;MUL100 ;R0=R1*100, uses R2
;	LSHF  R0 R1 #6 ;R0:=R1*64
;	LSHF  R2 R1 #5 ;R1:=R1*32
;	ADD   R0 R0 R2 ;R0:=R1*96
;	LSHF  R2 R1 #2 ;R2:=R1*4
;	ADD   R0 R0 R2 ;R0:=R1*100
;	RET
;	
;MUL1000 ;R0=R1*1000, uses R2
;	LSHF  R0 R1 #4 ;R0:=R1*16
;	LSHF  R2 R1 #3 ;R2:=R1*8
;	ADD   R2 R2 R0 ;R2:=R1*24
;	NOT   R2 R2    ;R2:=~(R1*24)
;	ADD   R2 R2 #1 ;R2:=-R1*24
;	LSHF  R0 R1 #10;R0:=R1*1024
;	ADD   R0 R0 R2 ;R0:=R0+R2 = R1*1000
;	RET
;	
;MUL10000 ;R0=R1*1000, uses R2
;	LSHF  R0 R1 #4 ;R0:=R1*16
;	LSHF  R2 R1 #8 ;R2:=R1*256
;	ADD   R0 R0 R2 ;R0:=R1*272
;	LSHF  R2 R1 #9 ;R2:=R1*512
;	ADD   R0 R0 R2 ;R0:=R1*784
;	LSHF  R2 R1 #10;R2:=R1*1024
;	ADD   R0 R0 R2 ;R0:=R1*1808
;	LSHF  R2 R1 #13;R2:=R1*8192
;	ADD   R0 R0 R2 ;R0:=R1*1000
;	RET
	
	
STACK_REG .blkw #1
PRINT_DECIMAL
	ST R7 STACK_REG
	LD R6 ASCII_BASE			;R6 = ASCII_BASE
	LD R2 N10000				;R2 = 10000	
	LEA R5 OUT_STR				;R5 = OUT_STR
	ADD R1 R0 #0				;R1 = N
	JSR DIV      				;R0 = R1/R2 = N/10000 /// R1 = R1 % R2
	ADD R0 R0 R6				;R0 = TOASCII(DIV)
		STR R0 R5 #0
	LD R2 N1000
	JSR DIV      				;R0 = R1/R2 = N/1000 /// R1 = R1 % R2
	ADD R0 R0 R6				;R0 = TOASCII(DIV)
		STR R0 R5 #1
	LD R2 N100
	JSR DIV      				;R0 = R1/R2 = N/100 /// R1 = R1 % R2
	ADD R0 R0 R6				;R0 = TOASCII(DIV)
		STR R0 R5 #2
	AND R2 R2 #0
	ADD R2 R2 #10
	JSR DIV      				;R0 = R1/R2 = N/10 /// R1 = R1 % R2
	ADD R0 R0 R6				;R0 = TOASCII(DIV)
		STR R0 R5 #3
	ADD R0 R1 R6				;R0 = TOASCII(R1)
		STR R0 R5 #4
	AND R0 R0 #0
		STR R0 R5 #6
	ADD R0 R0 #10
		STR R0 R5 #5
	PUTS
	LD R7 STACK_REG
	RET
	
	
RESULT .blkw #1
OUT_STR .blkw #7
ARG    .fill #8
N100   .fill #100
N1000  .fill #1000
N10000 .fill #10000
ASCII_BASE .fill #48
BYE    .stringz "Halting..."
.end