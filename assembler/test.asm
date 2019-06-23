.orig #x3000
ARG1 .FILL	#120
ARG2 .FILL  #18

LD	R1,	ARG1
LD	R2,	ARG2
JSR DIV ;R0=18 R1=12
ADD R3 R0 #0
JSR MUL
JSR PRINT_DECIMAL ;Should print 18*12 (216)
HALT


;Performs R0:=R1*R2
;No overflow managed
;Works with negative numbers
;Consumes both R1 and R2
;Uses R3
MUL
  AND R0 R0 #0
  ADD R2 R2 #0
  MUL_LOOP
    BRz MUL_DONE
    AND R3 R2 #x1
    BRz MUL_SKIP
      ADD R0 R0 R1
    MUL_SKIP
    LSHF R1 R1 #1
    RSHFL R2 R2 #1
    BR MUL_LOOP
  MUL_DONE
  RET

;Performs R0:=R1/R2 and R1:=R1%R2
;If R2=0, set R0:=0xFFFF and return
;Does it work with negatives?
;Consumes both R1 and R2
;Uses R3 and R4
DIV
  AND R0 R0 #0
  ADD R2 R2 #0
  BRz DIV_ERROR
  NOT R3 R2
  ADD R3 R3 #1
  DIV_SHIFT
    LSHF R3 R3 #1
    ADD R4 R1 R3
    BRp DIV_SHIFT
    BRz DIV_FOUND
    RSHFA R3 R3 #1
    DIV_NEXT
      LSHF R0 R0 #1
      ADD R4 R1 R3
      BRn DIV_SKIP
      DIV_FOUND
        ADD R0 R0 #1
        ADD R1 R4 #0
        DIV_SKIP
        ADD R4 R3 R2
        BRz DIV_DONE
        RSHFA R3 R3 #1
        BR DIV_NEXT
  DIV_ERROR
  ADD R0 R2 #-1
  DIV_DONE
  RET

RESULT .blkw #1
OUT_STR .blkw #7
ARG    .fill #8
N100   .fill #100
N1000  .fill #1000
N10000 .fill #10000
ASCII_BASE .fill #48
BYE    .stringz "Halting..."

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
	
.end
