;----------------------------------------------
; BASIC header
;----------------------------------------------

.segment "BASIC"

	.word load
load:   .word @end

; 10 PRINT CHR$(147)
	.byte $0a,$00,$99,$20,$c7,$28,$31,$34,$37,$29,$00,$2b,$12

; 20 PRINT"TURBOLOADER Y/N";:INPUT A$
	.byte $14,$00,$99,$22,"TURBOLOADER Y/N",$22,$3b,$3a,$85,$20,$41,$24,$00,$3f,$12

; 30 IF A$="Y" THEN SYS tload:SYS 41984
	.byte $1e,$00,$8b," A$",$b2,$22,"Y",$22,$20,$a7,$20,$9e,$20
	.byte .lobyte(tload/1000 .mod 10) + $30
	.byte .lobyte(tload/100 .mod 10) + $30
	.byte .lobyte(tload/10 .mod 10) + $30
	.byte .lobyte(tload/1 .mod 10) + $30
	.byte $3a,$9e," 41984",$00,$54,$12 

; 40 SYS pload
	.byte $28,$00,$9e,$20
	.byte .lobyte(pload/1000 .mod 10) + $30
	.byte .lobyte(pload/100 .mod 10) + $30
	.byte .lobyte(pload/10 .mod 10) + $30
	.byte .lobyte(pload/1 .mod 10) + $30
	.byte $00,$00

@end:   .word 0 

;----------------------------------------------
; Entry if selected "Y"
; Load the turboloader
;----------------------------------------------

tload:
	lda #(EndFileName-FileName)
	ldx #<FileName
	ldy #>FileName
	jsr $ffbd

; Which device to use (in A the file num 1 is OK - x with 8 is the type of device
; usually the first drive, and y set to 1 reads from the header
; of the file the location where to save it)

load_start:
	lda #1				; Load at addresss
	ldx #8				; Disk device
	ldy #1
	jsr $ffba

; load from disk to ram
	lda #0				; accumulator must be set to 0 to load the image
	ldx #<$ff			; target location low not set (got from header)
	ldy #>$ff			; target location high (got from header)
	jsr $ffd5
load_end:
	rts

;----------------------------------------------
; Entry if selected "N" and after turboloader
; Load the AGD game
;----------------------------------------------

pload:
	ldx #0				; Copy loader code to $0f00
load_loop:
	lda load_start,x
	sta $0f00,x
	inx
	cpx #(load_end-load_start)
	bne load_loop

	lda #$4c			; Add jmp $2000
	sta $0f00,x
	inx
	lda #$00
	sta $0f00,x
	inx
	lda #$20
	sta $0f00,x

	lda #(EndGameName-GameName)	; Set load parameters for AGD game
	ldx #<GameName
	ldy #>GameName
	jsr $ffbd
	jmp $0f00			; Load and start AGD game

FileName:
    .byte "TURBODISK"
EndFileName:

GameName:
    .byte "AGDGAME.PRG"
EndGameName:
