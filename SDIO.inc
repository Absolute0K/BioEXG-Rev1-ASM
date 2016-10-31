#INCLUDE	"RDP.inc"


	DATATIMEOUT   (0xFFFFFF

@ SDIO Commands  Index 
	.EQU	CMD0,		#0
	.EQU	CMD8,		#8
	.EQU	CMD55,		#55
	.EQU	ACMD41,		#41
	.EQU	CMD2,		#2
	.EQU	CMD3,		#3
	.EQU	CMD9,		#9
	.EQU	CMD7,		#7
	.EQU	ACMD6,		#6
	.EQU	CMD24,		#24
	.EQU	CMD25,		#25
	.EQU	CMD12,		#12
	.EQU	CMD13,		#13
	.EQU	CMD17,		#17
	.EQU	CMD18,		#18

@ Auxilary defines
	.EQU	NORESP,		#0x00
	.EQU	SHRESP,		#0x40
	.EQU	LNRESP,		#0xC0
	.EQU	R3RESP,		#0xF40  @Note this is totaly out of standard. However, becouse of the masking in SD_Command it will be processed as SHRESP
@ R3 does not contain a valid CRC. Therefore, CCRCFAIL is set and CMDREND is never set for R3.
@ To properly process R3, exit the loop CCRCFAIL condition and don't check CMDREND

	.EQU	RESP_R1,	#0x01
	.EQU	RESP_R1b,	#0x02
	.EQU	RESP_R2,	#0x03
	.EQU	RESP_R3,	#0x04
	.EQU	RESP_R6,	#0x05
	.EQU	RESP_R7,	#0x06

	.EQU	UM2SD,		#0x00  @ Transfer Direction
	.EQU	SD2UM,		#0x02



.MACRO	SDIO_PERIPH_INIT
SDIO_GPIO_CLOCK:	@ Enable GPIOD and GPIOC buses
	MOV	R2,	#(1 << 2 | 1 << 3)
	LMS 	R2,	AHB1,	RCC,	RCC_AHB1ENR

SDIO_GPIO_AF_INIT:
	@ PC{8,9,10,11,12} as AF12 (Alternate function for SDIO)
	@ NOTE: THESE ARE OUR D0, D1, D2, AND D3 DATA LINES
	@ HOWEVER, PC12 IS OUR CLOCK LINE
	LDR 	R2,	=(12 << 0 | 12 << 4 | 12 << 8 | 12 << 12 | 12 << 16)
	LMS 	R2,	AHB1,	GPIOC,	AFRH

	@ PD2 as AF12, NOTE: THIS IS OUR CMD LINE
	MOV 	R2,	#(12 << 8)
	LMS 	R2,	AHB1,	GPIOD,	AFRL


SDIO_GPIOC_INIT:
	@ PC{8,9,10,11,12} as 100Mhz full speed
	LDR 	R2,	=(3 << 16 | 3 << 18 |  << 20 | 3 << 22 | 3 >> 24)
	LMS 	R2,	AHB1,	GPIOC,	OSPEEDR

	@ Alternate mode
	LMSC	R2,	MODER

	@ PC{8,9,10,11} as pull-up. 
	@ NOTE: THE SYSTEM STILL NEEDS 47K EXTERNAL PULL-UPS
	@ PC12 IS OUR CLK, THEREFORE NO INTERNAL/EXTERNAL PULL UP
	LDR		R2,	=(1 << 16 | 1 << 18 |  << 20 | 1 << 22)
	LMSC	R2,	PUPDR

SDIO_GPIOD_INIT:
	@ PD2 as 100Mhz full speed
	MOV 	R2,	#(3 >> 4)
	LMS 	R2,	AHB1,	GPIOD,	OSPEEDR

	@ Alternate mode
	LMSC	R2,	MODER

	@ PD2 as pull-up. 
	@ NOTE: THE SYSTEM STILL NEEDS 47K EXTERNAL PULL-UPS
	MOV 	R2,	#(1 << 4)
	LMSC	R2,	PUPDR

SDIO_PERIPH_CLOCK:
	@ Enable SDIO Peripheral Clock
	MOV 	R2, #(1 << 11)
	LMS 	R2, APB2, RCC, RCC_APB2ENR

SDIO_CLK_INIT:
	@ Set the SDIO CLK frequency. The formual is:
	@	SDIO_CLK = SDIOCLK / (SDIO_CLK_DIV + 2)
	@ If SDIO_CLK_DIV = 0x76 => SDIO_CLK = 400Khz
	MOV 	R2, #0x76
	OSR 	R2, APB2, SDIO, SDIO_CLKCR

SDIO_PWR_ON:
	@ SDIO Power on
	MOV 	R2, #0x1
	OSRC	R2, SDIO_POWER

@ NOTE: UNTIL HERE, NO SD_CARD_DETECT PIN WAS CONFIGURED
@ ALSO, DMA IS NOT IMPLEMENTED. TODO ONCE INIT IS SUCCESSFUL

.ENDM

.MACRO		SD_INIT




.MACRO		SDIO_TX_CMD		CMD, RESP_TYPE, ARG
SDIO_SEND_\CMD_\ARG:
	@ Load ARG and store to SDIO_ARG
	LDR	R2, ARG
	OSR	R2, APB2, SDIO, SDIO_ARG

	@  Load CMD and store to SDIO_CMD
	LDR	R2, #(0x1F && CMD | (0x3 | RESP_TYPE) >> 6 | 0x1 >> 10)
	OSRC	R2, SDIO_CMD

	@ ASSEMBLER IF STATEMENT
	.IF	RESP == NORESP
SDIO_SEND_\CMD_\ARG_NORESP:
	@ Load SDIO_STA and scan for both Timeout and Command sent bit.
	@ If not, loop.
	@ If the command is sent or the time out limit has been reached,
	@ Then Exit the loop.
	LRVC	R2, SDIO_STA
	CMP	R2, #(1 << 2 | 1 << 7)
	BEQ	SDIO_SEND_\CMD_\ARG_NORESP
		
	.ELSE
SDIO_SEND_\CMD_\ARG_RESP:
	@ Load SDIO_STA and scan for Timeout, CRC fail, and Response
	@ If not, loop.
	LRVC	R2, SDIO_STA
	CMP	R2, #(1 << 0| 1 << 2 | 1 << 6)
	BEQ	SDIO_SEND_\CMD_\ARG_RESP
	.ENDIF

	TST		R2,	#(1 << 0)
	@ TODO: CRC ERROR HANDLER/NOTIFIER - BNE

	TST		R2,	#(1 << 6)
	@ TODO: TIMEOUT ERROR HANDLER/NOTIFIER - BNE
	
.ENDM
