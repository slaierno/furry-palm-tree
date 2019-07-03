.orig #x3000

LD	R1,	ARG1
LD	R2,	ARG2
JSR DIV ;R0=6 R1=12 R2=18
ADD R3 R0 #0
JSR MUL
JSR PRINT_DECIMAL ;Should print 18*12 (216)
LEA R0 BYE
PUTS
HALT

ARG1 .FILL	#120
ARG2 .FILL  #18

;Performs R0:=R1*R2
;No overflow managed
;Works with negative numbers
;Consumes both R1 and R2
;Uses R3
MUL AND R0 R0 #0
    ADD R2 R2 #0
    MUL_LOOP  BRz MUL_DONE
              AND R3 R2 #x1
              BRz MUL_SKIP
              ADD R0 R0 R1
              MUL_SKIP
              LSHF R1 R1 #1
              RSHFL R2 R2 #1
              BR MUL_LOOP
    MUL_DONE  RET

;Performs R0:=R1/R2 and R1:=R1%R2
;If R2=0, set R0:=0xFFFF and return
;Does it work with negatives?
;Consumes both R1 and R2
;Uses R3 and R4
;Performs R0:=R1/R2 and R1:=R1%R2
;If R2=0, set R0:=0xFFFF and return
;Does it work with negatives?
;Consumes both R1 and R2
;Uses R3 and R4
DIV
  AND R0 R0 #0
  NOT R3 R2
  ADD R3 R3 #1 ;R3 is now -R2, 
               ;this because we do not have a SUB operation, 
               ;so we shift the negative divisor left and 
               ;right and add it instead of subtracting it
  BRz ERROR ;We cannot divide by 0. Note that (~0+1)==0
  SHIFT ;We shift the divisor up to the highest lower-than-dividend value
        ;Remember that R3 is the -R2 and every subtraction is an ADD
    LSHF R3 R3 #1
    ADD R4 R1 R3
    BRp SHIFT
    BRz FOUND ;If we are lucky, we do not need to shift back because
              ;the shift divisor is exactly the dividend.
              ;We can also skip a some instructions, because
              ;R4 already holds R1+R3
    RSHFA R3 R3 #1 ;Shift back right because we shifted R3 until it
                   ;was greater than R1 to check if it was the last
                   ;possible shift
  NEXT
    LSHF R0 R0 #1  ;Shift left the quotient. At the first cycle it shifts a 0, so it's non-effective
    ADD R4 R1 R3   ;If the shifted divisor is greater than the dividend/remainder...
    BRn SKIP       ;..don't add 1 to the quotient and don't update the remainder
  FOUND
    ADD R0 R0 #1   ;Add 1 to the quotient
    ADD R1 R4 #0   ;Update the dividend/remainder with the subtraction result
  SKIP ;can this skip be moved below "BRz DONE"?
    ADD R4 R3 R2   ;If the shifted divisor is equal to the divisor itself...
    BRz DONE       ;We are done, R0 is the quotient and R1 is the remainder
    RSHFA R3 R3 #1 ;Otherwise, begin shifting right R3...
    BR NEXT        ;...(we will shift left the quotient after the branch)
  ERROR
  ADD R0 R2 #-1    ;return -1 in case of error...?
  DONE
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
  ADD R0 R5 #0
	PUTS
	LD R7 STACK_REG
	RET
	
.end
