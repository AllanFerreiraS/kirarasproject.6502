.setcpu "6502"

.segment "ZEROPAGE"

counter:    .res 1
result:     .res 1
temp:       .res 1
loop_count: .res 1

.segment "CODE"

.org $8000

reset:
	; Initialize registers
    	LDA #$00
	LDX #$05
	LDY #$03

    	STA counter
	STX result
	STY temp

	; Test arithmetic
	LDA #$10
	CLC
	ADC #$05
	STA result           ; result = $15

	; Test subtraction
	SEC
	SBC #$03
	STA temp             ; temp = $12

	; Test logical operations
	LDA #$F0
	AND #$0F
	STA counter          ; counter = $00

	LDA #$F0
	ORA #$0F
	STA counter          ; counter = $FF

	LDA #$AA
	EOR #$FF
	STA temp             ; temp = $55

	; Test shifts
	LDA #$01
	ASL A                ; A = $02
	ASL A                ; A = $04
	LSR A                ; A = $02
	STA counter

	; Test increment/decrement
	INC counter          ; $03
	DEC counter          ; $02

	INX                  ; X = $06
	DEX                  ; X = $05
	INY                  ; Y = $04
	DEY                  ; Y = $03

	; Test stack operations
	LDA #$42
	PHA

	LDA #$00
	PLA                  ; A = $42
	STA result

	; Test subroutine
	JSR calculate

	; Test loop and branch
	LDX #$05

loop:
	DEX
	BNE loop

	; Final values
	LDA #$AA
	STA $10

	LDA #$BB
	STA $11

	LDA #$CC
	STA $12

	JMP forever

calculate:
	LDA #$20
	CLC
	ADC #$22
	RTS                  ; A = $42

forever:
	JMP forever
