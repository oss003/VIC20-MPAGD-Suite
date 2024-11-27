;------------------------------------------------------------------------------
; Main routine
;
; Screen      = $200
; Colour attr.= $9600
; Char RAM    = $1000
;------------------------------------------------------------------------------

	.word load
load:   .word @end
	.word 2
	.byte $9e
	.byte .lobyte(main/1000 .mod 10) + $30
	.byte .lobyte(main/100 .mod 10) + $30
	.byte .lobyte(main/10 .mod 10) + $30
	.byte .lobyte(main/1 .mod 10) + $30
	.byte 0 
@end:   .word 0 

main:
	
	LDA $2194
	STA $900E	Bits 4-7 are auxiliary color information

	LDA $2195
	STA $900F	Screen and border color register

	LDA #$8C	Start of character memory
	STA $9005

	LDA #$96	Set # of columns
	STA $9002

	LDA #$17	Set # of rows
	STA $9003

	LDX #$1D
	LDA $124D,X
	STA $BF,X
	DEX
	BNE $1241

	LDY #$10
	JMP $00C0

;------------------------------------------------------------------------------
; Code copied to $c0
; LOAD 
;------------------------------------------------------------------------------

loadfile:
	lda #(EndFileName-FileName)
	ldx #<FileName
	ldy #>FileName
	jsr $ffba
	lda #1
	ldx #8 ; Disk device
	ldy #0
	jsr $ffba
	lda #0
	ldx #<$4000 ; target location low
	ldy #>$4000 ; target location high
	jsr $ffd5

FileName:
	.byte "TITLEDATA.PRG"
EndFileName: 

.C:124e  BD 74 12    LDA $1274,X
.C:1251  9D 00 10    STA $1000,X
.C:1254  E8          INX
.C:1255  D0 F7       BNE $124E
.C:1257  E6 C2       INC $C2
.C:1259  E6 C5       INC $C5
.C:125b  88          DEY
.C:125c  D0 F0       BNE $124E
.C:125e  A9 EF       LDA #$EF
.C:1260  8D 20 91    STA $9120	VIA2, Port B output register Keyboard scan
.C:1263  4E 21 91    LSR $9121	VIA2, Port A output register Keyboard scan
.C:1266  B0 FB       BCS $1263
.C:1268  4C 22 FD    JMP $FD22	Back to Basic

------------------------------------------------------------------------------
Set colour attribute and set screenchar byte
------------------------------------------------------------------------------

.C:126b  9D 00 96    STA $9600,X
.C:126e  8A          TXA
.C:126f  9D 00 02    STA $0200,X
.C:1272  CA          DEX
.C:1273  60          RTS
