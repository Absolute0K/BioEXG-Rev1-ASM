
@ Interrupt Macro
@ This is the cleaned up version of the old code made in 02/02
@ NOTE: Use the transplant one by one method to successfully update a code
@ Made in 08/15/13
@ Made by me


.MACRO		INT_EXTI3
EXTI3:
	PUSH	{LR}

	@ Turn on all 4 LEDs
	LRV		R2,	AHB1,	GPIOD,	ODR
	EOR		R2,	R2,	#(1 << 15)
	OSRC	R2,	ODR

	RDSPI

	TST		R2,	#(1 << 15)
	BEQ		POSITIVE

	MOV		R4, R2
	MOV		R2, '-'
	BL		CTX
	SUB		R2,	R4,	#1
	LDR		R3,	=0xFFFF
	EOR		R2,	R2,	R3

POSITIVE:
	BL		PDEC

	MOV		R2,	'\n'
	BL		CTX

	MOV		R2,	#(1 << 3)
	LMS		R2,	APB2,	EXTI,	EXTI_PR

	POP		{PC}
.ENDM

/*
.MACRO		INT_EXTI0
EXTI0:
	PUSH	{LR}

@	ADC_SYS_CALIB
	MOV		R2,	#(0x1 << 12)
	OSR		R2,	AHB1,	GPIOD,	ODR 
	

	MOV		R2,	#(1 << 0)
	LMS		R2,	APB2,	EXTI,	EXTI_PR

	POP		{PC}
.ENDM
*/

@ The initialisation of the Interrupt

.MACRO	INT_INIT
INT_BUSES:
	@ Turn on the peripheral clock:
	LDR		R2,	=(1 << 14)
	LMS		R2,	AHB1,	RCC,	RCC_AHB1ENR

INT_GPIOE_SPEED:
	@ PA0 and PA1
	MOV		R2,	#(1 << 0 | 1 << 3)
	LMS		R2,	AHB1,	GPIOA,	OSPEEDR
	
INT_SET_PRIGROUP:
	@ Priority Group Set
	LDR		R2,	=(0x5FA << 16 | 3 << 8)
	OSR		R2,	CORE,	SCB,	SCB_AIRCR

INT_SET_PRILEVEL:
	@ Set Priority Level - EXTI0
	LDR		R0,	=[CORE + PRILEVEL]
	MOV		R1,	#0x6					@ Interrupt Number
	MOV		R2,	#(6 << 4)				@ Priority Level 6
	STRB	R2,	[R0, R1]					@ Address offset of the remainder
	DSB

	@ Set Priority Level - EXTI1
	MOV		R1,	#0x9					@ Interrupt Number
	MOV		R2,	#(7 << 4)				@ Priority Level 7
	STRB	R2,	[R0, R1]					@ Address offset of the remainder
	DSB

ENABLE_INT:
	@ Enable Interrupt
	CPSIE	I							@ Enable Interrupt
	LDR		R2,	=(1 << 6 | 1 << 9)			@ IRQ6 | EXT0
	LMS		R2,	CORE,	NVIC,	0x0

SET_EXTI_TRI:
	@ Remove Interrupt mask on line 0 & 8
	MOV		R2,	(1 << 0 | 1 << 3)			@ Enable interrupt line 0 & 1
	OSR		R2,	APB2,	EXTI,	EXTI_IMR		@ Remove Interrupt mask on line 0 & 1
	@ Set trigger level - Rising (RTSR)
	@OSRC	R2,	EXTI_RTSR					@ Set trigger level - Rising (RTSR)
	@ Set trigger level - Falling (FTSR)
	OSRC	R2,	EXTI_FTSR					@ Set trigger level - Falling (FTSR)

EXTI_SETUP:
	MOV		R2,	#0x0					@ Connect EXTI -> PA
	OSR		R2,	APB2,	SYSCFG,	SYSCFG_EXTICR1
	OSRC	R2,	SYSCFG_EXTICR3
.ENDM
