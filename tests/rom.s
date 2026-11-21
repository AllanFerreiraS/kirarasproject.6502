; $00 -> $1F ROM
; $20 -> $2F RAM, where $20 to $23 are mirrored (3 mirrors)
LDA #$01
STA $20
LDA #$02
STA $21
LDA #$03
STA $22
LDA #$04
STA $23
LDA #$05
STA $24
LDA #$06
STA $25
LDA #$07
STA $26
JMP $30

