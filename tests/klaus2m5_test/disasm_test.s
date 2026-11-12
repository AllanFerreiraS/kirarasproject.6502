; ============================================================================
; disasm_test.s - one instance of every legal NMOS 6502 opcode (151 opcodes)
;
; Purpose : feed a disassembler. The source line you read here is the text
;           your disassembler should print for those bytes (modulo case and
;           branch targets, which a disassembler prints as absolute addresses).
; WARNING : this is NOT a program to run. It is decoding material only.
;
; Build   : ca65 -l disasm_test.lst -o disasm_test.o disasm_test.s
;           ld65 -C disasm_test.cfg -o disasm_test.bin disasm_test.o
;           (add -D ILLEGAL_BYTES to ca65 to append a few undocumented bytes)
;
; Operand values are varied on purpose ($00, $01, $7F, $80, $FF, ...) to expose
; byte-order and sign bugs. "a:" forces a 16-bit address even when the value
; fits in one byte (otherwise ca65 would silently pick the zero page opcode).
; ============================================================================

        .setcpu "6502"          ; legal opcodes only: illegal ones are errors

        .segment "CODE"

rom_start:

        ; ---- ADC
        adc  #$00               ; immediate
        adc  $00                ; zero page
        adc  $01,x              ; zero page,X
        adc  $1234              ; absolute
        adc  $ABCD,x            ; absolute,X
        adc  $0200,y            ; absolute,Y
        adc  ($42,x)            ; (indirect,X)
        adc  ($80),y            ; (indirect),Y
        ; ---- AND
        and  #$01               ; immediate
        and  $FF                ; zero page
        and  $A5,x              ; zero page,X
        and  $FFFF              ; absolute
        and  $8000,x            ; absolute,X
        and  a:$00FF,y          ; absolute,Y
        and  ($10,x)            ; (indirect,X)
        and  ($FE),y            ; (indirect),Y
        ; ---- ASL
        asl  a                  ; accumulator
        asl  $00                ; zero page
        asl  $01,x              ; zero page,X
        asl  $0100              ; absolute
        asl  $C0DE,x            ; absolute,X
        ; ---- BIT
        bit  $42                ; zero page
        bit  $1234              ; absolute
        ; ---- BRK
        brk                     ; implied
        ; ---- CLC
        clc                     ; implied
        ; ---- CLD
        cld                     ; implied
        ; ---- CLI
        cli                     ; implied
        ; ---- CLV
        clv                     ; implied
        ; ---- CMP
        cmp  #$7F               ; immediate
        cmp  $80                ; zero page
        cmp  $FF,x              ; zero page,X
        cmp  $ABCD              ; absolute
        cmp  $0200,x            ; absolute,X
        cmp  $FFFF,y            ; absolute,Y
        cmp  ($A5,x)            ; (indirect,X)
        cmp  ($10),y            ; (indirect),Y
        ; ---- CPX
        cpx  #$80               ; immediate
        cpx  $FE                ; zero page
        cpx  $8000              ; absolute
        ; ---- CPY
        cpy  #$FF               ; immediate
        cpy  $00                ; zero page
        cpy  a:$00FF            ; absolute
        ; ---- DEC
        dec  $01                ; zero page
        dec  $42,x              ; zero page,X
        dec  $0100              ; absolute
        dec  $C0DE,x            ; absolute,X
        ; ---- DEX
        dex                     ; implied
        ; ---- DEY
        dey                     ; implied
        ; ---- EOR
        eor  #$A5               ; immediate
        eor  $80                ; zero page
        eor  $FF,x              ; zero page,X
        eor  $1234              ; absolute
        eor  $ABCD,x            ; absolute,X
        eor  $0200,y            ; absolute,Y
        eor  ($A5,x)            ; (indirect,X)
        eor  ($10),y            ; (indirect),Y
        ; ---- INC
        inc  $FE                ; zero page
        inc  $00,x              ; zero page,X
        inc  $FFFF              ; absolute
        inc  $8000,x            ; absolute,X
        ; ---- INX
        inx                     ; implied
        ; ---- INY
        iny                     ; implied
        ; ---- JMP
        jmp  $00FF              ; absolute
        jmp  ($ABCD)            ; (indirect)
        ; ---- JSR
        jsr  $C0DE              ; absolute
        ; ---- LDA
        lda  #$3C               ; immediate
        lda  $01                ; zero page
        lda  $42,x              ; zero page,X
        lda  $1234              ; absolute
        lda  $ABCD,x            ; absolute,X
        lda  $0200,y            ; absolute,Y
        lda  ($80,x)            ; (indirect,X)
        lda  ($FF),y            ; (indirect),Y
        ; ---- LDX
        ldx  #$C3               ; immediate
        ldx  $A5                ; zero page
        ldx  $10,y              ; zero page,Y
        ldx  $FFFF              ; absolute
        ldx  $8000,y            ; absolute,Y
        ; ---- LDY
        ldy  #$00               ; immediate
        ldy  $FE                ; zero page
        ldy  $00,x              ; zero page,X
        ldy  a:$00FF            ; absolute
        ldy  $0100,x            ; absolute,X
        ; ---- LSR
        lsr  a                  ; accumulator
        lsr  $01                ; zero page
        lsr  $42,x              ; zero page,X
        lsr  $C0DE              ; absolute
        lsr  $1234,x            ; absolute,X
        ; ---- NOP
        nop                     ; implied
        ; ---- ORA
        ora  #$01               ; immediate
        ora  $80                ; zero page
        ora  $FF,x              ; zero page,X
        ora  $ABCD              ; absolute
        ora  $0200,x            ; absolute,X
        ora  $FFFF,y            ; absolute,Y
        ora  ($A5,x)            ; (indirect,X)
        ora  ($10),y            ; (indirect),Y
        ; ---- PHA
        pha                     ; implied
        ; ---- PHP
        php                     ; implied
        ; ---- PLA
        pla                     ; implied
        ; ---- PLP
        plp                     ; implied
        ; ---- ROL
        rol  a                  ; accumulator
        rol  $FE                ; zero page
        rol  $00,x              ; zero page,X
        rol  $8000              ; absolute
        rol  a:$00FF,x          ; absolute,X
        ; ---- ROR
        ror  a                  ; accumulator
        ror  $01                ; zero page
        ror  $42,x              ; zero page,X
        ror  $0100              ; absolute
        ror  $C0DE,x            ; absolute,X
        ; ---- RTI
        rti                     ; implied
        ; ---- RTS
        rts                     ; implied
        ; ---- SBC
        sbc  #$7F               ; immediate
        sbc  $80                ; zero page
        sbc  $FF,x              ; zero page,X
        sbc  $1234              ; absolute
        sbc  $ABCD,x            ; absolute,X
        sbc  $0200,y            ; absolute,Y
        sbc  ($A5,x)            ; (indirect,X)
        sbc  ($10),y            ; (indirect),Y
        ; ---- SEC
        sec                     ; implied
        ; ---- SED
        sed                     ; implied
        ; ---- SEI
        sei                     ; implied
        ; ---- STA
        sta  $FE                ; zero page
        sta  $00,x              ; zero page,X
        sta  $FFFF              ; absolute
        sta  $8000,x            ; absolute,X
        sta  a:$00FF,y          ; absolute,Y
        sta  ($01,x)            ; (indirect,X)
        sta  ($42),y            ; (indirect),Y
        ; ---- STX
        stx  $80                ; zero page
        stx  $FF,y              ; zero page,Y
        stx  $0100              ; absolute
        ; ---- STY
        sty  $A5                ; zero page
        sty  $10,x              ; zero page,X
        sty  $C0DE              ; absolute
        ; ---- TAX
        tax                     ; implied
        ; ---- TAY
        tay                     ; implied
        ; ---- TSX
        tsx                     ; implied
        ; ---- TXA
        txa                     ; implied
        ; ---- TXS
        txs                     ; implied
        ; ---- TYA
        tya                     ; implied

        ; ---- branches: offset = target - (address of branch + 2)
        ; "*" is the address of the branch instruction itself
        bcc  *+10               ; relative, offset $08 (+8)
        bcc  *-10               ; relative, offset $F4 (-12)
        bcs  *+10               ; relative, offset $08 (+8)
        bcs  *-10               ; relative, offset $F4 (-12)
        beq  *+10               ; relative, offset $08 (+8)
        beq  *-10               ; relative, offset $F4 (-12)
        bmi  *+10               ; relative, offset $08 (+8)
        bmi  *-10               ; relative, offset $F4 (-12)
        bne  *+10               ; relative, offset $08 (+8)
        bne  *-10               ; relative, offset $F4 (-12)
        bpl  *+10               ; relative, offset $08 (+8)
        bpl  *-10               ; relative, offset $F4 (-12)
        bvc  *+10               ; relative, offset $08 (+8)
        bvc  *-10               ; relative, offset $F4 (-12)
        bvs  *+10               ; relative, offset $08 (+8)
        bvs  *-10               ; relative, offset $F4 (-12)
        ; edge cases of the signed 8-bit offset
        bne  *+129              ; relative, offset $7F (+127)
        bne  *-126              ; relative, offset $80 (-128)
        bne  *+2                ; relative, offset $00 (+0)
        bne  *                  ; relative, offset $FE (-2)

rom_end:

.ifdef ILLEGAL_BYTES
        ; undocumented opcodes, emitted as raw bytes (ca65 refuses them with
        ; .setcpu "6502"). Decide what your disassembler should show for these.
        .byte $02               ; KIL/JAM
        .byte $03, $10          ; SLO (indirect,X)
        .byte $A7, $20          ; LAX zero page
        .byte $EB, $30          ; SBC immediate (unofficial duplicate)
        .byte $FF, $34, $12     ; ISC absolute,X
.endif
