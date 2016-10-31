@ Send Register value in Decimal
@ R0: SP; R1: Temp; R2: Register value;
@ R3: R2 / 10; R4: R2 - (R3 * 10) => Remainder


	PUSH	{LR}
	MOV		R0,	SP
	SUB		SP,	SP,	#12
	MOV		R1,	#0x0
	STRB	R1,	[R0, #-1]!

DEC_LOOP:
	UDIV	R3,	R2,	#10
	MUL		R1,	R3,	#10
	SUB		R4,	R2,	R1
	ADD		R4,	R4,	#48
	STRB	R4,	[R0, #-1]!	

	MOVS	R2,	R3
	BNE		DEC_LOOP

	MOV		R2,	R0
	BL		STX
	ADD		SP,	SP,	#12
	POP		{LR}
