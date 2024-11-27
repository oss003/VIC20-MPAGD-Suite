	ldx #0
colscrloop:
	lda colscr,x
	sta $9400,x
	inx
	cpx #242
	bne colscrloop

	lda #<datscr
	sta srcdata+1
	lda #>datscr
	sta srcdata+2

	lda #<$1000
	sta desdata+1
	lda #>$1000
	sta desdata+2

; Copy picture to screen

	ldx #16
	ldy #0
srcdata:
	lda datscr,y
desdata:
	sta $1000,y
	iny
	bne srcdata
	inc srcdata+2
	inc desdata+2
	dex
	bne srcdata
	rts
