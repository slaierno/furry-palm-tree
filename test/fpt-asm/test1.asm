.orig #x3000

LD	R1,	ARG1
LD	R2,	ARG2
JSR NEXT

ARG1 .FILL	#120
ARG2 .FILL  #18

BYE    .stringz "Halting..."
STACK_REG .blkw #1
NEXT
    ST R2 STACK_REG
    LEA	R0,	BYE
    PUTS
    HALT

.end