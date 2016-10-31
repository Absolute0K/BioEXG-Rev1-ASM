@ Reason it did not work: Using LRVC at the beggining of the subroutine

.MACRO	USART_INIT
USART_GPIO_CLOCK:
	MOV	R2,	#(1 << 1 | 1 << 2 | 1 << 3)
	LMS	R2,	AHB1,	RCC,	RCC_AHB1ENR

USART_CLOCK:
	MOV	R2,	#(1 << 4) 
	LMSC	R2,	RCC_APB2ENR

USART_GPIOD_INIT:
	MOV	R2,	#(1 << 24 | 1 << 26 | 1 << 28 | 1 << 30)
	LMS	R2,	AHB1,	GPIOD,	MODER

UASRT_GPIOB_CONF:
	LDR	R2,	=(2 << 12 | 2 << 14)
	LMS	R2,	AHB1,	GPIOB,	MODER
	
	LDR	R2,	=(2 << 12 | 2 << 14)
	LMSC	R2,	OSPEEDR

	MOV	R2,	#(7 << 24 | 7 << 28)
	LMSC	R2,	AFRL

USART_BAUD:
	@	84Mhz/115200
	LDR	R2,	=(84000000 / 115200)
	OSR	R2,	APB2,	USART1,	USART_BRR

USART_CR:
	@	Transmit, Recieve Enabled, USART1 Enabled 
	LDR	R2,	=(1 << 2 | 1 << 3 | 1 << 13)
	LMSC	R2,	USART_CR1
.ENDM


.MACRO	CTX
CTX:
WAIT_TXE:
	LRV	R3,	APB2,	USART1,	USART_SR
	TST	R3,	#(1 << 7)
	BEQ	WAIT_TXE

SEND_C:
	OSRC	R2,	USART_DR,	B

WAIT_TC:
	LRVC	R3,	USART_SR
	TST	R3,	#(1 << 6)
	BEQ	WAIT_TC
	BX	LR

.ENDM


.MACRO	CRX
CRX:
WAIT_RXNE:
	LRV	R3,	APB2,	USART1,	USART_SR
	TST	R3,	#(1 << 5)
	BEQ	WAIT_RXNE

READ_C:
	LRVC	R2,	USART_DR,	B
	BX	LR

.ENDM


.MACRO	STX
STX:
	PUSH	{LR}
	MOV	R1,	R2

RC_SEND:
	LDRB	R2,	[R1],	#0x1

	TST	R2,	#0xFF	@ Get out when R2 = 0x0
	BEQ	EXIT_STR

	BL	CTX
	B	RC_SEND

EXIT_STR:
	POP	{PC}

.ENDM


.MACRO	PREG
PREG:
	PUSH	{LR}
	MOV	R1,	R2	@ Save Register
	MOV	R4, #8		@ Counter

	MOV	R2,	#'0'
	BL	CTX

	MOV	R2,	#'x'
	BL	CTX

HEX_SEND:
	ROR	R1,	R1, #28
	AND	R2, R1,	#0xF	@ Extract 1 digit

	CMP	R2, #0xA
	ITE	GE

	ADDGE	R2, #55		@ Convert to ASCII (Capital letters)
	ADDLT	R2, #48 	@ Just use number 0-9
	BL	CTX

	SUBS	R4, #0x1
	BNE	HEX_SEND
	POP	{PC}

.ENDM

.MACRO	PDEC
PDEC:
@ Send Register value in Decimal
@ R0: SP; R1: Temp; R2: Register value;
@ R3: R2 / 10; R4: R2 - (R3 * 10) => Remainder


	PUSH	{LR}
	MOV	R0,	SP
	SUB	SP,	SP,	#12
	MOV	R1,	#0x0
	STRB	R1,	[R0, #-1]!

	MOV	R5, #10

DEC_LOOP:
	UDIV	R3,	R2,	R5
	MUL	R1,	R3,	R5
	SUB	R4,	R2,	R1
	ADD	R4,	R4,	#48
	STRB	R4,	[R0, #-1]!	

	MOVS	R2,	R3
	BNE	DEC_LOOP

	MOV	R2,	R0
	BL	STX
	ADD	SP,	SP,	#12
	POP	{PC}
.ENDM

