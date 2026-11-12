.segment "CODE"

RESET:
    SEI             ; Disable IRQs initially

    LDX #$FF
    TXS             ; Initialize stack pointer

    LDA #$00
    STA $0200       ; IRQ counter = 0
    STA $0201       ; Main program counter marker = 0

    CLI             ; Enable IRQs

MAIN:
    INC $0201       ; Increment main program counter marker

    LDX $0201	    ; Load Main program counter to X

    JMP MAIN


; ==========================================
; IRQ HANDLER
; ==========================================

IRQ:
    PHA             ; Save accumulator

    INC $0200       ; Increment IRQ counter
    
    LDY $0200	    ; Load IRQ counter to Y

    PLA             ; Restore accumulator

    RTI             ; Return from interrupt


; ==========================================
; INTERRUPT VECTORS
; ==========================================

.segment "VECTORS"

.word $0000         ; NMI
.word RESET         ; RESET
.word IRQ           ; IRQ / BRK
