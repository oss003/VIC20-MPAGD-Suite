	beq tilescrolldown
	cmp #1
	beq screenscrolldown
	rts

tilescrolldown:
	lda $170f
	pha
	lda $170e
	sta $170f
	lda $170d
	sta $170e
	lda $170c
	sta $170d
	lda $170b
	sta $170c
	lda $170a
	sta $170b
	lda $1709
	sta $170a
	lda $1708
	sta $1709
	pla
	sta $1708
	rts

screenscrolldown:
	inc Reg1
	rts
