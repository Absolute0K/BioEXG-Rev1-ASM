@ My #17 Program - Establishing communication with the EEG platform
@ This program uses newly coded CLOCK macro and USART macro
@ Made in 08/08/13
@ Got everything working in 08/08/13
@ Made by me

@ Almost died because of: DID NOT KNOW THE IMPORTANCE OF LOGIC ANALYSERS!!!

@ Try to establish continuous communication
@ Then you are done :)


@ Constants:

	.EQU	STACK_TOP,		0x20000800

	.INCLUDE	"CLOCK.inc"
	.INCLUDE	"USART.inc"
	.INCLUDE	"SPI.inc"
	.INCLUDE	"INT.inc"
	.INCLUDE	"RDP.inc"
	.INCLUDE	"ADC.inc"

	.TEXT
	.SYNTAX	UNIFIED
	.THUMB
	.GLOBAL		_vectors
	.GLOBAL		START
	.GLOBAL		EXTI3
@	.GLOBAL		EXTI0
	.TYPE		START, %function
	.TYPE		EXTI3, %function
@	.TYPE		EXTI0, %function

/*
STATM:		.asciz		"Status and Multiplexer: "
DRGPIO:		.asciz		"Data rate and GPIO: "
ADC_DATA:	.asciz		"Data: "
			.align
*/

START:
	CLOCK_INIT								@ Initialize Clock @ 168Mhz	
	USART_INIT								@ Initialize USART
	SPI_INIT

ADC_INIT:
	DELAY	90000000						@ Wait a sec
	ADC_RESET
	BL		ADC_SELF_CALIB

ADC_REG_INIT:
	DELAY	600
	TSPI	0x0,	(1 << 1 | 1 << 2),	0x0,	0x0
	BL		DRDY_ONLINE

	DELAY	602
	TSPI	0x01,	0x67,	0x0,	0x0
	MOV		R2,	#(7 << 12)
	OSR		R2,	AHB1,	GPIOD,	ODR

	DELAY	604
	TSPI	0x02,	0x03,	0x0,	0x0
	BL		DRDY_ONLINE

	DELAY	601
	TSPI	0x03,	0x82,	0x0,	0x0
	BL		DRDY_ONLINE

	MOV		R2,	#(7 << 12)
	OSR		R2,	AHB1,	GPIOD,	ODR

/*
CALIB_O_BTN:
	@ Calibrate
	LRV		R2,	AHB1,	GPIOA,	IDR
	TST		R2,	#0x1
	BEQ		CALIB_O_BTN
	BL		ADC_SYS_OFFSET_CALIB

CALIB_G_BTN:
	@ Calibrate
	LRV		R2,	AHB1,	GPIOA,	IDR
	TST		R2,	#0x1
	BEQ		CALIB_G_BTN
	BL		ADC_SYS_GAIN_CALIB
*/
	INT_INIT								@ Initialize Interrupts

DEADLOOP:

	B	DEADLOOP

	@ SPI Subroutines
	SPI_R_CMD									@ SPI Subroutine
	SPI_W_CMD									@ SPI Subroutine
	DRDY_ONLINE									@ DRDY Online? Subroutine
	ADC_SELF_CALIB
	ADC_SYS_OFFSET_CALIB
	ADC_SYS_GAIN_CALIB
	
	@ USART Subroutines
	CTX
	CRX
	STX
	PDEC

	@ Interrupt Subroutines
	INT_EXTI3
@	INT_EXTI0
.END
