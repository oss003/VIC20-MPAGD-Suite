; Game engine code --------------------------------------------------------------

; Arcade Game Designer.
; (C) 2008 Jonathan Cauldwell.
; ZX Spectrum Next Engine v0.1.

; Conditional compilation flags
; Code is installed if flag is set
; Flags are set in commandline assembly or by the compiler

; Flags set by AGD compiler
;	mflag  			; MENU + INV
;	pflag 			; particle engine
;	sflag 			; scrollytext
;	dflag 			; digging mode
;	cflag			; collectable blocks
;	oflag			; objects
;	lflag			; ladders
;	bflag			; big sprites (16x24)
;	rflag			; Pre-shifted byte tables
;
; Flags set manually
;	aflag			; adventure mode
;	iflag			; invert mode

.if mflag 
	.out "- MEN/INV enabled"  
.endif
.if pflag 
	.out "- Particles enabled" 
.endif
.if sflag 
	.out "- Scrolling enabled" 
.endif
.if dflag
	.out "- Digging enabled" 
.endif
.if cflag
	.out "- Collectable blocks enabled" 
.endif
.if oflag
	.out "- Objects enabled" 
.endif
.if lflag
	.out "- Ladders enabled" 
.endif
.if aflag
	.out "- Adventure mode enabled" 
.endif
.if bflag
	.out "- Big Sprites (16x24) enabled" 
.endif
.if iflag
	.out "- Invert mode enabled" 
.endif
.if rflag
	.out "- Pre-shifted byte tables enabled" 
.endif

;------------------------------------------------------------
; VIC20 check for PAL or NTSC
;------------------------------------------------------------

.if modelflag
	ScrWidth	= 22	; PAL settings
	ScrHeight	= 22
	CentreX		= 12
	CentreY		= 38
	CentreXMP   = 14
	CentreYMP   = 36	
	timervalue	= 24000
	timerint	= 24000			; Interrupt timer 1/50 sec
	.out "- PAL"
.else
	ScrWidth	= 22	; NTSC settings
	ScrHeight	= 22
	CentreX		= 4
	CentreY		= 27
	CentreXMP   = 6
	CentreYMP   = 24
	timervalue	= 20000
	timerint	= 20000			; Interrupt timer 1/50 sec
 	.out "- NTSC"
.endif
.out ""

;------------------------------------------------------------
; Constants MPAGD
;------------------------------------------------------------

; Global definitions

	FONT 		= font 
	MAP 		= MapAddr		; properties map buffer (3x256 bytes)
	SCADTB_lb	= MAP + $300		; Screen address table
	SCADTB_hb	= SCADTB_lb + $100
	SHRAPN 		= SCADTB_hb + $100	; shrapnel table (55x6 bytes)
	COLATTTB_lb	= SHRAPN + 55*6		; Colour attribute address table
	COLATTTB_hb	= COLATTTB_lb + 11

; Block characteristics.

	PLATFM	= 1		; platform.
	WALL	= PLATFM + 1	; solid wall.
	LADDER	= WALL + 1	; ladder.
	FODDER	= LADDER + 1	; fodder block.
	DEADLY	= FODDER + 1	; deadly block.
	CUSTOM	= DEADLY + 1	; custom block.
	WATER	= CUSTOM + 1	; water block.
        COLECT	= WATER + 1	; collectable block.
        NUMTYP	= COLECT + 1	; number of types.

; Sprites.

.if bflag 
	SPR_HGT	= 24		; Sprite height
	SPR_WID = 16		; Sprite width
	NUMSPR	= 8		; number of sprites.
.else				; 16x16 sprites
	SPR_HGT = 16		; Sprite height
	SPR_WID = 16		; Sprite width
	NUMSPR	= 12		; number of sprites.
.endif
	TABSIZ = 18			; size of each entry.
	SPRBUF = NUMSPR * TABSIZ; size of entire table.
	NMESIZ = 4			; bytes stored in nmetab for each sprite.

; Sprite table variable offsets.

	var_Type = 0		; sprite type
	var_Image = 1		; sprite time number
	var_Frame = 2		; sprite frame
	var_Y = 3			; sprite y coordinate
	var_X = 4			; sprite X coordinate

	var_newType = 5		; sprite new type
	var_newImage = 6	; sprite new image number
	var_newFrame = 7	; sprite new frame
	var_newY = 8		; sprite new y coordinate
	var_newX = 9		; sprite new x coordinate
	
	var_Direction = 10	; sprite direction
	var_Param1 = 11		; sprite parameter 1
	var_Param2 = 12		; sprite parameter 2

	var_jumpLo = 13		; sprite jump ptr low
	var_jumpHi = 14		; sprite jump ptr high
	var_dataLo = 15		; sprite data ptr low
	var_dataHi = 16		; sprite data ptr high
	var_colour = 17		; sprite colour

; Particle engine.

	NUMSHR = 55			; pieces of shrapnel.
	SHRSIZ = 6			; bytes per particle.

.if iflag
	TxtInvert   = $ff	; Invert byte for character printing
	ScrFillByte = $ff	; Screen fill byte for CLS
.else
	TxtInvert   = $00	; Invert byte for character printing
	ScrFillByte = $00	; Screen fill byte for CLS
.endif

	ASCII_NEWLINE = 13

;===============================================================
; Game starts here
;===============================================================

;--------------------------------------------------------------
; If a font is required...
;--------------------------------------------------------------

start:

; Clear variables

	lda #0
	tax
clrloop:
	sta 0,x
	inx
	bne clrloop

	jsr game	 		; start the game.
	jmp start

; Don't change the order of these four.  
; Menu routine relies on winlft following wintop.

wintop:	.byte WINDOWTOP		; top of window.
winlft:	.byte WINDOWLFT		; left edge.
winhgt:	.byte WINDOWHGT		; window height.
winwid:	.byte WINDOWWID		; window width.
numob:	.byte NUMOBJ		; number of objects in game.

; Pixel versions of wintop, winlft, winhgt, winwid.

wntopx:	.byte (8 * WINDOWTOP)
wnlftx:	.byte (8 * WINDOWLFT)
wnbotx:	.byte ((WINDOWTOP * 8) + (WINDOWHGT * 8) - 16)
wnrgtx:	.byte ((WINDOWLFT * 8) + (WINDOWWID * 8) - 16)

; Make sure pointers are arranged in the same order as the data itself.

frmptr:	.word frmlst        ; sprite frames.

; Assorted game routines which can go in contended memory.

;--------------------------------------------------------------
; Modify for inventory.
; called by the INV command
;
; Input:
;  X   = message nr with objects seperated with ,
;
; Output:
;  OPT = selected line nr of INV menu
;--------------------------------------------------------------

.if mflag
minve:
.if xflag
	lda #WINDOWHGT
	asl a
	sta winhgt
	lda #WINDOWWID
	asl a
	sta winwid
.endif
	lda #<(invdis)		; routine address.
	sta mod0+1		; set up menu routine.
	sta mod2+1		; set up count routine.
	lda #>(invdis)
	sta mod0+2
	sta mod2+2
	lda #<(fopt)		; find option from available objects.
	sta mod1+1		; set up routine.
	lda #>(fopt)
	sta mod1+1+1
	jmp dbox		; do menu routine.

;--------------------------------------------------------------
; Modify for menu.
; called by the MENU command
;
; Input:
;  X   = message nr with menu items seperated with ,
;
; Output:
;  OPT = selected line nr of MENU menu
;--------------------------------------------------------------

mmenu:
	lda #<(always)		; routine address.
	sta mod0+1		; set up routine.
	sta mod2+1		; set up count routine.
	lda #>(always)
	sta mod0+2
	sta mod2+2

	lda #<(fstd)		; standard option selection.
	sta mod1+1		; set up routine.
	lda #>(fstd)
	sta mod1+2

; Drop through into box routine.

;--------------------------------------------------------------
; Work out size of box for message or menu.
;--------------------------------------------------------------

dbox:
	lda #<msgdat		; pointer to messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
	
	jsr getwrd		; get message number.

	lda z80_h		; store pointer to message.
	sta TmpAddr
	lda z80_l
	sta TmpAddr+1

	lda #1			; height.
	sta z80_d
	lda #0			; start at object zero.
	sta combyt		; store number of object in combyt.
	sta z80_e		; maximum width.
dbox5:
	lda #0			; this line"s width.
	sta z80_b
mod2:
	jsr always		; item in player"s possession?
	cmp #255
	bne dbox6		; not in inventory, skip this line.
	inc z80_d		; add to tally.
dbox6:
	ldy #0			; get character.
	lda (z80_hl),y
	sta z80_a
	inc z80_l		; next character.
	bne :+
	inc z80_h
:
	lda z80_a		; reached end of line?
	cmp #','
	beq dbox3		; yes.
	cmp #ASCII_NEWLINE
	beq dbox3		; yes.
	inc z80_b		; add to this line"s width.
	lda z80_a
	bmi dbox4		; end of message? yes, end count.
	jmp dbox6		; repeat until we find the end.
dbox3:
	lda z80_e		; maximum line width.
	cmp z80_b		; have we exceeded longest so far?
	bpl dbox5		; no, carry on looking.
	lda z80_b		; make this the widest so far.
	sta z80_e
	jmp dbox5		; keep looking.
dbox4:
	lda z80_e		; maximum line width.
	cmp z80_b		; have we exceeded longest so far?
	bpl dbox8		; no, carry on looking.
	lda z80_b		; final line is the longest so far.
	sta z80_e
dbox8:
	dec z80_d		; decrement items found.
	bne :+			; total was zero.
	lda #255
	sta varopt
	jmp dbox15		
:
	lda z80_e		; longest line.
	bne :+			; was it zero?
	jmp dbox15		; total was zero.
:
	sta bwid		; set up size.
	lda z80_d
	sta blen

;--------------------------------------------------------------
; That's set up our box size.
;--------------------------------------------------------------

	lda winhgt		; window height in characters.
	sec
	sbc z80_d		; subtract height of box.
	lsr a			; divide by 2.
	clc
	adc wintop		; add top edge of window.
	sta btop		; set up box top.

	lda winwid		; window width in characters.
	sec	
	sbc z80_e		; subtract box width.
	lsr a			; divide by 2.
	clc
	adc winlft		; add left edge of window.
	sta blft		; box left.

	lda #<(FONT-256)		; font.
	sta grbase		; set up for text display.
	lda #>(FONT-256)
	sta grbase+1

	lda TmpAddr+1		; restore message pointer.
	sta z80_l
	lda TmpAddr
	sta z80_h

	lda btop		; box top.
	sta dispy		; set display coordinate.
	lda #0			; start at object zero.
	sta combyt		; store number of object in combyt.
dbox2:
	lda combyt		; get object number.
	sta z80_a
mod0:
	jsr always		; check inventory for display.
	cmp #255
	beq :+			
	jmp dbox13		; not in inventory, skip this line.
:
	lda blft		; box left.
	sta dispx		; set left display position.
	lda bwid		; box width.
	sta z80_b		; store width.
dbox0:
	ldy #0			; get character.
	lda (z80_hl),y
	cmp #','		; end of line?
	beq dbox1		; yes, next one.
	cmp #ASCII_NEWLINE			; end of line?
	beq dbox1		; yes, next one.

	cmp #141			; end of line?
	bne :+
	dec bwid
	jmp dbox7		; yes, next one.
:
	dec z80_b		; one less to display.
	and #127		; remove terminator.

	jsr pchr		; display on screen.

	ldy #0
	lda (z80_hl),y		; get character.
	sta z80_a
	inc z80_l		; next character.
	bne :+
	inc z80_h
:
	lda z80_a
	cmp #128		; end of message?
	bmi :+
	jmp dbox7		; yes, job done.
:
	lda z80_b		; chars remaining.
	beq :+			; are any left?
	jmp dbox0		; yes, continue.
:
;---------------------------------------------------
; Reached limit of characters per line.
;---------------------------------------------------

dbox9:
	ldy #0
	lda (z80_hl),y		; get character.
	inc z80_l		; next one.
	bne :+
	inc z80_h
:
	cmp #','		; another line?
	beq dbox10		; yes, do next line.
	cmp #ASCII_NEWLINE	; another line?
	beq dbox10		; yes, do next line.
	cmp #128		; end of message?
	bcs :+
	jmp dbox11		; yes, finish message.
:
	jmp dbox9

;---------------------------------------------------
; Fill box to end of line.
;---------------------------------------------------

dboxf:
	lda #32			; space character.
	jsr pchr		; display character.
	dec z80_b
	beq :+
	jmp dboxf		; repeat for remaining chars on line.
:
	rts
dbox1:
	inc z80_l		; skip character.
	bne :+
	inc z80_h
:
	jsr dboxf		; fill box out to right side.
dbox10:
	inc dispy		; y coordinate down a line next position.
	jmp dbox2		; next line.
dbox7:
	lda z80_b		; chars remaining.
	bne :+			; are any left?
	jmp dbox11		; no, nothing to draw.
:
	jsr dboxf		; fill message to line.

;------------------------------------------------------
; Drawn the box menu, now select option.
;------------------------------------------------------

dbox11:
	lda btop		; box top.
	sta dispy		; set bar position.
dbox14:
	jsr joykey		; get controls.
	cmp #$7f		; anything pressed?
	bne dbox14		; yes, debounce it.
	jsr dbar		; draw bar.
dbox12:
	jsr joykey		; get controls.
	cmp #$7f		; anything pressed?
	beq dbox12		; no, nothing.
	and #16			; fire button pressed?
	bne :+
mod1:
	jmp fstd		; yes, job done.
:
	jsr dbar		; delete bar.

	lda joyval		; joystick reading.
	and #8			; going up?
	beq dboxu		; yes, go up.

	ldx dispy		; vertical position of bar.
	inx			; look down.
	txa
	sec
	sbc btop		; find distance from top.
	cmp blen		; top of box.
	bne :+
	jmp dbox14		; yes, go no further.
:
	inc dispy		; move bar.
	jmp dbox14		; continue.
dboxu:
	lda dispy		; vertical position of bar.
	cmp btop		; are we at the top?
	bne :+
	jmp dbox14		; yes, go no further.
:
	dec dispy		; move bar.
	jmp dbox14		; continue.
fstd:
	lda dispy		; bar position.
	sec
	sbc btop		; find selected option.
	sta varopt		; store the option.
	jmp redraw		; redraw the screen.

;------------------------------------------------------
; Option not available.  Skip this line.
;------------------------------------------------------

dbox13:
	ldy #0
	lda (z80_hl),y		; get character.
	inc z80_l		; next one.

	bne :+
	inc z80_h
:
	cmp #','		; another line?
	bne :+
	jmp dbox2		; yes, do next line.
:
	cmp #ASCII_NEWLINE			; another line?
	bne :+
	jmp dbox2		; yes, do next line.
:

	bpl :+			; end of message?
	jmp dbox11		; yes, finish message.
:
	jmp dbox13
dbox15:
.if xflag
	lda #WINDOWWID
	sta winwid
	lda #WINDOWHGT
	sta winhgt
.endif
	lda TmpAddr		; pop message pointer from the stack.
	sta z80_h
	lda TmpAddr+1
	sta z80_l
	rts

;------------------------------------------------------
; Invert bar
;------------------------------------------------------

dbar:
	lda blft		; box left.
	sta dispx		; set display coordinate.
	jsr gprad		; get printing address.

	lda bwid		; box width.
	sta z80_c		; loop counter in c.
	lda z80_h		
	sta z80_d		; store screen address high byte.
dbar1:
	ldx #7			; pixel height in b.
dbar0:
	ldy scrtab,x
	lda (scraddr),y		; get screen byte.
	eor #255		; reverse all bits.
	sta (scraddr),y		; write back to screen.
	dex			; next line down.
	bpl dbar0		; draw rest of character.

	lda scraddr
	adc #16
	sta scraddr
	bcc :+
	inc scraddr+1
:
	dec z80_c		; decrement character counter.
	bne dbar1		; repeat for whole line.
	rts

;------------------------------------------------------
; Point to object
;
; Input:
;  -
;
; Output:
;  A = object number, A=255 if already in possession
;------------------------------------------------------

invdis:
	lda z80_l		; store message text pointer.
	pha
	lda z80_h
	pha
	lda combyt		; object number.
	inc combyt		; ready for next one.
	jsr gotob		; check if we have object.
	tay
	pla
	sta z80_h
	pla
	sta z80_l
	tya
	rts

;------------------------------------------------------
; Find option selected.
;
; Input:
;  -
;
; Output:
;  OPT = selected object
;------------------------------------------------------

fopt:
	lda dispy
	sec
	sbc btop		; find selected option.
	sta tmp+2		; option selected in b register.
	inc tmp+2

	lda #0			; set to first item.
	sta combyt		; object number.
fopt0:
	jsr fobj		; find next object in inventory.
	dec tmp+2
	bne fopt0		; repeat for relevant steps down the list.

	lda combyt		; get option.
	sta varopt		; store the option.
	dec varopt		; one less, due to where we increment combyt.
	jmp redraw		; redraw the screen.
fobj:
	ldy combyt		; object number.
	inc combyt		; ready for next item.
	tya
	jsr gotob		; do we have this item?
	cmp #255
	bne :+
	rts
:
	jmp fobj		; yes, it's on the list.
.endif

;----------------------------------------------------
; Clear sprite table.
;
; sprtab[0] - sprtab[SPRBUF-1] = 255
;----------------------------------------------------

xspr:
	lda #255		; clear byte.
	ldx #0			; length of table.
xspr0:
	sta sprtab,x		; sprite table.
	inx			; move to next byte.
	cpx #SPRBUF
	bne xspr0		; repeat for rest of table.
	rts

;-------------------------------------------------------------
; Initialise all objects.
;
; Reset current room,y,x to start room,y,x for all objects
;-------------------------------------------------------------

.if oflag
iniob:
	lda #<objdta		; objects table.
	sta z80_x
	lda #>objdta
	sta z80_i

	ldx numob 		; number of objects in the game.
iniob0:
	ldy #36
	lda (z80_ix),y 		; start screen.
	ldy #33
	sta (z80_ix),y 		; set start screen.

	ldy #37
	lda (z80_ix),y 		; find start y.
	ldy #34
	sta (z80_ix),y 		; set start y.

	ldy #38
	lda (z80_ix),y 		; get initial x.
	ldy #35
	sta (z80_ix),y 		; set x coord.

	clc 			; point to next object.
	lda z80_x
	adc #39			; distance between objects.
	sta z80_x
	bcc :+
	inc z80_i
:
	dex 			; repeat.
	bne iniob0

	rts
.endif

;-----------------------------------------------
; Redraw the screen.
;
; Remove old copy of all sprites for redraw.
;-----------------------------------------------

redraw:
.if xflag
	lda #WINDOWWID
	sta winwid
	lda #WINDOWHGT
	sta winhgt
.endif
	lda z80_i 		; place sprite pointer on stack.
	pha
	lda z80_x
	pha

	jsr droom		; show screen layout.
.if oflag
	jsr shwob		; draw objects.
.endif
numsp0:
	lda #NUMSPR		; sprites to draw.
	sta tmp

	lda #<sprtab		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
redrw0:
	ldy #0
	lda (z80_ix),y		; old sprite type.
	cmp #255		; is it enabled?
	beq redrw1 		; no, find next one.

	ldy #var_Y
	lda (z80_ix),y 		; sprite y.
	cmp #SpriteMaxY		; beyond maximum?
	bcs redrw1		; yes, nothing to draw.

	jsr sspria		; show single sprite.

redrw1:
	clc			; next sprite.
	lda z80_x
	adc #TABSIZ		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec tmp			; repeat for remaining sprites.
	bne redrw0

rpblc1:
;	jsr dshrp		; redraw shrapnel.

.if aflag
	jsr rbloc		; draw blocks for this screen
.endif
	pla			; retrieve sprite pointer.
	sta z80_x
	pla
	sta z80_i

	rts

;----------------------------------------------------------------------
; Clear screen routine.
;
; Fill screenmem $9400-$97ff with ScrFillByte
;----------------------------------------------------------------------

cls:
	lda #>CharAddress	; Character RAM address.
	sta clrdata+2

; Clear bitmap data

	lda #ScrFillByte	; Clear screen
	ldy #0
	ldx #16
clrdata:
	sta CharAddress,y
	iny
	bne clrdata
	inc clrdata+2
	dex
	bne clrdata

; Clear colour attributes

	lda fontcol
	ldx #0
clrcolour:
	sta ColorAttr-$200,x
	sta ColorAttr,x
	inx
	bne clrcolour
	rts

;----------------------------------------------------------------------
; Clear colour attributes
;
; Reset colour attributes to fontcol
;----------------------------------------------------------------------

clrcol:
	lda fontcol		; Clear colour attributes
	ldx #0
clscol:
	sta ColorAttr,x
	inx
	cpx #242
	bne clscol

	rts

;----------------------------------------------------------------------
; FODDER check
;----------------------------------------------------------------------

.if pflag .or dflag
fdchk:
	cmp #FODDER 		; is it fodder?
	beq :+
	rts 			; no.
:
	lda #0			; wipe fodder in MAP
	ldy #0
	sta (bufaddr),y 	; rewrite block type.

	lda dispx		; x=x/8
	pha
	lsr a
	lsr a
	lsr a
	sta dispx

	lda dispy		; y=y/8
	pha
	lsr a
	lsr a
	lsr a
	sta dispy

	lda #0 			; block to write.
	jsr pattr 		; write block.

	pla
	sta dispy
	pla
	sta dispx
	rts
.endif

;----------------------------------------------------
; Scrolly text and puzzle variables.
;----------------------------------------------------

.if sflag
txtbit:	.byte 128		; bit to write.
txtwid:	.byte 16		; width of ticker message.
txtpos:	.word msgdat
txtini:	.word msgdat
txtscr:	.word ScreenAddr
.endif

;----------------------------------------------------
; Specialist routines.
; Process shrapnel.
;----------------------------------------------------
proshr:
.if pflag
	lda #<SHRAPN		; table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; shrapnel pieces to process.
	sta shrctr
prosh0:
	ldy #0
	lda (z80_ix),y		; on/off marker.
	asl a
proshx:
	bcs :+
	jsr prosh1 		; on, so process it.
:
	clc
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec shrctr		; round again.
	bne prosh0
.endif
.if sflag
	jsr scrly
.endif
	rts

.if pflag
;----------------------------------------------------
; Proces shrapnel piece
;----------------------------------------------------

prosh1:
	jsr plot 		; delete the pixel.

	lda #<shrptr		; shrapnel routine pointers.
	sta z80_l
	lda #>shrptr
	sta z80_h

	ldy #0
	lda (z80_ix),y		; restore shrapnel type.
	jsr prosh2 		; run the routine.
	jsr chkxy		; check x and y are good before we redisplay.

	lda #<SHRSIZ 		; distance to next.
	sta z80_e
	lda #>SHRSIZ
	sta z80_d
	rts

;----------------------------------------------------
; Run the routine
;----------------------------------------------------

prosh2:
	asl a 			; 2 bytes per address.
	tay
	lda shrptr,y
	sta z80_l
	lda shrptr+1,y 		; fetch high byte from table.
	sta z80_h
	jmp (z80_hl) 		; jump to routine.

;----------------------------------------------------
; Paricle routine table
;----------------------------------------------------

shrptr:	.word laser		; laser.
	.word trail		; vapour trail.
	.word shrap		; shrapnel from explosion.
	.word dotl		; horizontal starfield left.
	.word dotr		; horizontal starfield right.
	.word dotu		; vertical starfield up.
	.word dotd		; vertical starfield down.
	.word ptcusr		; user particle.

;----------------------------------------------------
; Explosion shrapnel.
;----------------------------------------------------

shrap:
	ldy #1
	lda (z80_ix),y 		; get the angle.
	clc
	adc #<shrsin		; shrapnel sine table.
	sta z80_l
	lda #>shrsin
	adc #0
	sta z80_h

	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_e
	inc z80_l 		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y		; fetch value from table.
	sta z80_d
	inc z80_l		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_c
	inc z80_l 		; next byte of table.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 		; fetch value from table.
	sta z80_b

	ldy #2
	lda (z80_ix),y 		; x coordinate in hl.
	clc
	adc z80_e		; add sine lb
	sta (z80_ix),y		; store new coordinate lb.
	ldy #3
	lda (z80_ix),y
	adc z80_d		; add sine hb
	sta (z80_ix),y		; store new coordinate hb.

	ldy #4
	lda (z80_ix),y	 	; y coordinate in hl.
	clc
	adc z80_c		; add cosine lb
	sta (z80_ix),y		; store new coordinate lb.
	ldy #5
	lda (z80_ix),y
	adc z80_b		; add cosine lb
	sta (z80_ix),y		; store new coordinate hb.

	rts

;----------------------------------------------------
; Move dots
;----------------------------------------------------

dotl:
	ldy #5
	lda (z80_ix),y
	sec
	sbc #1		 	; move left.
	sta (z80_ix),y
	rts
dotr:
	ldy #5
	lda (z80_ix),y
	clc
	adc #1		 	; move left.
	sta (z80_ix),y
	rts
dotu:
	ldy #3
	lda (z80_ix),y
	sec
	sbc #1		 	; move up.
	sta (z80_ix),y
	rts
dotd:
	ldy #3
	lda (z80_ix),y
	clc
	adc #1			; move down.
	sta (z80_ix),y
	rts

;----------------------------------------------------
; Check if coordinates are ok before redrawing at new position.
;
; left:   X>L		X=L	Ok
; right:  R+15>X	X=R	Ok
; top:    Y>T		Y=T	Ok
; bottom: B+15>Y	Y=B	Ok
;----------------------------------------------------

chkxy:

; top:    Y>T		Y=T	Ok

	ldy #3
	lda (z80_ix),y	 	; fetch shrapnel coordinate.
	cmp wntopx		; window top.
	bcs :+			; compare with top window limit.
	jmp kilshr		; out of window, kill shrapnel.
:
; left:   X>L		X=L	Ok

	ldy #5
	lda (z80_ix),y	 	; fetch shrapnel coordinate.
	cmp wnlftx		; left edge.
	bcs :+			; compare with left window limit.
	jmp kilshr		; out of window, kill shrapnel.
:
; bottom: B+15>Y	Y=B	Ok

	lda wnbotx		; point to bottom.
	clc
	adc #15
	ldy #3
	cmp (z80_ix),y	 	; fetch shrapnel coordinate.
	bcs :+			; compare with shrapnel x coordinate.
	jmp kilshr		; off screen, kill shrapnel..
:
; right:  R+15>X	X=R	Ok

	lda wnrgtx		; point to right edge.
	clc
	adc #15
	ldy #5
	cmp (z80_ix),y	 	; fetch shrapnel coordinate.
	bcs :+			; compare with window limit.
	jmp kilshr		; off screen, kill shrapnel.
:

;----------------------------------------------------
; Drop through.
; Display shrapnel.
;----------------------------------------------------

plot:
	ldy #3
	lda (z80_ix),y		; y integer.
	sta dispy	 	; workspace coordinates.
	ldy #5
	lda (z80_ix),y	 	; x integer.
	sta dispx 		; workspace coordinates.

	ldy #0
	lda (z80_ix),y 		; type.
	bne :+			; is it a laser?
	jmp plot1 		; yes, draw laser instead.
:
plot0:
	lda dispx		; which pixel within byte do we
	and #7			; want to set first?
	tay
	lda dots,y 		; table of small pixel positions.
	sta z80_e 		; get value.

	jsr scadd 		; screen address.
	ldy #0
	lda (scraddr),y		; see what's already there.
	eor z80_e
	sta (scraddr),y 	; put back on screen.
	rts

plot1:
	jsr scadd 		; screen address.
	ldy #0
	lda (scraddr),y 	; fetch byte there.
	eor #255 		; toggle all bits.
	sta (scraddr),y 	; new byte.
	rts

;----------------------------------------------------
; Switch off shrapnel
;----------------------------------------------------

kilshr:
	lda #128
	ldy #0
	sta (z80_ix),y	; switch off shrapnel.
	rts

;----------------------------------------------------
; Sine/cosine table
;----------------------------------------------------

shrsin:	.word 0,1024,391,946,724,724,946,391
	.word 1024,0,946,65144,724,64811,391,64589
	.word 0,64512,65144,64589,64811,64811,64589,65144
	.word 64512,0,64589,391,64811,724,65144,946

;----------------------------------------------------
; Create trail
;----------------------------------------------------

trail:
	ldy #1
	lda (z80_ix),y 	; time remaining.
	sec
	sbc #1
	sta (z80_ix),y
	bne :+
	jmp trailk		; time to switch it off.
:
	jsr qrand		; get a random number.
	lsr a 			; x or y axis?
	bcc :+
	jmp trailv		; use y.
:
; Trail horizontal

	lsr a 			; which direction?
	bcc :+
	jmp traill		; go left.
:
; Trail right

	ldy #5
	lda (z80_ix),y
	clc
	adc #1	 		; go right.
	sta (z80_ix),y
	rts

; Trail left

traill:
	ldy #5
	lda (z80_ix),y
	sec
	sbc #1 			; go left.
	sta (z80_ix),y
	rts

; Trail vertical

trailv:
	lsr a		 	; which direction?
	bcc :+
	jmp trailu		; go up.
:
; Trail down

	ldy #3
	lda (z80_ix),y
	clc
	adc #1 			; go down.
	sta (z80_ix),y
	rts

; Trail up

trailu:
	ldy #3
	lda (z80_ix),y
	sec
	sbc #1 			; go up.
	sta (z80_ix),y
	rts

; Kill trail

trailk:
	lda #200		; set off-screen to kill vapour trail.
	ldy #3
	sta (z80_ix),y
	rts

;----------------------------------------------------
; Create laser beam
;----------------------------------------------------

laser:
	ldy #1
	lda (z80_ix),y 		; direction.
	ror a 			; left or right?
	bcs :+
	jmp laserl		; move left.
:
; Laser right

	lda #8			; distance to travel.
	sta z80_b
	jmp laserm		; move laser.

; Laser left

laserl:
	lda #248		; distance to travel.
	sta z80_b
laserm:
	ldy #5
	lda (z80_ix),y		; x position.
	clc
	adc z80_b		; add distance.
	sta (z80_ix),y		; set new x coordinate.

; Test new block.

	sta dispx 		; set x for block collision detection purposes.
	ldy #3
	lda (z80_ix),y 		; get y.
	sta dispy		; set coordinate for collision test.
	jsr tstbl 		; get block type there.
	cmp #WALL		; is it solid?
	bne :+
	jmp trailk		; yes, it cannot pass.
:
.if pflag .or dflag
        cmp #FODDER             ; is it fodder?
        bne :+
        jsr fdchk               ; remove fodder block.
        jmp trailk              ; destroy laser.
:
.endif
	rts

;----------------------------------------------------
; Dots mask
;----------------------------------------------------

dots:	.byte 128,64,32,16,8,4,2,1


;----------------------------------------------------
; Plot, preserving de.
;----------------------------------------------------

plotde:
	lda z80_d 		; put de on stack.
	pha
	lda z80_e
	pha

	jsr plot 		; plot pixel.

	pla			; restore de from stack.
	sta z80_e
	pla
	sta z80_d

	rts

;----------------------------------------------------
; Shoot a laser.
;----------------------------------------------------

shoot:
	sta z80_c		; store direction in c register.
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
shoot1:
	adc #(SPR_HGT/2-1)	; down 7 pixels.
	sta z80_l 		; puty y coordinate in l.

	ldy #9
	lda (z80_ix),y 		; x coordinate in h.
	sta z80_h

	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	jsr fpslot 		; find particle slot.
	bcs :+
	jmp vapou2		; failed, restore ix.
:
	lda #0
	ldy #0
	sta (z80_ix),y 		; set up a laser.

	lda z80_c
	ldy #1
	sta (z80_ix),y 		; set the direction.

	lda z80_l
	ldy #3
	sta (z80_ix),y		; set y coordinate.

	ror z80_c		; check direction we want.
	bcc :+
	jmp shootr		; shoot right.
:
	lda z80_h		; X position.
shoot0:
	and #248		; align on character boundary.
	ldy #5
	sta (z80_ix),y		; set x coordinate.
	jmp vapou0 		; draw first image.
shootr:
	lda z80_h		; x position.
	clc
	adc #15			; look right.
	jmp shoot0		; align and continue.

;----------------------------------------------------
; Create a bit of vapour trail.
;----------------------------------------------------

vapour:
	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
vapou3:
	adc #(SPR_HGT/2-1)	; mid-point of sprite.
	sta z80_l

	ldy #9
	lda (z80_ix),y 		; x coordinate.
	adc #7
	sta z80_h

	jsr fpslot 		; find particle slot.
	bcc :+
	jmp vapou1		; no, we can use it.
:
vapou2:
	pla
	sta z80_x
	pla
	sta z80_i
	rts
vapou1:
	lda z80_l
	ldy #3
	sta (z80_ix),y		; set up y.

	lda z80_h
	ldy #5
	sta (z80_ix),y 		; set up x coordinate.

	jsr qrand		; get quick random number.
	and #15			; random time.
	clc
	adc #15			; minimum time on screen.
	ldy #1
	sta (z80_ix),y		; set time on screen.

	lda #1
	ldy #0
	sta (z80_ix),y		; define particle as vapour trail.
vapou0:
	jsr chkxy		; plot first position.
	jmp vapou2

;----------------------------------------------------
; Create a user particle.
;----------------------------------------------------

ptusr:
	sta z80_f		; store timer.

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	clc
	adc #7			; mid-point of sprite.
	sta z80_l

	ldy #9
	lda (z80_ix),y 		; x coordinate.
	clc
	adc #7			; mid-point of sprite.
	sta z80_h

	jsr fpslot 		; find particle slot.
	bcs ptusr1
	rts 			; out of slots, can't generate anything.
ptusr1:
	lda z80_l
	ldy #3
	sta (z80_ix),y 		; set up y.

	lda z80_h
	ldy #5
	sta (z80_ix),y		; set up x coordinate.

	lda z80_f 		; restore timer.
	ldy #1
	sta (z80_ix),y		; set time on screen.

	lda #7
	ldy #0
	sta (z80_ix),y		; define particle as user particle.

	jmp chkxy		; plot first position.

;----------------------------------------------------
; Create a vertical or horizontal star.
;----------------------------------------------------

star:
	lda z80_i		; store pointer to sprite.
	pha
	lda z80_x
	pha

	jsr fpslot 		; find particle slot.
	bcs star7		; found one we can use.
star0:
	pla 			; restore sprite pointer.
	sta z80_x
	pla
	sta z80_i
	rts 			; out of slots, can't generate anything.
star7:
	lda z80_c		; direction.
	and #3 			; is it left?
	bne :+
	jmp star1 		; yes, it's left.
:
	cmp #1 			; is it right?
	bne :+
	jmp star2 		; yes, it's right.
:
	cmp #2 			; is it up?
	bne :+
	jmp star3 		; yes, it's up.
:
	ldy wntopx 		; get edge of screen.
	iny			; down one pixel.
	tya
star8:
	ldy #3
	sta (z80_ix),y 		; set y coord.
	jsr qrand 		; get quick random number.
star9:
	ldy #5
	sta (z80_ix),y		; set x position.

	lda z80_c		; direction.
	and #3			; zero to three.
	clc
	adc #3			; 3 to 6 for starfield.
	ldy #0
	sta (z80_ix),y		; define particle as star.
	jsr chkxy		; plot first position.
	jmp star0
star1:
	jsr qrand		; get quick random number.
	ldy #3
	sta (z80_ix),y 		; set y coord.

	lda wnrgtx 		; get edge of screen.
	clc
	adc #15			; add width of sprite minus 1.
	jmp star9
star2:
	jsr qrand 		; get quick random number.
	ldy #3
	sta (z80_ix),y		; set y coord.

	lda wnlftx		; get edge of screen.
	jmp star9
star3:
	lda wnbotx 		; get edge of screen.
	clc
	adc #15 		; height of sprite minus one pixel.
	jmp star8

;----------------------------------------------------
; Find particle slot for lasers or vapour trail.
; can't use alternate accumulator.
;----------------------------------------------------

fpslot:
	lda #<SHRAPN 		; shrapnel table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; number of pieces in table.
	sta z80_b
fpslt0:
	ldy #0
	lda (z80_ix),y		; get type.
	asl a  			; is this slot in use?
	bcc :+
	rts			; no, we can use it.
:
	clc			; point to more shrapnel.
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec z80_b		; repeat for all shrapnel.
	bne fpslt0

	clc
	rts 			; out of slots, can't generate anything.

;----------------------------------------------------
; Create an explosion at sprite position.
;----------------------------------------------------

explod:
	sta z80_c 		; particles to create.

	lda z80_i 		; store pointer to sprite.
	pha
	lda z80_x
	pha

	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta z80_l
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta z80_h

	lda #<SHRAPN		; shrapnel table.
	sta z80_x
	lda #>SHRAPN
	sta z80_i

	lda #NUMSHR		; number of pieces in table.
	sta explcnt
expld0:
	ldy #0
	lda (z80_ix),y		; get type.
	asl a 			; is this slot in use?
	bcs expld1		; no, we can use it.
expld2:
	clc
	lda z80_x
	adc #SHRSIZ
	sta z80_x
	bcc :+
	inc z80_i
:
	dec explcnt		; repeat for all shrapnel.
	bne expld0
expld3:
	pla			; restore sprite pointer.
	sta z80_x
	pla
	sta z80_i
	rts 			; out of slots, can't generate any more.

expld1:
	lda z80_c		; shrapnel counter.
	and #15			; 0 to 15.
	clc			; add to x.
	adc z80_l
	ldy #3
	sta (z80_ix),y		; y coord.

	lda seed3 		; crap random number.
	and #15			; 0 to 15.
	clc 			; add to y.
	adc z80_h
	ldy #5
	sta (z80_ix),y		; x coord.

	lda #2
	ldy #0
	sta (z80_ix),y		; switch it on.

	jsr chkxy		; plot first position.
	jsr qrand		; quick random angle.
	and #60 		; keep within range.
	ldy #1
	sta (z80_ix),y		; angle.

	dec z80_c		; one less piece of shrapnel to generate.
	bne expld2 		; back to main explosion loop.
	jmp expld3 		; restore sprite pointer and exit.

;----------------------------------------------------
; Quick random
;----------------------------------------------------

qrand:
	jsr random		; r register.
	eor seed3		; combine with seed.
	sta seed3 		; new seed.
	rts

;----------------------------------------------------
; Display all shrapnel.
;----------------------------------------------------

dshrp:
	lda #<plotde		; display routine.
	sta proshx+1
	lda #>plotde
	sta proshx+2
	jsr proshr		; process shrapnel.

	lda #<prosh1		; processing routine.
	sta proshx+1
	lda #>prosh1
	sta proshx+2
	rts

;------------------------------------------------------
; Particle engine.
;
; Init particle data for 55 particles in SHRAPN table.
; Every particle has 6 bytes.
;
; global:	-
; local:	x,y,hl
; calls:	-
;------------------------------------------------------

inishr:
	lda #<SHRAPN 		; table.
	sta z80_l
	lda #>SHRAPN
	sta z80_h

	ldy #0
	ldx #NUMSHR		; shrapnel pieces to process.
inish0:
	lda #255 		; kill the shrapnel.
	sta (z80_hl),y

	clc 			; point there.
	lda z80_l
	adc #SHRSIZ		; distance to next.
	sta z80_l
	bcc :+
	inc z80_h
:
	dex
	bne inish0 		; round again.
	rts

;------------------------------------------------------
; Check for collision between laser and sprite.
;------------------------------------------------------

lcol:
	lda #<SHRAPN		; shrapnel table.
	sta z80_l
	lda #>SHRAPN
	sta z80_h

	lda #NUMSHR		; number of pieces in table.
	sta z80_b
lcol0:
	ldy #0
	lda (z80_hl),y 		; get type.
	beq lcol1		; yes, check collision.
lcol3:
	clc			; point to more shrapnel.
	lda z80_l
	adc #SHRSIZ
	sta z80_l
	bcc :+
	inc z80_h
:
	dec z80_b		; repeat for all shrapnel.
	bne lcol0
	clc
	rts 			; no collision, carry not set.
lcol1:
	ldy #3
	lda (z80_hl),y		; get y.
	sec
	ldy #8
	sbc (z80_ix),y		; subtract sprite y.
lcolh:
	cmp #SPR_HGT 		; within range?
	bcc :+
	jmp lcol2		; no, missed.
:
	ldy #5
	lda (z80_hl),y 		; get x.
	sec
	ldy #9
	sbc (z80_ix),y 		; subtract sprite y.
	cmp #16			; within range?
	bcs :+
	jmp lcol4 		; yes, collision occurred.
:
lcol2:
	jmp lcol3
lcol4:
	sec
	rts 			; return with carry set for collision.
.endif

;------------------------------------------------------
; Main game engine code starts here.
; After initialisation, mloop is the main loop
;------------------------------------------------------

game:

; Set up screen address table.

setsat:
	lda #<CharAddress	; start of screen.
	sta scraddr
	lda #>CharAddress
	sta scraddr+1

	ldy #0			; vertical lines on screen.
setsa0:
	lda scraddr
	sta SCADTB_lb,y		; write low byte.
	lda scraddr+1
	sta SCADTB_hb,y		; write high byte.
	jsr nline		; next line down.
	iny			; next position in table.
	bne setsa0

; Set up colour address table

setsof:
	lda #<ColorAttr		; start of colour attributes
	sta bufaddr
	lda #>ColorAttr
	sta bufaddr+1

	ldy #0			; vertical lines on screen.
setof0:
	lda bufaddr
	sta COLATTTB_lb,y	; write low byte.
	lda bufaddr+1
	sta COLATTTB_hb,y	; write high byte.
	clc			; next line down.
	lda bufaddr
	adc #ScrWidth
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	iny			; next position in table.
	cpy #ScrHeight/2
	bne setof0

; Init graphics mode

	jsr init

; Init joysticks
	jsr joyinit		; VIC20 joystick

rpblc2:
.if pflag
	jsr inishr 		; initialise particle engine.
.endif
evintr:
	jsr evnt12 		; call intro/menu event.

	lda #WALL 		; write default property.
	ldx #0
clrmap:
	sta MAP,x 		; block properties.
	sta MAP+256,x
	inx
	bne clrmap

.if oflag
	jsr iniob 		; initialise objects.
.endif
	lda #0			; put zero in accumulator.
	sta gamwon		; reset game won flag.

	jsr inisc 		; init the score.
mapst:
	lda stmap 		; start position on map.
	sta roomtb		; set up position in table, if there is one.

inipbl:
.if aflag
	lda #<eop		; reset blockpointer
	sta pbptr
	lda #>eop
	sta pbptr+1
.endif
	jsr initsc 		; set up first screen.

	lda #<ssprit 		; default to spare sprite in table.
	sta z80_x
	lda #>ssprit
	sta z80_i
evini:
	jsr evnt13 		; initialisation.

; Two restarts.
; First restart - clear all sprites and initialise everything.

rstrt:
	jsr rsevt 		; restart events.
	jsr xspr 		; clear sprite table.
	jsr sprlst 		; fetch pointer to screen sprites.
	jsr ispr 		; initialise sprite table.
	jmp rstrt0

; Second restart - clear all but player, and don't initialise him.

rstrtn:
	jsr rsevt		; restart events.
	jsr nspr 		; clear all non-player sprites.
	jsr sprlst 		; fetch pointer to screen sprites.
	jsr kspr 		; initialise sprite table, no more players.

; Set up the player and/or enemy sprites.

rstrt0:
	lda #0 			; zero in accumulator.
	sta nexlev 		; reset next level flag.
	sta restfl 		; reset restart flag.
	sta deadf 		; reset dead flag.

	jsr droom 		; show screen layout.
rpblc0:
.if pflag
	jsr inishr 		; initialise particle engine.
.endif
.if aflag
	jsr rbloc		; draw blocks for this screen
.endif
.if oflag
	jsr shwob		; draw objects.
.endif

	lda #<sprtab 		; address of sprite table, even sprites.
	sta z80_x
	lda #>sprtab
	sta z80_i
	jsr dspr 		; display sprites.
	lda #<(sprtab+TABSIZ) 	; address of first odd sprite.
	sta z80_x
	lda #>(sprtab+TABSIZ)
	sta z80_i
	jsr dspr 		; display sprites.
mloop:
	jsr vsync 		; synchronise with display.

	lda #<sprtab 		; address of sprite table, even sprites.
	sta z80_x
	lda #>sprtab
	sta z80_i
	jsr dspr 		; display even sprites.

;	jsr plsnd 		; play sounds.
	jsr vsync 		; synchronise with display.
	jsr proshr

	lda #<(sprtab+TABSIZ) 	; address of first odd sprite.
	sta z80_x
	lda #>(sprtab+TABSIZ)
	sta z80_i
	jsr dspr 		; display odd sprites.

	lda #<(ssprit) 		; point to spare sprite for spawning purposes.
	sta z80_x
	lda #>(ssprit)
	sta z80_i
evlp1:
	jsr evnt10 		; called once per main loop.
	jsr pspr 		; process sprites.

; Main loop events.

	lda #<ssprit 		; point to spare sprite for spawning purposes.
	sta z80_x
	lda #>ssprit
	sta z80_i
evlp2:
	jsr evnt11 		; called once per main loop.
bsortx:
	jsr bsort 		; sort sprites.

	lda nexlev		; finished level flag.
	bne newlev		; is set, go to next level.
	lda gamwon		; finished game flag.
	bne evwon		; is set, finish the game.
	lda restfl 		; finished level flag.
	cmp #1			; has it been set?
	bne :+
	jmp rstrt		; yes, go to next level.
:
	cmp #2			; has it been set?
	bne :+
	jmp rstrtn		; yes, go to next level.
:
	lda deadf 		; dead flag.
	bne pdead		; yes, player dead.

; back to start of main loop.

	inc frmno
	inc clock
	jmp mloop		; switched to a jmp mloop during test mode.

;----------------------------------------------------------
; Read blocks from list and update screen accordingly.
;----------------------------------------------------------

.if aflag
rbloc:
	lda #<eop		; reset blockpointer
	sta pbbuf
	lda #>eop
	sta pbbuf+1

rbloc2:
	lda pbbuf			; check for last block
	cmp pbptr
	bne rbloc1
	lda pbbuf+1
	cmp pbptr+1
	bne rbloc1
	rts
rbloc1:
	ldy #0
	lda (pbbuf),y		; check if block for this scno
	cmp scno
	bne rbloc0		; if not, skip
	iny
	lda (pbbuf),y		; get y
	sta dispy
	iny
	lda (pbbuf),y		; get x
	sta dispx
	iny
	lda (pbbuf),y		; get blocknr
	jsr pattr2		; draw block
rbloc0:
	clc			; point to next block
	lda pbbuf
	adc #4
	sta pbbuf
	bcc rbloc2
	inc pbbuf+1
	jmp rbloc2
.endif

;----------------------------------------------------------
; New level
;----------------------------------------------------------

newlev:
	lda scno 			; current screen.
	clc
	adc #1				; next screen.
	cmp numsc 			; total number of screens.
	bcs evwon			; yes, game finished.
	sta scno			; set new level number.
	jmp rstrt			; restart, clearing all aliens.

evwon:
	jsr evnt18		 	; game completed.
	jmp tidyup			; tidy up and return to BASIC/calling routine.

;----------------------------------------------------------
; Player dead.
;----------------------------------------------------------

pdead:
	lda #0				; zeroise accumulator.
	sta deadf			; reset dead flag.
evdie:
	jsr evnt16 			; death subroutine.
	lda numlif			; number of lives.
	beq :+
	jmp rstrt 			; restart game.
:
evfail:
	jsr evnt17 			; failure event.
	jmp game			; restart game

;----------------------------------------------------------
; Tidy things up
;----------------------------------------------------------

tidyup:
	ldy #0				; digits to check.
tidyu2:
	lda score,y 			; get score digit.
	cmp hiscor 			; are we larger than high score digit?
	bcc tidyu0			; high score is bigger.
	bne tidyu1			; score is greater, record new high score.
	iny				; next digit of high score.
	cpy #6
	bne tidyu2			; repeat for all digits
tidyu0:
	lda #<score			; return pointing to score.
	sta z80_c
	lda #>score
	sta z80_b
	rts
tidyu1:
	ldy #5
tidyu3:
	lda score,y			; score.
	sta hiscor,y			; high score.
	dey
	bpl tidyu3 			; copy score to high score.
evnewh:
	jsr evnt19			; new high score event.
	jmp tidyu0			; tidy up.

;--------------------------------------------------
; Restart event.
;--------------------------------------------------

rsevt:
	lda #<ssprit 			; default to spare element in table.
	sta z80_x
	lda #>ssprit
	sta z80_i

evrs:
	jmp evnt14	 		; call restart event.

;------------------------------------------------------------------
; Copy number passed in a to string position bc, right-justified.
;
; Input:
;  A  = number
;  BC = string address
;
; Output:
;  BC = string with number
;-----------------------------------------------------------------

num2ch:
	sta z80_d		; Save number

	lda #0
	sta flag
numdg3:
	ldx #100		; hundreds column.
	stx z80_e
	jsr numdg		; show digit.
numdg2:
	ldx #10			; tens column.
	stx z80_e
	jsr numdg		; show digit.

	inc flag
	ldx #1			; units column.
	stx z80_e
numdg:
	lda #48			; clear digit.
	sta z80_a
numdg1:
	lda z80_d
	cmp z80_e
	bcc numdg0		; nothing to show.
	sec
	lda z80_d
	sbc z80_e		; subtract from column.
	sta z80_d
	inc z80_a		; increment digit.
	inc flag
	jmp numdg1		; repeat until column is zero.
numdg0:
	ldy #0
	lda z80_a
	sta (z80_bc),y		; write digit to buffer.
	lda flag
	beq :+
	inc z80_c		; next buffer position.
	bne :+
	inc z80_b
:
	rts
num2dd:
	sta z80_d		; Save number

	lda #1
	sta flag

	jmp numdg2
num2td:
	sta z80_d		; Save number

	lda #1
	sta flag
	jmp numdg3

;---------------------------------------------------------
; Reset score to "000000"
;---------------------------------------------------------

inisc:
	lda #'0'
	ldx #5			; digits to initialise.
inisc0:
	sta score,x 		; write zero digit.
	dex			; next column.
	bpl inisc0		; repeat for all digits.

	rts

;-----------------------------------------------------
; Multiply h by d and return in hl.
;
; Input:
;  H = first number
;  D = second number
;
; Output:
;  HL = result H x D
;-----------------------------------------------------

imul:
	lda z80_d		; HL = H * D
	sta z80_e
	lda z80_h
	sta z80_c		; make c first multiplier.
imul0:
	lda #0			; zeroise total.
	sta z80_l
	sta z80_h

	lda z80_h
	sta z80_d		; zeroise high byte.

	lda #8			; repeat 8 times.
	sta z80_b
imul1:
	lsr z80_c		; rotate rightmost bit into carry.
	bcc imul2		; wasn't set.
	clc			; bit was set, so add de.
	lda z80_l
	adc z80_e
	sta z80_l
	lda z80_h
	adc z80_d
	sta z80_h
	clc 			; reset carry.
imul2:
	rol z80_e 		; shift de 1 bit left.
	rol z80_d
	dec z80_b
	bne imul1		; repeat 8 times.

	rts

;-----------------------------------------------
; Divide d by e and return in d, remainder in a.
;
; Input:
;  D = first number
;  E = second number
;
; Output:
;  D = result D/E
;  A = remainder
;-----------------------------------------------

idiv:
	lda #0
	ldy #8		 	; bits to shift.
	asl z80_d
idiv0:
	rol a 			; multiply d by 2.
	cmp z80_e 		; test if e is smaller.
	bcc idiv1		; e is greater, no division this time.
	sbc z80_e		; subtract it.
idiv1:
	rol z80_d		; rotate into d.
	dey
	bne idiv0		; repeat
	rts

;---------------------------------------------------
; Play AY sound effect
;---------------------------------------------------

plsnd:
	rts

;---------------------------------------------------
; Objects handling.
; 32 bytes for image
; 1 for colour
; 3 for room, y and x
; 3 for starting room, y and x.
; 254 = disabled.
; 255 = object in player"s pockets.
;---------------------------------------------------

;---------------------------------------------------
; Show items present.
;---------------------------------------------------

.if oflag
shwob:
	lda #<objdta			; objects table.
	sta z80_l
	lda #>objdta
	sta z80_h
	lda numob 			; number of objects in the game.
	sta sprcnt
shwob0:
	ldy #33 			; distance to room number.
	lda (z80_hl),y 			; same as an item?
	cmp scno 			; current location.
	bne :+
	jsr dobj 			; yes, display object.
:
	clc
	lda z80_l
	adc #39 			; distance to next item.
	sta z80_l
	lda z80_h
	adc #0
	sta z80_h	 		; point to it.
	dec sprcnt
	bne shwob0 			; repeat for others.
	rts

;---------------------------------------------------
; Display object.
; hl must point to object's start address.
;
; Input:
;  HL = object address
;---------------------------------------------------

dobj:
	ldy #32
	lda (z80_hl),y 			; point to y.
	sta objcol
	ldy #34
	lda (z80_hl),y 			; point to y.
	sta dispy
	iny
	lda (z80_hl),y 			; point to x.
	sta dispx
dobj1:
	jmp sprite 			; draw this sprite.

;--------------------------------------
; Remove an object.
;
; Input:
;  A = object number
;--------------------------------------

remob:
	cmp numob			; number of objects in game.
	bcc :+				; are we checking past the end?
	rts				; yes, can't get non-existent item.
:
	pha				; remember object.
	jsr getob			; pick it up if we haven't already got it.
	pla				; retrieve object number.
	jsr gotob			; get its address.
	lda #254
	ldy #33
	sta (z80_hl),y			; remove it.
	rts

;---------------------------------------------------
; Pick up object number held in the accumulator.
;
; Input:
;  A = object number
;---------------------------------------------------

getob:
	cmp numob 		; number of objects in game.
	bcc :+			; are we checking past the end?
	rts			; yes, can't get non-existent item.
:
	jsr gotob 		; check if we already have it.
	cmp #255
	bne :+
	rts			; we already do.
:
	ldy #33
	lda (z80_hl),y		; is it on this screen?
	cmp scno 		; current screen.
	bne getob0		; not on screen, so nothing to delete.

	lda #255
	sta (z80_hl),y		; pick it up.
	iny 			; point to y coord.
getob1:
	ldy #34
	lda (z80_hl),y		; y coord.
	sta dispy
	ldy #35
	lda (z80_hl),y 		; x coord.
	sta dispx
	jmp dobj1 		; delete object sprite.
getob0:
	lda #255
	sta (z80_hl),y 		; pick it up.
	rts
.endif

;-----------------------------------------------------------------
; Got object check.
; Call with object in accumulator, returns zero set if in pockets.
;
; Input:
;  A = object number
;-----------------------------------------------------------------
.if oflag .or mflag
gotob:
	cmp numob 		; number of objects in game.
	bcc :+ 			; are we checking past the end?
	jmp gotob0 		; yes, we can't have a non-existent object.
:
	jsr findob		; find the object.
gotob1:
	rts

gotob0:
	lda #254 		; missing.
	jmp gotob1

findob:
	pha			; save object number
	lda #<objdta 		; objects.
	sta z80_l
	lda #>objdta
	sta z80_h
	pla			; retreive object number
	beq fndob1 		; is it zero? yes, skip loop.
	tax 			; loop counter
fndob2:
	clc
	lda z80_l
	adc #39 		; size of each object.
	sta z80_l
	bcc :+
	inc z80_h
:
	dex 			; repeat until we find address.
	bne fndob2
fndob1:
	ldy #33			; distance to room it's in.
	lda (z80_hl),y		; fetch status.
	rts
.endif

;---------------------------------------------
; Drop object number at (dispx, dispy).
;
; Input:
;  A = object number
;---------------------------------------------

.if oflag
drpob:
	cmp numob 		; are we checking past the end?
	bcc :+
	rts			; yes, can't drop non-existent item.
:
	jsr gotob		; make sure object is in inventory.
	cmp scno		; already on this screen?
	bne :+
	rts			; yes, nothing to do.
:
	ldy #33
	lda scno
	sta (z80_hl),y		; bring onto screen.
	lda dispy		; sprite y coordinate.
	iny 
	sta (z80_hl),y		; point to object y.
	lda dispx 		; sprite x coordinate.
	iny
	sta (z80_hl),y 		; point to object x
	jmp dobj		; draw the object sprite.

;-----------------------------------------------
; Seek objects at sprite position.
;
; Output:
;  A = object number, if not found A=255
;-----------------------------------------------

skobj:
	lda #<objdta 		; pointer to objects.
	sta z80_l
	lda #>objdta
	sta z80_h

	lda numob 		; number of objects in game.
	sta z80_b 		; set up the loop counter.
skobj0:
	lda scno		; current room number.
	ldy #33
	cmp (z80_hl),y		; is object in here?
	bne :+
	jsr skobj1		; yes, check coordinates.
:
	clc			; point to next object in table.
	lda z80_l
	adc #39			; size of each object.
	sta z80_l
	bcc :+
	inc z80_h
:
	dec z80_b
	bne skobj0		; repeat for all objects.

	lda #255		; end of list and nothing found, return 255.
	rts

skobj1:
	ldy #34			; point to y coordinate.
	lda (z80_hl),y		; point to y coordinate.
	sec
	ldy #var_newY
	sbc (z80_ix),y 		; subtract sprite y.
	clc
	adc #15			; add sprite height minus one.
	cmp #31			; within range?
	bcs skobj2		; no, ignore object.

	ldy #35			; point to x coordinate now.
	lda (z80_hl),y 		; get coordinate.
	sec
	ldy #var_newX
	sbc (z80_ix),y 		; subtract the sprite x.
	clc			; add sprite width minus one.
	adc #15
	cmp #31			; within range?
	bcs skobj2		; no, ignore object.

	pla			; remove return address from stack.
	pla

	lda numob 		; objects in game.
	sec
	sbc z80_b		; subtract loop counter.
skobj2:
	rts			; accumulator now points to object.
.endif

;---------------------------------------------------------------------
; Spawn a new sprite.
;---------------------------------------------------------------------

spawn:
	lda #<sprtab		; sprite table.
	sta z80_l
	lda #>sprtab
	sta z80_h
numsp1:
	lda #NUMSPR		; number of sprites.
	sta spcnt
spaw0:
	ldy #var_Type
	lda (z80_hl),y		; get sprite type.
	cmp #255		; is it an unused slot?
	beq spaw1 		; yes, we can use this one.

	clc 			; point to next sprite in table.
	lda z80_l
	adc #TABSIZ		; size of each entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	dec spcnt		; one less iteration.
	bne spaw0		; keep going until we find a slot.

; Didn't find one but drop through and set up a dummy sprite instead.

spaw1:
	lda z80_i		; address of original sprite.
	pha
	lda z80_x
	pha

	lda z80_l		; store spawned sprite address.
	sta spptr
	lda z80_h
	sta spptr+1

	lda z80_c
	ldy #var_Type
	sta (z80_hl),y 		; set the type.
	ldy #var_newType
	sta (z80_hl),y		; copy

	lda z80_b
	ldy #var_Image
	sta (z80_hl),y		; set the image.
	ldy #var_newImage
	sta (z80_hl),y		; copy

	lda #0 				; frame zero.
	ldy #var_Frame
	sta (z80_hl),y		; set frame.
	ldy #var_newFrame
	sta (z80_hl),y		; copy

	ldy #8
	lda (z80_ix),y 		; x coordinate.
	ldy #3
	sta (z80_hl),y		; set sprite coordinate.
	ldy #8
	sta (z80_hl),y		; copy

	ldy #9
	lda (z80_ix),y 		; y coordinate.
	ldy #4
	sta (z80_hl),y		; set sprite coordinate.
	ldy #9
	sta (z80_hl),y		; copy

	ldy #10				; direction of original.
	lda (z80_ix),y
	sta (z80_hl),y		; direction

	lda #0
	ldy #var_jumpLo
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
	iny
	sta (z80_hl),y		; reset parameter.
	iny
	lda fontcol
	sta (z80_hl),y		; reset parameter.
rtssp:
	lda spptr			; address of new sprite.
	sta z80_x
	lda spptr+1
	sta z80_i
evis1:
	jsr evnt09 			; call sprite initialisation event.

	lda spptr 			; address of new sprite.
	sta z80_x
	lda spptr+1
	sta z80_i

	; _BEEB clipping code copied from CPC Engine - MISSING?!
	ldy #var_Y
	lda (z80_hl), y		; old x coord
	cmp #SpriteMaxY     ; beyond maximum?
	bcs :+				; yes, don't draw it.

	jsr sspria 			; display the new sprite.
:
	pla					; address of original sprite.
	sta z80_x
	pla
	sta z80_i

	rts

score:	.byte "000000"		; player"s score.
hiscor:	.byte "000000"		; high score.
bonus:	.byte "000000"		; bonus.
grbase:	.word ScreenAddr	; graphics base address.

;----------------------------------------------------
; Check y-pos
;----------------------------------------------------

checkx:
	lda dispy		; y position.
	cmp #22			; off screen?
	bcs :+
	rts			; no, it's okay.
:
	pla			; remove return address from stack.
	sta z80_l
	pla
	sta z80_h
	rts

;-----------------------------------------------
; Displays the current high score.
;-----------------------------------------------

dhisc:
	lda #<hiscor 		; high score text.
	sta dscor3+1
	sta bscor3+1
	lda #>hiscor
	sta dscor3+2
	sta bscor3+2
	jmp dscor1		; check in printable range then show 6 digits.

;------------------------------------------------------
; Displays the current score.
;------------------------------------------------------

dscor:
	lda #<score		; score text.
	sta dscor3+1
	sta bscor3+1
	lda #>score
	sta dscor3+2
	sta bscor3+2
dscor1:
	sty tmpscr
	jsr preprt		; set up font and print position.
	jsr checkx		; make sure we're in a printable range.

;	lda #6			; digits to display.
;	sta z80_b
	lda prtmod		; get print mode.
	beq :+			; standard size text?
	jmp bscor0		; no, show double-height.
:
dscor0:
;	ldy #0
;	lda (z80_hl),y 		; fetch character.
	ldy tmpscr
dscor3:
	lda score,y
	jsr pchar 		; display character.
	inc dispx		; move along x coordinate

;	inc z80_l		; next score column.
;	bne :+
;	inc z80_h
;:
	inc tmpscr
	dec z80_b
	bne dscor0 		; repeat for all digits.
dscor2:
	lda dispx 		; set up display coordinates.
	sta charx
	lda dispy
	sta chary
	rts

tmpscr:	.byte 0
;------------------------------------------------------
; Displays the current score in double-height characters.
;
; Input:
;  B  = digit number
;  HL = score string
;------------------------------------------------------

bscor0:
;	ldy #0

;	lda (z80_hl),y 		; fetch character.
	ldy tmp
bscor3:
	lda score,y
	jsr bchar 		; display big char.

;	inc z80_l 		; next score column.
;	bne :+
;	inc z80_h
;:
	inc tmp
	dec z80_b
	beq :+
	jmp bscor0 		; repeat for all digits.
:
	jmp dscor2 		; tidy up line and column variables.

;-----------------------------------------------------
; Adds number in the hl pair to the score.
;-----------------------------------------------------

addsc:
	lda #<(score+1) 	; ten thousands column.
	sta z80_e
	lda #>(score+1)
	sta z80_d
	lda #<10000		; amount to add each time.
	sta z80_c
	lda #>10000
	sta z80_b
	jsr incsc		; add to score.

	inc z80_e		; thousands column.
	bne :+
	inc z80_d
:
	lda #<1000		; amount to add each time.
	sta z80_c
	lda #>1000
	sta z80_b
	jsr incsc 		; add to score.

	inc z80_e		; hundreds column.
	bne :+
	inc z80_d
:
	lda #<100		; amount to add each time.
	sta z80_c
	lda #>100
	sta z80_b
	jsr incsc		; add to score.

	inc z80_e 		; tens column.
	bne :+
	inc z80_d
:
	lda #<10		; amount to add each time.
	sta z80_c
	lda #>10
	sta z80_b
	jsr incsc 		; add to score.

	inc z80_e		; units column.
	bne :+
	inc z80_d
:
	lda #<1			; units.
	sta z80_c
	lda #>1
	sta z80_b
incsc:
	lda z80_h		; store amount to add.
	pha
	lda z80_l
	pha

	sec			; subtract from amount to add.
	lda z80_l
	sbc z80_c
	sta z80_l
	lda z80_h
	sbc z80_b
	sta z80_h
	bcc incsc0		; too much, restore value.

	pla			; delete the previous amount from the stack.
	pla

	lda z80_d 		; store column position.
	pha
	lda z80_e
	pha
	jsr incsc2		; do the increment.

	pla			; restore column.
	sta z80_e
	pla
	sta z80_d
	jmp incsc		; repeat until all added.

incsc0:
	pla			; restore previous value.
	sta z80_l
	pla
	sta z80_h
	rts
incsc2:
	ldy #0
	lda (z80_de),y 		; get amount.
	clc
	adc #1			; add one to column.
	sta (z80_de),y		; write new column total.
	cmp #'9'+1		; gone beyond range of digits?
	bcs :+
	rts			; no, carry on.
:
	lda #'0'		; make it zero.
	sta (z80_de),y		; write new column total.
	dec z80_e		; back one column.
	bne :+
	dec z80_d
:
	jmp incsc2

;------------------------------------
; Add bonus to score and reset bonus
;------------------------------------

addbo:
	ldx #5			; last digit.
	clc
addbo0:
	lda score,x		; get score.
	adc bonus,x		; add bonus.
	sec			; 0 to 18.
	sbc #48
	pha
	lda #'0'
	sta bonus,x
	pla
	cmp #58
	bcc addbo1
	sec
	sbc #10
addbo1:
	sta score,x		; zeroise bonus.
	dex			; next digit.
	bpl addbo0		; repeat for all 6 digits.
	rts

;------------------------------------
; Swap score and bonus.
;------------------------------------

swpsb:
	ldx #5			; digits to add.
swpsb0:
	lda score,x 		; get score digits.
	pha			; save digit
	lda bonus,x 		; get bonus digits.
	sta score,x		; switch score-bonus
	pla
	sta bonus,x
	dex 			; repeat for all 6 digits.
	bpl swpsb0
	rts

;----------------------------------------------------
; Get print address.
;----------------------------------------------------

gprad:
	tya
	pha

	lda dispx 		; x coordinate.
	sta scraddr
	lda #0
	sta scraddr+1
	asl scraddr  		; multiply char by 16
	rol scraddr+1
	asl scraddr
	rol scraddr+1
	asl scraddr
	rol scraddr+1
	asl scraddr
	rol scraddr+1

	lda dispy		; y coordinate.
	asl a
	asl a
	asl a			; multiply char by 8
	tay

	clc
	lda scraddr
	adc SCADTB_lb,y
	sta scraddr
	lda scraddr+1
	adc SCADTB_hb,y
	sta scraddr+1

	lda prtmod		; Double line if bigtxt
	beq noskip
	lda dispx
	and #$e0
	beq noskip
	inc scraddr+1
noskip:

	pla
	tay
	rts
;--------------------------------------------------------------
; Get property buffer address of char at (dispx, dispy) in hl.
; 
; Output:
;  buffaddr = bufaddr = MAP + dispy*22 + dispx
;--------------------------------------------------------------

pradd:
	lda dispy 		; y coordinate.
	sta bufaddr
	lda #0
	sta bufaddr+1
	asl bufaddr  		; multiply char by 32
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	clc			; add address of MAP graphics.
	lda bufaddr
	adc dispx
	adc #<MAP
	sta bufaddr
	lda bufaddr+1
	adc #>MAP
	sta bufaddr+1
	rts

;--------------------------------------------------------------
; Get attribute address at (dispx, dispy).
; 
; Output:
;  bufaddr = COLOR + dispy*22 + dispx
;--------------------------------------------------------------

gaadd:
	lda dispy
	lsr a
	tay
	lda COLATTTB_hb,y
	sta bufaddr+1
	lda COLATTTB_lb,y
	clc
	adc dispx
	sta bufaddr
	rts

;----------------------------------------------
; Print attributes, properties and pixels.
;
; Input:
;  A	= tile number
;----------------------------------------------

pbpattr:			; entry PUTBLOCK command
	ldy charx
	sty dispx
	ldy chary
	sty dispy
pattr:
.if aflag
	pha
	jsr wbloc		; save blockinfo
	pla
.endif

pattr2:
	sta z80_b		; store cell in b register for now.
	tax
	lda bprop,x 		; block properties.
	sta z80_c
	cmp #COLECT
	bne :+
	lda z80_b
	sta colpatt
:
	jsr pradd 		; get property buffer address.
	lda z80_c
	ldy #0
	sta (bufaddr),y 	; write property.
	lda z80_b 		; restore cell.
	jmp panp
; Print attributes, no properties.

;colpatt:	.byte 0

panp:
	sta z80_e		; displacement in e.
	lda #0
	sta z80_d		; no high byte.
	asl z80_e  		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	clc
	lda z80_e
	adc #<chgfx 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>chgfx
	sta tileaddr+1

	ldx z80_b		; set colour attribute
	lda bCol,x
	pha
	jsr gaadd
	ldy #0
	pla
	sta (bufaddr),y

	jsr gprad 		; get screen address.
	ldx #7			; number of pixel rows to write.
panp0:
	ldy #0
	lda (tileaddr),y 	; get image byte.
.if iflag
	eor #$ff		; Invert
.endif
	ldy scrtab,x
	sta (scraddr),y 	; copy to screen.
	inc tileaddr 		; next image byte.
	bne :+
	inc tileaddr+1
:
	dex	 		; repeat for 8 pixel rows.
	bpl panp0
	inc dispx 		; move along one.
	inc charx
	rts

;----------------------------------------------
; Write block
;----------------------------------------------

.if aflag
wbloc:
	ldy #3
	sta (pbptr),y		; store block number
	dey
	lda dispx
	sta (pbptr),y		; write x position of block.
	dey
	lda dispy
	sta (pbptr),y		; write y position of block.
	dey
	lda scno
	sta (pbptr),y		; write screen.
	clc			; point to next free location
	lda pbptr
	adc #4
	sta pbptr
	bcc :+
	inc pbptr+1
:
	rts
.endif

;.if kflag
;panp:
;.endif

;----------------------------------------------
; Print character pixels, no more.
;
; Input:
;  A	= character to print
;----------------------------------------------

pchr:
	jsr pchar 		; show character in accumulator.
	inc dispx		; move along one.
	rts

;----------------------------------------------------
; Shifter sprite routine for objects.
;----------------------------------------------------

.if oflag
sprit7:
	lda z80_b
	beq sprit0
	sta z80_a
sprit3:
	lsr spr			; shift into position.
	ror spr+1
	ror spr+2
	dec z80_a		; one less iteration.
	bne sprit3
sprit0:
	rts 			; now apply to screen.
.endif

;-----------------------------------------------------------
; Get room address.
;-----------------------------------------------------------

groom:
	ldx scno 		; screen number.
	ldy #0
groomx:
	lda #<scdat 		; pointer to screens.
	sta z80_l
	lda #>scdat
	sta z80_h
groom1:
	cpx #0			; is it the first one?
	beq groom0 		; no more screens to skip.

	clc
	lda z80_l
	adc scdat,y 		; low byte of screen size.
	sta z80_l
	iny			; point to high byte.
	lda z80_h
	adc scdat,y 		; high byte of screen size.
	sta z80_h
	iny			; next address.

	dex 			; one less iteration.
	jmp groom1 		; loop until we reach the end.
groom0:
	lda numsc		; add displacement.
	asl a
	clc			; add double displacement to address.
	adc z80_l
	sta z80_l
	lda z80_h
	adc #0
	sta z80_h
	rts

;-----------------------------------------------------------
; Draw present room.
;-----------------------------------------------------------

droom:
	lda wintop 		; window top.
	sta dispy		; set cursor y position.
droom2:
	jsr groom 		; get address of current room.
	lda #0	 		; zero in accumulator.
	sta comcnt 		; reset compression counter.
	lda winhgt 		; height of window.
	sta rrow		; set row counter
droom0:
	lda winlft 		; window left edge.
	sta dispx 		; set cursor x position.
	lda winwid 		; width of window.
	sta rcol		; set column counter
droom1:
	jsr flbyt 		; decompress next byte on the fly.
.if xflag
	jsr drwmeta
.else
	jsr pattr2 		; show attributes and block.
.endif
	dec rcol		; one less column.
	bne droom1 		; repeat for entire line.
	inc dispy		; move down one line.
.if xflag
	inc dispy		; move down one line.
.endif
	dec rrow 		; one less row.
	bne droom0 		; repeat for all rows.
	rts

;----------------------------------------------
; Decompress bytes on-the-fly.
;----------------------------------------------

flbyt:
	lda comcnt 		; compression counter.
	bne flbyt1		; any more to decompress?  yes.

	ldy #0
	lda (z80_hl),y 		; fetch next byte.
	inc z80_l 		; point to next cell.
	bne :+
	inc z80_h
:
	cmp #255 		; is this byte a control code?
	beq :+
	rts 			; no, this byte is uncompressed.
:
	lda (z80_hl),y 		; fetch byte type.
	sta combyt 		; set up the type.
	inc z80_l 		; point to quantity.
	bne :+
	inc z80_h
:
	lda (z80_hl),y 		; get quantity.
	inc z80_l 		; point to next byte.
	bne :+
	inc z80_h
:
flbyt1:
	sta comcnt 		; store new quantity.
	dec comcnt		; one less.
	lda combyt 		; byte to expand.
	rts

; ------------------------------------------------------------------------------------------------------------------------------------------
; Drawing a MetaBlock (4 tiles 8x8 => 16x16)
; param in regA tells the block number to use, if 0 use 0,0,0,0  else use N,N+2 || N+1,N+3
; ------------------------------------------------------------------------------------------------------------------------------------------
.if xflag
drwmeta:
	sta tmpblk	
	jsr pattr2

	inc dispy
	dec dispx
	ldx tmpblk
	beq :+
	inx
:
	txa
	jsr pattr2

	ldx tmpblk
	beq :+
	inx
	inx
	inx
:
	txa
	jsr pattr2

	dec dispy
	dec dispx
	ldx tmpblk
	beq :+
	inx
	inx
:
	txa
	jsr pattr2
	rts

tmpblk:	.byte 0

.endif

;------------------------------------------
; Ladder down check.
;
; Input:
;  IX = sprite pointer
;------------------------------------------

.if lflag
laddd:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx

	ldy #8
	lda (z80_ix),y		; y coordinate.
	and #254		; make it even.
	sta (z80_ix),y 		; reset it.
	clc 			; look down 16 pixels.
numsp5:
	adc #SPR_HGT
	sta dispy		; coords in dispx,dispy.
	jmp laddv

;------------------------------------------
; Ladder up check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;------------------------------------------

laddu:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx

	ldy #8
	lda (z80_ix),y		; y coordinate.
	and #254 		; make it even.
	sta (z80_ix),y		; reset it.
	clc 			; look 2 pixels above feet.
numsp6:
	adc #SPR_HGT-2
	sta dispy		; coords in dispx,dispy.
laddv:
	jsr tstbl 		; get map address.
	jsr ldchk 		; standard ladder check.
	beq :+
	rts 			; no way through.
:
	inc bufaddr 		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr ldchk 		; do the check.
	beq :+
	rts 			; impassable.
:
	lda dispx 		; y coordinate.
	and #7 			; position straddling block cells.
	bne :+
	rts 			; no more checks needed.
:
	inc bufaddr 		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr ldchk 		; do the check.
	rts  			; return with zero flag set accordingly.
.endif

;---------------------------------------------------------
; Can go up check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangu:
	ldy #9
	lda (z80_ix),y		; x coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sec
	sbc #2
	sta dispy		; coords in dispx,dispy.
	jsr tstbl 		; get map address.
	jsr lrchk 		; standard left/right check.
	beq :+
	rts			; no way through.
:
	inc bufaddr		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr lrchk 		; do the check.
	beq :+
	rts			; impassable.
:
	lda dispx		; x coordinate.
	and #7			; position straddling block cells.
	bne :+
	rts			; no more checks needed.
:
	inc bufaddr		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr lrchk		; do the check.
	rts 			; return with zero flag set accordingly.

;---------------------------------------------------------
; Can go down check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangd:
	ldy #9
	lda (z80_ix),y 		; x coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y		; y coordinate.
	clc
numsp3:
	adc #SPR_HGT 		; look down 16 pixels.
	sta dispy		; coords in dispx,dispy.
	jsr tstbl 		; get map address.
	jsr plchk 		; block, platform check.
	beq :+
	rts			; no way through.
:
	inc bufaddr		; look right one cell.
	bne :+
	inc bufaddr+1
:
	jsr plchk		; block, platform check.
	beq :+
	rts			; impassable.
:
	lda dispx		; x coordinate.
	and #7			; position straddling block cells.
	bne :+
	rts			; no more checks needed.
:
	inc bufaddr		; look to third cell.
	bne :+
	inc bufaddr+1
:
	jsr plchk		; block, platform check.
	rts			; return with zero flag set accordingly.

;---------------------------------------------------------
; Can go left check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangl:
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta dispy
	ldy #9
	lda (z80_ix),y 		; x coordinate.
	sec
	sbc #2			; look left 2 pixels.
	sta dispx		; coords in dispx,dispy.
	jmp cangh		; test if we can go there.

;---------------------------------------------------------
; Can go right check.
;
; Input:
;  IX = sprite pointer
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;---------------------------------------------------------

cangr:
	ldy #8
	lda (z80_ix),y		; y coordinate.
	sta dispy
	ldy #9
	lda (z80_ix),y		; x coordinate.
	clc
	adc #16			; look right 16 pixels.
	sta dispx		; coords in dispx,dispy.
cangh:
cangh2:
	lda #(SPR_HGT/8+1)	; default rows to write.
	sta z80_b
	lda dispy		; y position.
	and #7			; does x straddle cells?
	bne cangh0		; yes, loop counter is good.
	dec z80_b		; one less row to write.
cangh0:
	jsr tstbl		; get map address.
cangh1:
	jsr lrchk		; standard left/right check.
	beq :+
	rts			; no way through.
:
	pha
	clc
	lda bufaddr
	adc #32		; look down.
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	pla

	dec z80_b
	bne cangh1
	rts

;-------------------------------------
; Check left/right movement is okay.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;-------------------------------------

lrchk:
	ldy #0
	lda (bufaddr),y		; fetch map cell.
	cmp #WALL 		; is it passable?
	beq lrchkx		; no.

	cmp #FODDER		; fodder has to be dug.
	beq lrchkx		; not passable.
	lda #0
	rts

;--------------------------------------------------------------
; Called by mmenu
;--------------------------------------------------------------

always:
	lda #255		; report it as okay.
	rts

lrchkx:
	lda #1 			; reset all bits.
	rts


;--------------------------------------------------------------
; Check platform or solid item is not in way.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;--------------------------------------------------------------

plchk:
	ldy #0
	lda (bufaddr),y 	; fetch map cell.
	cmp #WALL 		; is it passable?
	beq lrchkx		; no.
.if pflag .or dflag
	cmp #FODDER		; fodder has to be dug.
	beq lrchkx		; not passable.
.endif
	cmp #PLATFM		; platform is solid.
	beq plchkx		; not passable.
.if lflag
	cmp #LADDER		; is it a ladder?
	beq lrchkx		; on ladder, deny movement.
.endif
plchk0:
	lda #0			; report as ok
	rts
plchkx:
	lda dispy		; x coordinate.
	and #7			; position straddling blocks.
	beq lrchkx		; on platform, deny movement.
	jmp plchk0

;--------------------------------------------------------------
; Check ladder is available.
;
; Input:
;  bufaddr = MAP + x/8 + y/8*32
;
; Output:
;  A  = 0 is ok, A <>0 is not ok
;--------------------------------------------------------------

.if lflag
ldchk:
	ldy #0
	lda (bufaddr),y 	; fetch cell.
	cmp #LADDER 		; is it a ladder?
	beq ldchk1
	lda #1
	rts  			; return with zero flag set accordingly.
ldchk1:
	lda #0
	rts
.endif

;--------------------------------------------------------------
; Get collectables.
;--------------------------------------------------------------

.if cflag
getcol:
        lda #COLECT             ; collectable blocks.
        sta z80_b
        jsr tded                ; test for collectable blocks.
        cmp z80_b               ; did we find one?
        beq :+
        rts                     ; none were found, job done.
:
        jsr gtblk               ; get block.
        jsr evnt20              ; collected block event.
        jmp getcol              ; repeat until none left.

; Get collectable block.

gtblk:
	ldy #0
	lda (bufaddr),y
	sta z80_a
        lda #0
        sta (bufaddr),y		; make it empty now.
       
	lda bufaddr		; set dispx
	and #31
	sta dispx

	lda bufaddr+1		; Set dispy
	sec
	sbc #>MAP
	sta bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	lda bufaddr+1
	sta dispy
 
	lda colpatt		; get blocknr
	sta z80_e		; displacement in e.
	lda #0
	sta z80_d		; no high byte.
	asl z80_e  		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	clc
	lda z80_e
	adc #<chgfx 		; address of graphics.
	sta tileaddr
	lda z80_d
	adc #>chgfx
	sta tileaddr+1

	jsr gprad 		; get screen address.
	ldx #7			; number of pixel rows to write.
gtblk0:
	ldy #0
	lda (tileaddr),y 	; get image byte.
	ldy scrtab,x
	eor (scraddr),y 	; XOR tile on screen
	sta (scraddr),y 	; copy to screen.
	inc tileaddr 		; next image byte.
	bne :+
	inc tileaddr+1
:
	dex	 		; repeat for 8 pixel rows.
	bpl gtblk0
	rts
.endif

;--------------------------------------------------------------
; Touched deadly block check.
; returns with DEADLY (must be non-zero) in accumulator if true.
;
; Input:
;  IX = sprite address
;
; Output:
;  A  = 0 is ok, A=5 is not ok
;--------------------------------------------------------------

tded:
	ldy #8
	lda (z80_ix),y 		; y coordinate.
	sta dispy
	iny
	lda (z80_ix),y 		; x coordinate.
	sta dispx		; coords in dispx,dispy.
	jsr tstbl		; get map address.
	pha
	lda #31			; default distance to next line down.
	sta z80_e
	pla
	cmp z80_b		; is this the required block?
	bne :+
	rts			; yes.
:
	inc bufaddr 		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch type.
	cmp z80_b 		; is this deadly/custom?
	bne :+
	rts			; yes.
:
	lda dispx		; horizontal position.
	sta z80_c 		; store column in c register.
	and #7			; is it straddling cells?
	bne :+
	jmp tded0		; no.
:
	inc bufaddr 		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this the block?
	bne :+
	rts			; yes.
:
	dec z80_e		; one less cell to next row down.
tded0:
	clc 			; point to next row.
	lda bufaddr
	adc z80_e
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch left cell block.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	inc bufaddr 		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	lda z80_c		; horizontal position.
	and #7			; is it straddling cells?
	bne :+
	jmp tded1 		; no.
:
	inc bufaddr		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
tded1:
	lda dispy		; vertical position.
	and #7 			; is it straddling cells?
	bne :+
	rts			; no, job done.
:
	clc			; point to next row.
	lda bufaddr
	adc z80_e
	sta bufaddr
	bcc :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch left cell block.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	inc bufaddr		; next cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y 	; fetch type.
	cmp z80_b		; is this fatal?
	bne :+
	rts			; yes.
:
	lda z80_c		; horizontal position.
	and #7			; is it straddling cells?
	bne :+
	rts			; no.
:
	inc bufaddr		; last cell.
	bne :+
	inc bufaddr+1
:
	ldy #0
	lda (bufaddr),y		; fetch final type.
	rts 			; return with final type in accumulator.

;---------------------------------------------------
; Fetch block type at (dispx, dispy).
;
; Output:
;  A = block type
;---------------------------------------------------

tstbl:
	lda dispy 		; fetch y coord.
	lsr a			; bufaddr = y/8
	lsr a
	lsr a
;	sta chary

	sta bufaddr
	lda #0
	sta bufaddr+1

	asl bufaddr  		; bufaddr = y/8 * 32
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1
	asl bufaddr
	rol bufaddr+1

	lda dispx		; x/8
	lsr a
	lsr a
	lsr a
;	sta charx

	clc			; bufaddr = MAP + x/8 + y/8*32
	adc bufaddr
	adc #<MAP
	sta bufaddr
	lda bufaddr+1
	adc #>MAP
	sta bufaddr+1

	ldy #0
	lda (bufaddr),y 	; fetch byte there.
	rts

;-------------------------------------------------------------------
; Jump - if we can.
; Requires initial speed to be set up in accumulator prior to call.
;
; Input:
;  IX = sprite address
;-------------------------------------------------------------------

jump:
	ldy #var_jumpLo
	lda (z80_ix),y		; jump table low.
	ldy #var_jumpHi
	ora (z80_ix),y		; jump table high.
	beq :+
	rts			; already in the air.
:
	lda #>jtab
	ldy #var_jumpHi
	sta (z80_ix),y		; set jump high.
	lda #<jtab		; jump table start.
	ldy #var_jumpLo
	sta (z80_ix),y		; set jump low.
	rts

; Jump table.
; _BEEB - jump table must reside entirely in a page otherwise grav fn breaks
;.align 32
;jtab:
;	.byte 248,250,252
;	.byte 254,254,255
;	.byte 255,255,0,0
;	.byte 0,1,1,1,2,2
;	.byte 4,6,8,8,8,99

;------------------------------------------------
; Random numbers code.
; Pseudo-random number generator, 8-bit.
;
; Output:
;  RND = random number
;------------------------------------------------

random:
	lda seed
	beq doEor
	asl a
	beq noEor
	bcc noEor
doEor:	eor #$1d
noEor:	sta seed
	rts

;	lda seed		; get last random number.
;	asl a
;	asl a
;	clc
;	adc seed
;	clc
;	adc #$45
;	sta seed		; store new seed.
;	sta varrnd		; return number in variable.
;	rts

;-------------------------------------------------------
; Joystick and keyboard reading routines.
;
; contrl = 0, Keyboard
;          1, JoyKeyb
;          2, JoyMMC
;
;      joyval=x65FUDLR (bit cleared if key pressed)
;             ||||||||
;             |||||||+> Right    KEY 0  - X
;             ||||||+-> Left     KEY 1  - Z
;             |||||+--> Down     KEY 2  - .
;             ||||+---> Up       KEY 3  - ;
;             |||+----> Fire1    KEY 4  - SPC
;             ||+-----> Fire2    KEY 5  - Q
;             |+------> Fire3    KEY 6  - P
;             +-------> Not used
;
;                       Option1  KEY 7  - 1
;                       Option2  KEY 8  - 2
;                       Option3  KEY 9  - 3
;                       Option4  KEY 10 - 4
;-------------------------------------------------------

joykey:
	lda contrl 		; control flag.
	cmp #1
	beq joy1 		; scan joystick1
	cmp #2
	beq joy2 		; scan joystick2

; Scan keyboard

	lda #0		 	; zero reading.
	sta z80_e

	ldy #6	 		; address of last key.
joyke0:
	lda keys,y 		; get key from table.
	jsr ktest		; being pressed?
	rol z80_e 		; rotate into reading.

	dey		 	; next key.
	bpl joyke0 		; repeat for all keys.
joyret:
	lda z80_e
	sta joyval
	rts

; Scan joystick 1

joy1:
	jsr joy1scan
	jmp joyret 		; store the value.

; Scan joystick 2

joy2:
	jsr joy2scan
	jmp joyret 		; store the value.

;---------------------------------------------------------------
; Display message.
;
; Input:
;  A = message number
;---------------------------------------------------------------

dmsg:
	tax
	lda #<msgdat		; pointer to messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
	jsr getwrd		; get message number.
dmsg3:
	jsr preprt		; pre-printing stuff.
	jsr checkx		; make sure we"re in a printable range.
	lda prtmod		; print mode.
	bne bmsg1		; no, double-height text.
dmsg0:
	lda z80_h		; store string pointer.
	pha
	lda z80_l
	pha

	ldy #0
	lda (z80_hl),y		; fetch byte to display.
	and #127		; remove any end marker.
	cmp #ASCII_NEWLINE
	beq dmsg1
	jsr pchar		; display character.
	jsr nexpos 		; display position.
	bne dmsg2		; not on a new line.
	jsr nexlin		; next line down.
dmsg2:
	pla			; retrieve string pointer
	sta z80_l
	pla
	sta z80_h

	ldy #0
	lda (z80_hl),y		; fetch last character.
	asl a  			; was it the end?
	bcc :+
	jmp dscor2		; yes, job done.
:
	inc z80_l		; next character to display.
	bne :+
	inc z80_h
:
	jmp dmsg0
dmsg1:
	inc dispy
	lda dispy
	cmp #24
	bcc dmsg4
	lda #0
	sta dispy
dmsg4:
	lda #0
	sta dispx
	jmp dmsg2


;----------------------------------------------------------
; Display message in big text.
;
; Input:
;  HL = string pointer
;----------------------------------------------------------

bmsg1:
	ldy #0
	lda (z80_hl),y 		; get character to display.
	and #127		; only want 7 bits.
	cmp #ASCII_NEWLINE
	beq bmsg2
	jsr bchar 		; display big char.
bmsg3:
	ldy #0
	lda (z80_hl),y 		; look at last character.
	pha
	inc z80_l 		; next character in list.
	bne :+
	inc z80_h
:
	pla
	asl a  			; was terminator flag set?
	bcc bmsg1		; no, keep going.

	rts
bmsg2:
	lda #0
	sta dispx
	inc dispy
	inc dispy
	lda dispy
	cmp #23
	bcc bmsg3
	lda #0
	sta dispy
	jmp bmsg3

;----------------------------------------------------------
; Big character display.
;
; Input:
;  A = character
;----------------------------------------------------------

bchar:
	sta z80_e		; save char in lb
	lda #0
	sta z80_d		; reset hb

	asl z80_e 		; multiply char by 8.
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d		; de = a*8

	clc			; de = #<(FONT-256) + a*8
	lda z80_e
	adc #<(FONT-256) 		; address of font.
	sta z80_e
	lda z80_d
	adc #>(FONT-256)
	sta z80_d

	jsr gaadd		; set colour attribute
	lda fontcol
	ldy #0
	sta (bufaddr),y

	jsr gprad 		; get screen address.

	ldx #0			; height of character in font.
bchar0:
	ldy #0
	lda (z80_de),y 		; get a bit of the font.

.if iflag
	eor #$ff
.endif

	sta (scraddr),y
	pha
	jsr nline 		; next line down.
	pla
	sta (scraddr),y
	jsr nline 		; next line down.

	clc
	inc z80_e 		; next line of font.
	bne :+
	inc z80_d
:
	inx
	cpx #8
	bne bchar0
	
	jsr nexpos		; display position.
	bne bchar2 		; not on a new line.
bchar3:
	inc dispy
	jsr nexlin 		; next line check.
bchar2:
	jmp dscor2		; tidy up line and column variables.


;-------------------------------------------------
; Display a character.
;
; Input:
;  A = character
;-------------------------------------------------

achar:
	sta z80_b 		; copy to b.
	jsr preprt 		; get ready to print.
	lda z80_b		; character in accumulator.
	ldx prtmod 		; print mode.
	beq :+
	jmp bchar 		; no, double-height text.
:
	jsr pchar 		; display character.
	jsr nexpos 		; display position.
	beq bchar3		; next line down.
	jmp bchar2 		; tidy up.


;-------------------------------------------------
; Get next print column position.
;-------------------------------------------------

nexpos:
	inc dispx		; move along one position.
	lda dispx 		; get coordinate.
	cmp #22
	bne :+
	lda #0
	sta dispx
	lda prtmod
	beq :+
	jsr nexlin
	jmp nexlin
:
	rts

;-------------------------------------------------
; Get next print line position.
;-------------------------------------------------

nexlin:
	inc dispy 		; newline.
	lda dispy		; vertical position.
	cmp #22			; past screen edge?
	bcs :+
	rts			; no, still okay.
:
	lda #0			; restart at top.
	sta dispy
	rts

;--------------------------------------------------------
; Pre-print preliminaries.
;--------------------------------------------------------

preprt:
	lda #<(FONT-256)		; font pointer.
	sta grbase		; set up graphics base.
	lda #>(FONT-256)
	sta grbase+1
prescr:
	lda charx 		; display coordinates.
	sta dispx		; set up general coordinates.
	lda chary
	sta dispy
	rts

;--------------------------------------------------------------
; Get messagenr x in hl
;
; Input:
;  HL = pointer to message list
;  X  = message number.
;--------------------------------------------------------------

getwrd:
	cpx #0
	bne:+ 			; first word in list?
	rts 			; yep, don't search.
:
	ldy #0
getwd0:
	lda (z80_hl),y
	pha
	inc z80_l
	bne :+
	inc z80_h
:
	pla
	cmp #128		; found end?
	bmi getwd0		; no, carry on.
	dex			; until we have right number.
	bne getwd0
	rts

;-----------------------------------------------------------
; Bubble sort.
;-----------------------------------------------------------

bsort:
	lda #NUMSPR - 1		; sprites to swap.
	sta qscnt

	lda #<sprtab 		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
bsort0:
	ldy #0
	lda (z80_ix),y 		; first sprite type.
	cmp #255 		; is it switched off?
	beq swemp		; yes, may need to switch another in here.

	ldy #TABSIZ
	lda (z80_ix),y 		; check next slot exists.
	cmp #255 		; is it enabled?
	beq bsort2 		; no, nothing to swap.

	ldy #TABSIZ+3
	lda (z80_ix),y 		; fetch next sprite's coordinate.
	ldy #3
	cmp (z80_ix),y 		; compare with this x coordinate.
	bcc bsort1		; next sprite is higher - may need to switch.
bsort2:
	clc
	lda z80_x
	adc #TABSIZ 		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec qscnt 
	bne bsort0		; repeat for remaining sprites.
	rts

bsort1:
	ldy #TABSIZ
	lda (z80_ix),y		; sprite on/off flag.
	cmp #255		; is it enabled?
	beq bsort2		; no, nothing to swap.
	jsr swspr		; swap positions.
	jmp bsort2
swemp:
	ldy #TABSIZ
	lda (z80_ix),y		; next table entry.
	cmp #255		; is that one on?
	beq bsort2		; no, nothing to swap.
	jsr swspr		; swap positions.
	jmp bsort2

; Swap sprites.

swspr:
	lda z80_x		; table address 
	sta z80_e		; copy to de pair.
	sta z80_l		; copy to hl pair.
	lda z80_i
	sta z80_h
	sta z80_d

	clc
	lda z80_l
	adc #TABSIZ		; distance to second entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	lda #TABSIZ		; bytes to swap.
	sta z80_b
	ldy #0
swspr0:
	lda (z80_hl),y		; fetch second byte.
	pha
	lda (z80_de),y 		; fetch first byte.
	sta (z80_hl),y 		; copy to second.
	pla
	sta (z80_de),y 		; copy to first sprite entry.

	inc z80_e 		; next byte.
	bne :+	
	inc z80_d
:
	inc z80_l 		; next byte.
	bne :+
	inc z80_h
:
	dec z80_b
	bne swspr0 		; swap all bytes in table entry.
	rts

;----------------------------------------------------
; Process sprites.
;----------------------------------------------------

pspr:
	lda #NUMSPR		; sprites to process.
	sta sprptr

	lda #<sprtab 		; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
pspr1:
	ldy #0
	lda (z80_ix),y		; fetch sprite type.
	cmp #9 			; within range of sprite types?
	bcs :+
	jsr pspr2 		; yes, process this one.
:
	clc
	lda z80_x
	adc #TABSIZ 		; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i		; next sprite.
:
	dec sprptr 		; repeat for remaining sprites.
	bne pspr1
	rts

pspr2:
	lda z80_x 		; store original sprite pointer.
	sta ogptr
	lda z80_i
	sta ogptr+1
	jsr pspr3		; do the routine.
rtorg:
	lda ogptr 		; restore original pointer to sprite.
	sta z80_x
	lda ogptr+1
	sta z80_i
rtorg0:
	rts

pspr3:
	lda #<evtyp0		; sprite type events list.
	sta z80_l
	lda #>evtyp0
	sta z80_h
pspr4:
	lda (z80_ix),y
	asl a			; double accumulator.
	clc
	adc z80_l
	sta z80_l
	bcc :+
	inc z80_h
:
	lda (z80_hl),y
	sta z80_e 		; copy to de.
	pha

	inc z80_l 		; next byte of address.
	bne :+
	inc z80_h
:
	lda (z80_hl),y 		; address high.
	sta z80_d

	pha	 		; swap address into hl.
	lda z80_h
	sta z80_d
	pla
	sta z80_h
	pla
	sta z80_l
	lda z80_l
	sta z80_e
	
	jmp (z80_hl) 		; go there.

; Address of each sprite type's routine.

evtyp0:	.word evnt00
evtyp1:	.word evnt01
evtyp2:	.word evnt02
evtyp3:	.word evnt03
evtyp4:	.word evnt04
evtyp5:	.word evnt05
evtyp6:	.word evnt06
evtyp7:	.word evnt07
evtyp8:	.word evnt08

;--------------------------------------------------------------
; Display sprites.
;
; Input:
;  IX = sprite table
;--------------------------------------------------------------

dspr:
	lda #(NUMSPR/2)		; number of sprites to display.
	sta sprcnt

dspr0:
	ldy #var_Type
	lda (z80_ix),y 		; get sprite type.
	cmp #255 		; is it enabled?
	bne dspr1 		; yes, it needs deleting.
dspr5:
	ldy #var_newType
	lda (z80_ix),y 		; new type.
	cmp #255		; is it enabled?
	beq dspr2
	jmp dspr3 		; yes, it needs drawing.
dspr2:
	ldy #var_newType
	lda (z80_ix),y 		; copy new type.
	ldy #var_Type
	sta (z80_ix),y
	ldy #var_newImage
	lda (z80_ix),y 		; copy new image number.
	ldy #var_Image
	sta (z80_ix),y
	ldy #var_newFrame
	lda (z80_ix),y 		; copy new frame.
	ldy #var_Frame
	sta (z80_ix),y
	ldy #var_newY
	lda (z80_ix),y 		; copy new y.
	ldy #var_Y
	sta (z80_ix),y
	ldy #var_newX
	lda (z80_ix),y 		; copy new x.
	ldy #var_X
	sta (z80_ix),y

	clc
	lda z80_x
	adc #(TABSIZ*2)		; distance to next odd/even entry.
	sta z80_x
	lda z80_i
	adc #0
	sta z80_i 		; next sprite.
	dec sprcnt
	bne dspr0		; repeat for remaining sprites.
	rts

dspr1:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_Y
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     	; beyond maximum?
	bcs dspr5		; yes, don't delete it.

	ldy #var_newType
	lda (z80_ix),y 		; type of new sprite.
	cmp #255		; is this enabled?
	bne dspr4 		; yes, display both.

dspr6:
	jsr sspria 		; show single sprite.
	jmp dspr2

; Displaying two sprites.  Don't bother redrawing if nothing has changed.

dspr4:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_newY
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     	; beyond maximum?
	bcs dspr6		; yes, don't display it.

	ldy #var_X
	lda (z80_ix),y		; old x.
	ldy #var_newX
	cmp (z80_ix),y 		; compare with new value.
	bne dspr7 		; they differ, need to redraw.

	ldy #var_Y
	lda (z80_ix),y		; old y.
	ldy #var_newY
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7		; they differ, need to redraw.

	ldy #var_Frame
	lda (z80_ix),y 		; old frame.
	ldy #var_newFrame
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7 		; they differ, need to redraw.

	ldy #var_Image
	lda (z80_ix),y 		; old image.
	ldy #var_newImage
	cmp (z80_ix),y 		; compare against new value.
	bne dspr7 		; they differ, need to redraw.
	jmp dspr2		; everything is the same, don't redraw.
dspr7:
	jsr sspric 		; delete old sprite, draw new one simultaneously.
	jmp dspr2
dspr3:
	; _BEEB clipping code copied from CPC Engine
	ldy #var_newY
	lda (z80_ix), y		; old x coord
	cmp #SpriteMaxY     	; beyond maximum?
	bcc :+			; no, continue
	jmp dspr2		; yes, don't display it.
:
	jsr ssprib 		; show single sprite.
	jmp dspr2

;-----------------------------------------
; Get sprite address calculations.
; gspran = new sprite, gsprad = old sprite.
;
; Input:
;  IX = sprite address
;-----------------------------------------

gspran:
	ldy #var_newY
	lda (z80_ix),y 		; new y coordinate.
	sta dispy
	ldy #var_newX
	lda (z80_ix),y 		; new x coordinate.
	sta dispx
	ldy #var_newImage
	lda (z80_ix),y 		; new sprite image.
	jsr gfrm		; fetch start frame for this sprite.

	ldy #0
	lda (z80_hl),y 		; frame in accumulator.
	ldy #var_newFrame
	clc
	adc (z80_ix),y 		; new add frame number.
	jmp gspra0

;-----------------------------------------
; Calculate old sprite address
;
; Input:
;  IX = sprite address
;
; Output:
;  B  = right byte mask
;  C  = left byte mask
;  DE = spriteframe address
;  scraddr = screenaddress(dispx,dispy)
;-----------------------------------------

gsprad:
	ldy #var_Y
	lda (z80_ix),y		; y coordinate.
	sta dispy
	ldy #var_X
	lda (z80_ix),y		; x coordinate.
	sta dispx
	ldy #var_Image
	lda (z80_ix),y 		; sprite image.
	jsr gfrm 		; fetch start frame for this sprite.

	ldy #0
	lda (z80_hl),y 		; frame in accumulator.
	ldy #var_Frame
	clc
	adc (z80_ix),y 		; add frame number.

gspra0:
.if rflag
	sta z80_e		; multiply by 32.
	lda #0
	sta z80_d

	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
	asl z80_e
	rol z80_d
.if bflag
	lda z80_e		; multiply by 48
	sta tmp1
	lda z80_d
	sta tmp2
.endif
	asl z80_e
	rol z80_d
.if bflag
	clc
	lda z80_e
	adc tmp1
	sta z80_e
	lda z80_d
	adc tmp2
	sta z80_d
.endif
.else
	lsr a	  		; multiply by 128.
	sta z80_d 		; store in d.
	sta tmp1
	lda #0
	ror a
	sta z80_e 		; got low byte.
	sta tmp2
.if bflag
	lsr tmp1		; multiply by 192
	ror tmp2
	clc
	lda tmp2
	adc z80_e
	sta z80_e
	lda tmp1
	adc z80_d
	sta z80_d
.endif
.endif
	clc 			; address of play sprites.
	lda z80_e
	adc #<sprgfx 
	sta z80_e
	lda z80_d
	adc #>sprgfx
	sta z80_d

	lda dispx 		; y coordinate.
	and #6 			; position within byte boundary.
	tax	 		; low byte of table displacement.
.if rflag
	stx sprshft
.else
	asl a	  		; multiply by 32.
	asl a  			; already a multiple
	asl a  			; of 2, so just 4
.if bflag
	sta tmp1
	asl a  			; shifts needed.
	clc
	adc tmp1
.else
	asl a  			; shifts needed.
.endif
	clc 			; add to sprite address.
	adc z80_e
	sta z80_e
	bcc :+
	inc z80_d
:
.endif
	lda spmask,x		 ; pointer to mask table.
	sta z80_c 		; left mask.
	lda spmask+1,x
	sta z80_b 		; right mask.
; Drop into screen address routine.
	jmp scadd

spmask:	.byte 255,0,63,192,15,240,3,252


;-----------------------------------------------------------
; Animates a sprite.
;
; Input:
;  IX = sprite address
;  HL = last sprite address
;-----------------------------------------------------------

animsp:
	and frmno
	beq :+
	rts
:
	ldy #var_newImage
	lda (z80_ix),y		; sprite image
	jsr gfrm		; get frame data.

	inc z80_l		; point to frames.
	bne :+
	inc z80_h
:
	ldy #var_newFrame
	lda (z80_ix),y		; sprite frame.
	clc
	adc #1			; next one along.
	ldy #0
	cmp (z80_hl),y		; reached the last frame?
	bcc anims0		; no, not yet.
	lda #0			; start at first frame.
anims0:
	ldy #var_newFrame
	sta (z80_ix),y		; new frame.
	rts

;--------------------------------------------------------------
; Animate back
;
; Input:
;  IX = sprite address
;  HL = last sprite address
;--------------------------------------------------------------

animbk:
	and frmno
	beq :+
	rts
:
	ldy #var_newImage
	lda (z80_ix),y		; sprite image.
	jsr gfrm		; get frame data.

	inc z80_l 		; point to frames.
	bne :+
	inc z80_h
:
	ldy #var_newFrame
	lda (z80_ix),y 		; sprite frame.
	beq :+
	jmp rtanb0 		; yes, start at end.
:
	ldy #0
	lda (z80_hl),y 		; last sprite.
rtanb0:
	sec
	sbc #1			; next one along.
	jmp anims0		; set new frame.

;--------------------------------------------------------------
; Check for collision with other sprite, strict enforcement.
;
; Input:
;  b		= sprite to test for
;  ix		= current sprite pointer
;
; global:	b
; local:	x,y,hl,de,skptr
; calls:	-
;--------------------------------------------------------------

sktyp:
	lda #<sprtab				; sprite table.
	sta z80_l
	lda #>sprtab
	sta z80_h
numsp2:
	lda #NUMSPR				; number of sprites.
	sta sktptr
sktyp0:
	lda z80_l 				; store pointer to sprite.
	sta skptr
	lda z80_h
	sta skptr+1

	ldy #0
	lda (z80_hl),y 				; get sprite type.
	cmp z80_b				; is it the type we seek?
	beq coltyp				; yes, we can use this one.
sktyp1:
	clc
	lda skptr				; retrieve sprite pointer.
	adc #TABSIZ				; size of each entry.
	sta z80_l
	lda skptr+1
	adc #0
	sta z80_h
	dec sktptr					; one less iteration.
	bne sktyp0				; keep going until we find a slot.
	lda #0					; default to ROM address - no sprite.
	sta z80_l
	sta z80_h
	sta skptr				; store pointer to sprite.
	sta skptr+1

	clc					; don't return with zero flag set.
	rts 					; didn't find one.

coltyp:
	ldy #0
	lda (z80_ix),y				; current sprite type.
	cmp z80_b				; seeking sprite of same type?
	beq colty1				; yes, need to check we're not detecting ourselves.
colty0:
	ldy #9					; distance to x position in table.
	lda (z80_hl),y				; fetch x coordinate.
	sta z80_e
	dey
	lda (z80_hl),y				; fetch y coordinate.
	sta z80_d

; Drop into collision detection.

colc16:
	ldy #9
	lda (z80_ix),y			 	; x coord.
	sec					; subtract x.
	sbc z80_e
	bcs  colc1a 				; result is positive.
	eor #$ff				; make negative positive.
	clc
	adc #1
colc1a:
	cmp #16 				; within x range?
	bcs sktyp1				; no - they"ve missed.
	sta z80_c				; store difference.

	ldy #8
	lda (z80_ix),y				; y coord.
	sec
	sbc z80_d				; subtract y.
	bcs colc1b				; result is positive.
	eor #$ff				; make negative positive.
	clc
	adc #1
colc1b:
	cmp #16					; within y range?
	bcs sktyp1 				; no - they've missed.
	clc					; add x difference.
	adc z80_c
	cmp #26					; only 5 corner pixels touching?
	bcs :+
	sec
	rts 					; carry set if there's a collision.
:
	jmp sktyp1				; try next sprite in table.
colty1:
	lda z80_x  				; compare the two.
	cmp z80_l
	bne end_col
	lda z80_i
	cmp z80_h
	bne end_col
	jmp sktyp1 				; addresses are identical.
end_col:
	jmp colty0

;-----------------------------------------------------------
; Display number, left aligned
; 
; Input:
;  a		= number
;
; global:	-
; local:	a,y,bc,hl,displ0
; calls:	num2ch,dmsg3
;-----------------------------------------------------------

disply:
	sta z80_a
	lda #<displ0				; display workspace.
	sta z80_c
	lda #>displ0
	sta z80_b
	lda z80_a
	jsr num2ch				; convert accumulator to string.
displ1:
	dec z80_c				; back one character.
	lda z80_c
	cmp #$ff
	bne :+
	dec z80_b
:
	ldy #0
	lda (z80_bc),y				; fetch digit.
	ora #128				; insert end marker.
	sta (z80_bc),y				; new value.

	lda #<displ0				; display space.
	sta z80_l
	lda #>displ0
	sta z80_h
	jmp dmsg3				; display the string.

displ0:	.byte 0,0,0,13+128

;----------------------------------------------------------------
; Initialise screen.
;
; global:	roomtb,scno
; local:	-
; calls:	tstcs
;----------------------------------------------------------------

initsc:
	lda roomtb 			; whereabouts in the map are we?
	jsr tstsc 			; find displacement.
	cmp #255 			; is it valid?
	beq init_end 			; no, it's rubbish.
	sta scno			; store new room number.
init_end:
	rts

;----------------------------------------------------------------
; Test screen.
;
; global:	-
; local:	x
; calls:	-
;----------------------------------------------------------------

tstsc:
	sta tmproom
	clc
	adc #MAPWID 			; add width in case we"re negative.
	tax 				; add displacement to map data.
	lda mapdat-MAPWID,x 		; find room number there.
	rts

;--------------------------
; Screen left.
;--------------------------

scrl:
	lda roomtb 			; present room table pointer.
	sec
	sbc #1				; room left.
scrl0:
	jsr tstsc			; test screen.
	cmp #255			; is there a screen this way?
	bne :+
	rts				; no, return to loop.
:
	lda tmproom			; restore room displacement.
	sta roomtb			; new room table position.
scrl1:
	jsr initsc 			; set new screen.
	lda #2
	sta restfl 			; set it.
	rts
scrr:
	lda roomtb 			; room table pointer.
	clc
	adc #1				; room right.
	jmp scrl0
scru:
	lda roomtb 			; room table pointer.
	sec
	sbc #MAPWID 			; room up.
	jmp scrl0
scrd:
	lda roomtb 			; room table pointer.
	clc
	adc #MAPWID 			; room down.
	jmp scrl0

;-----------------------------------------
; Jump to new screen.
;-----------------------------------------

nwscr:
	ldx #0				; start of map data.
nwscr0:
	cmp mapdat,x
	beq nwscr1			; have we found a match for screen?
	inx 				; next room.
	cpx #80				; zero room count, 80 to search.
	bne nwscr0			; keep looking.
	rts
nwscr1:
	stx roomtb			; set the map position.
	jmp scrl1			; draw new room.


;----------------------------------------------------------
; Gravity processing.
;----------------------------------------------------------

grav:
	ldy #13
	lda (z80_ix),y			; jump pointer low.
	sta z80_l
	ldy #14
	lda (z80_ix),y			; jump pointer high.
	sta z80_h
	ora z80_l			; merge in low byte.
	bne :+
	rts				; if neither is set, we're not in the air.
:
	ldy #0
	lda (z80_hl),y			; pixels to move.
	sta z80_a
	cmp #99				; reached the end?
	bne grav0			; no, continue.
grav2:
	dec z80_l			; go back to previous value.
	cmp #$ff
	bne :+
	dec z80_h
:
	lda (z80_hl),y			; fetch that from table.
	sta z80_a
grav0:
	inc z80_l			; point to next table entry.
	bne :+
	inc z80_h
:
	lda z80_l
	ldy #13
	sta (z80_ix),y			; store new pointer low.
	lda z80_h
	ldy #14
	sta (z80_ix),y			; store new pointer high.
grav1:
	lda z80_a
	bne :+				; any movement required?
	rts				; no, not this time.
:
	lda z80_a
	cmp #128			; is it up or down?
	bcs gravu			; it's up.
gravd:
	sta z80_b			; set pixels to move.
gravd0:
	jsr cangd			; can we go down?
	bne gravst			; can't move down, so stop.
	ldy #8
	lda (z80_ix),y			; adjust new x coord.
	clc
	adc #1
	sta (z80_ix),y
	dec z80_b
	bne gravd0
	rts
gravu:
	eor #$ff			; flip the sign so it's positive.
	clc
	adc #1
	sta z80_b			; set pixels to move.
gravu0:
	jsr cangu			; can we go up?
	bne ifalls			; can't move up, go down next.
	ldy #8
	lda (z80_ix),y
	sec
	sbc #1
	sta (z80_ix),y			; adjust new x coord.
	dec z80_b
	bne gravu0
	rts
gravst:
	ldy #var_jumpLo
	lda (z80_ix),y			; jump pointer low.
	sta z80_l
	ldy #var_jumpHi
	lda (z80_ix),y			; jump pointer high.
	sta z80_h

	lda #0				; null value in pointer.
	ldy #var_jumpLo
	sta (z80_ix),y			; store new pointer low.
	iny
	sta (z80_ix),y			; store new pointer high.

	ldy #0
	lda (z80_hl),y			; fetch byte from table.
	cmp #99				; is it the end marker?
evftf:
	beq :+				; yes, fallen too far.
	rts
:
	jmp evnt15			; EVENT FELLTOOFAR

;------------------------------------------------
; Initiate fall check.
;------------------------------------------------

ifall:
	ldy #var_jumpLo
	lda (z80_ix),y 			; jump pointer low.
	sta z80_l
	ldy #var_jumpHi
	lda (z80_ix),y 			; jump pointer high.
	sta z80_h			; high byte in accumulator.
	ora z80_l			; merge in low byte.
	beq :+
	rts				; if either is set, we're already in the air.
:
	ldy #9
	lda (z80_ix),y			; y coordinate.
	sta dispx
	ldy #8
	lda (z80_ix),y			; look x coordinate.
	clc
numsp7:
	adc #SPR_HGT			; add 16 pixels.
	sta dispy			; set up test coordinates.
	jsr tstbl			; get map address.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
	inc bufaddr			; look right one cell.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
	lda dispx			; y coordinate.
	and #7				; position straddling block cells.
	beq ifalls			; no more checks needed.
	inc bufaddr			; look to third cell.
	jsr plchk			; block, platform check.
	beq :+
	rts				; it's solid, don't fall.
:
ifalls:
	lda #<jtab			; jump table start.
	sta z80_l
	lda #>jtab
	sta z80_h
ifal0:
	inc z80_l			; point to next value.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y			; fetch value.
	beq ifal0			; no, get next value.
	cmp #99				; reached end of table?
	bne :+
	rts				; yes, don't fall.
:
	cmp #128			; is it going up?
	bcs ifal0			; yes, looking for first movement down.

	ldy #13
	lda z80_l
	sta (z80_ix),y 			; set jump low.
	ldy #14
	lda z80_h
	sta (z80_ix),y 			; set jump high.
	rts


;----------------------------------------------------
; Get frame data for a particular sprite.
; Input:
;  a		= framenumer
; Output:
;  hl		= frame address
;
; global:	hl,frmptr
; local:	-
; calls:	-
;----------------------------------------------------

gfrm:
	asl a	 		 	; multiple of 2.
	clc
	adc frmptr 			; frames used by game.
	sta z80_l
	lda frmptr+1
	adc #0
	sta z80_h 			; point to frame start.
	rts

;----------------------------------------------------
; Find sprite list for current room.
;
; global:	hl
; local:	x,y
; calls:	-
;----------------------------------------------------

sprlst:
	lda #<nmedat 			; list of enemy sprites.
	sta z80_l
	lda #>nmedat
	sta z80_h
	ldx scno 			; screen number.
	bne sprls2 			; is it the first screen?
	rts 				; yes, don't need to search data.
sprls2:
	ldy #0
sprls1:
	lda (z80_hl),y 			; fetch type of sprite.
	cmp #255			; is it an end marker?
	beq sprls0 			; yes, end of this room.

	clc 				; point to next sprite in list.
	lda z80_l
	adc #NMESIZ
	sta z80_l
	bcc :+
	inc z80_h
:
	jmp sprls1 			; continue until end of room.
sprls0:
	inc z80_l 			; point to start of next screen.s
	bne :+
	inc z80_h
:
	dex
	bne sprls1 			; continue until room found.
	rts


;----------------------------------------------------
; Clear all but a single player sprite.
;
; global:	-
; local:	x,y,ix
; calls:	-
;----------------------------------------------------

nspr:
	lda #NUMSPR			; sprite slots in table.
	sta sprcnt
	lda #<sprtab 			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
nspr0:
	ldy #0 				; fetch sprite type.
	lda (z80_ix),y 			; is it a player?
	beq nspr1 			; yes, keep this one.

	lda #255
	ldy #0 				; fetch sprite type.
	sta (z80_ix),y 			; delete sprite.
	ldy #5
	sta (z80_ix),y 			; remove next type.

	clc	 			; next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less space in the table.
	bne nspr0
	rts
nspr1:
	lda #255
	ldy #0
	sta (z80_ix),y 			; delete sprite.

	clc	 			; point to next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less to do.
	bne nspr2
	rts
nspr2:
	lda #255
	ldy #0
	sta (z80_ix),y 			; delete sprite.
	ldy #5
	sta (z80_ix),y 			; remove next type.

	clc	 			; next sprite.
	lda z80_x
	adc #TABSIZ 			; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt	 			; one less space in table.
	bne nspr2
	rts

;----------------------------------------------------------
; Two initialisation routines.
; Initialise sprites - copy everything from list to table.
;
; global:	-
; local:	x,y,ix
; calls:	cpsp
;----------------------------------------------------------

ispr:
	lda #NUMSPR			; sprite slots in table.
	sta sprcnt
	lda #<sprtab			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
ispr2:
	ldy #0
	lda (z80_hl),y 			; fetch byte.
	cmp #255 			; is it an end marker?
	bne :+
	rts 				; yes, no more to do.
:
ispr1:
	ldy #0
	lda (z80_ix),y 			; fetch sprite type.
	cmp #255 			; is it enabled yet?
	bne ispr4			; yes, try another slot.

	ldy #5
	lda (z80_ix),y		 	; next type.
	cmp #255 			; is it enabled yet?
	beq ispr3 			; no, process this one.
ispr4:
	clc 				; next sprite.
	lda z80_x
	adc #TABSIZ		 	; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dec sprcnt
	bne ispr1 			; repeat for remaining sprites.
	rts  				; no more room in table.
ispr3:
	jsr cpsp			; initialise a sprite.
	dec sprcnt			; one less space in the table.
	bne ispr2
	rts


;-----------------------------------------------------------------------
; Initialise sprites - but not player, we're keeping the old one.
;
; global:	-
; local:	x,y,ix
; calls:	cpsp
;-----------------------------------------------------------------------

kspr:
	ldx #NUMSPR			; sprite slots in table.
	lda #<sprtab 			; sprite table.
	sta z80_x
	lda #>sprtab
	sta z80_i
kspr2:
	ldy #0
	lda (z80_hl),y 			; fetch byte.
	cmp #255 			; is it an end marker?
	bne :+
	rts 				; yes, no more to do.
:
	cmp #0
	bne kspr1 			; no, add to table as normal.

	clc 				; next sprite.
	lda z80_l
	adc #NMESIZ		 	; distance to next odd/even entry.
	sta z80_l
	bcc :+
	inc z80_h
:
	jmp kspr2
kspr1:
	ldy #0 				; fetch sprite type.
	lda (z80_ix),y
	cmp #255 			; is it enabled yet?
	bne kspr4 			; yes, try another slot.

	ldy #5 				; next type.
	lda (z80_ix),y
	cmp #255 			; is it enabled yet?
	beq kspr3 			; no, process this one.
kspr4:
	clc 				; next sprite.
	lda z80_x
	adc #TABSIZ		 	; distance to next odd/even entry.
	sta z80_x
	bcc :+
	inc z80_i
:
	dex	 			; repeat for remaining sprites.
	bne kspr1
	rts  				; no more room in table.
kspr3:
	jsr cpsp 			; copy sprite to table.
	dex	 			; one less space in the table.
	bne kspr2
	rts

;----------------------------------------------
; Copy sprite from list to table.
;
; global:	hl,ix
; local:	y
; calls:	evnt09
;----------------------------------------------

cpsp:
	ldy #0				; fetch byte from table.
	lda (z80_hl),y
	; y=var_Type
	sta (z80_ix),y			; set up type.
	ldy #var_newType
	sta (z80_ix),y 			; set up type.

	inc z80_l 			; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0 				; fetch byte from table.
	lda (z80_hl),y
	ldy #var_newImage
	sta (z80_ix),y			; set up image.

	inc z80_l 			; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0
	lda (z80_hl),y 			; fetch byte from table.
	ldy #var_newY
	sta (z80_ix),y 			; set up coordinate.

	lda #200 			; set initial coordinate off screen.
	ldy #var_Y
	sta (z80_ix),y

	inc z80_l 			; move to next byte.
	bne :+
	inc z80_h
:
	ldy #0 				; fetch byte from table.
	lda (z80_hl),y
	ldy #var_newX
	sta (z80_ix),y 			; set up coordinate.

	inc z80_l 			; move to next byte.
	bne :+
	inc z80_h
:
	lda #0				; zeroes in accumulator.
	ldy #var_newFrame 		; reset frame number.
	sta (z80_ix),y
	ldy #var_Direction 		; reset direction.
	sta (z80_ix),y
	ldy #var_jumpLo			; reset jump pointer low.
	sta (z80_ix),y
	ldy #var_jumpHi	 		; reset jump pointer high.
	sta (z80_ix),y

	lda #255 			; reset data pointer to auto-restore.
	ldy #var_dataHi
	sta (z80_ix),y

	lda fontcol			; set background INK
	ldy #var_colour
	sta (z80_ix),y

evis0:
	lda z80_i
	pha
	lda z80_x
	pha
	lda z80_h
	pha
	lda z80_l
	pha

	jsr evnt09 				; perform event.

	pla
	sta z80_l
	pla
	sta z80_h
	pla
	sta z80_x
	pla
	sta z80_i

	clc
	lda z80_x 			; distance to next odd/even entry.
	adc #TABSIZ		 	; next sprite.
	sta z80_x
	bcc :+
	inc z80_i
:
	rts


;-------------------------------------
; Clear the play area window.
;-------------------------------------

clw:
	lda wintop			; get coordinates of window.
	sta dispy			; put into dispx for calculation.
	lda winlft
	sta dispx

	lda winhgt			; height of window.
	sta rrow			; copy to b register.
clw3:
	lda winwid 			; width of window.
	sta rcol
clw2:
	jsr gprad 			; get print address.
	lda #32
	jsr pchar
	inc dispx			; next column.
	dec rcol			; one less to do.
	bne clw2			; repeat for remaining columns.

	lda winlft			; get left edge.
	sta dispx 			; reset x.
	inc dispy 			; next line down.

	dec rrow
	bne clw3			; repeat down the screen.

	lda wintop			; get coordinates of window.
	sta chary			; put into display position.
	lda winlft
	sta charx
	rts


;----------------------------------------------------------
; Effects code.
; Ticker routine is called 25 times per second.
;
; txtini = text scroller address
; txtscr = left text screen address
; txtwid = scroller width
; txtbit = 128
; X      = message nr
;----------------------------------------------------------

.if sflag

scrollpos_lb:	.byte $00,$10,$20,$30,$40,$50,$60,$70
		.byte $80,$90,$a0,$b0,$c0,$d0,$e0,$f0
		.byte $00,$10,$20,$30,$40,$50
scrollpos_hb:	.byte $00,$00,$00,$00,$00,$00,$00,$00
		.byte $00,$00,$00,$00,$00,$00,$00,$00
		.byte $01,$01,$01,$01,$01,$01

scrly:

; Set scr_l = txtscr

	rts
	.word txtscr         	; set scr_l = left screen address.
	sta scr_l
	sta scr_r
	lda txtscr+1
	sta scr_l+1
	sta scr_r+1
	
	stx xtmp		; save x

; Calculate scr_r = scr_l + 16*(txtwid -1)

	ldx txtwid
	dex
	lda scrollpos_lb,x
	clc
	adc scr_r
	sta scr_r		; set lb scr_r = scr_l + (txtwid - 1) * 8
	bcc :+
	inc scr_r+1
:
	clc
	lda scrollpos_hb,x
	adc scr_r+1
	sta scr_r+1

; Scroll string

	lda #0
	sta line_cnt		; Set line counter
scrly1:
	clc
	lda scr_r
	adc line_cnt
	sta scr_tmp
	lda scr_r+1
	sta scr_tmp+1

	ldx txtwid		; set column counter
	dex
	ldy #0
	clc			; Reset pixel to be shifted in
scrly0:
	lda (scr_tmp),y
	rol a			; Shift screenbyte
	sta (scr_tmp),y		; store screenbyte
	php			; save carry

	sec			; subtract 16 form screenaddress
	lda scr_tmp
	sbc #$10
	sta scr_tmp
	bcs :+
	dec scr_tmp+1
:
	plp			; restore carry
	dex			; previous column
	bpl scrly0		; repeat for all columns

	inc line_cnt		; next line
	lda line_cnt
	cmp #8
	bne scrly1		; repeat for 8 lines

; Get next character in string

	lda txtpos 		; get text pointer.
	sta scr_txt
	lda txtpos+1
	sta scr_txt+1

	ldy #0
	lda (scr_txt),y 	; find character we're displaying.
	and #127 		; remove end marker bit if applicable.
	cmp #13			; is it newline?
	bne scrly5 		; no, it's okay.
	lda #32			; convert to a space instead.
scrly5:
	sta fntaddr		; calculate char address
	lda #0
	sta fntaddr+1
	asl fntaddr  		; multiply char by 8.
	rol fntaddr+1
	asl fntaddr
	rol fntaddr+1
	asl fntaddr
	rol fntaddr+1
	lda fntaddr
	clc
	adc #<(FONT-256)
	sta scrly3+1		; that's the low byte.
	lda fntaddr+1
	adc #>(FONT-256)
	sta scrly3+2		; add displacement.

	ldy #0
scrly3:
	lda $3333,y		; get image of char line.
	and txtbit
	beq scrly2		; don't plot pixel
	lda (scr_r),y
	clc
	ora #1
	sta (scr_r),y		; plot pixel
scrly2:
	iny			; next line of char.
	cpy #8
	bne scrly3

	lsr txtbit		; bit of text to display.
	bcs :+
	rts
:
	ldy #0
	lda (scr_txt),y 	; what was the character?
	asl a	  		; end of message?
	bcs scrly4
	inc txtpos
	bne :+
	inc txtpos+1
:
	jmp scrly6 		; not yet - continue.
scrly4:
	lda txtini 		; start of scrolling message.
	sta txtpos
	lda txtini+1
	sta txtpos+1
scrly6:
	lda #128
	sta txtbit
	ldx xtmp
	rts

tmp_byte:	.byte 0
line_cnt:	.byte 0

;-------------------------------------------------------
; Entry TICKER command
;
; Entry:
;  z80_b = message nr
;  z80_c = width
;-------------------------------------------------------

iscrly:
	jsr prescr 		; set up display position.
	lda #<msgdat 		; text messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
	lda z80_c 		; width.
	sec
	sbc #1			; subtract one.
	cmp #32 		; is it between 1 and 32?
	bcc :+
	lda #$60
	jmp iscrl0		; no, disable messages.
:
	ldx z80_b		; message number.
	jsr getwrd 		; find message start.

	lda z80_l		; set initial text position.
	sta txtini
	lda z80_h
	sta txtini+1

	lda #$ad		; code for lda adrr
iscrl0:
	sta scrly		; enable/disable scrolling routine.

	jsr prescr 		; set up display position.
	jsr gprad 		; get print address.

	lda scraddr 		; set text screen address.
	sta txtscr
	lda scraddr+1
	sta txtscr+1

	lda z80_c		; width.
	sta txtwid		; set width in working storage.

	lda #128 		; start with leftmost bit.
	sta txtbit

	jmp scrly4
.endif

;------------------------------------------------------------------
; Dig routine, conditional assembly depending on dflag
;------------------------------------------------------------------
.if dflag
dig:
	and #3
	beq digr		; dig right
	cmp #1
	beq digl		; dig left
	cmp #2
	beq digd		; dig down

; Dig up.

digu:				; dig up
	ldy #8
	lda (z80_ix),y
	sec
	sbc #2
	sta dispy		; set y

	iny
	lda (z80_ix),y
	sta dispx		; set x
	jmp digv

; Dig down.

digd:
	ldy #9
	lda (z80_ix),y
	sta dispx		; set y

	dey
	clc
	lda (z80_ix),y
	adc #16
	sta dispy		; set y
	jmp digv

; Dig left.

digl:
	ldy #8
	lda (z80_ix),y
	sta dispy		; set y

	iny
	lda (z80_ix),y
	sec
	sbc #2			; x=x-2
	sta dispx		; set x
	jmp digh

; Dig right.

digr:
	ldy #8
	lda (z80_ix),y
	sta dispy		; set y

	iny
	lda (z80_ix),y
	clc
	adc #16
	sta dispx		; set x+16
	jmp digh

; Vertical digging

digv:
	jsr tstbl		; check blocktype in MAP
	jsr fdchk		; test if FODDER

	clc
	lda dispx		; look 1 cell down
	adc #8
	sta dispx
	jsr tstbl		; check blocktype in MAP
	jsr fdchk
	lda dispx
	and #7
	bne :+
	rts
:
	clc
	lda dispx		; look 1 cell down
	adc #8
	sta dispx
	jsr tstbl		; check blocktype in MAP
	jmp fdchk

; Horizontal digging

digh:
	jsr tstbl		; check blocktype in MAP
	jsr fdchk		; test if FODDER

	clc
	lda dispy		; look 1 cell down
	adc #8
	sta dispy
	jsr tstbl		; check blocktype in MAP
	jsr fdchk
	lda dispy
	and #7
	bne :+
	rts
:
	clc
	lda dispy		; look 1 cell down
	adc #8
	sta dispy
	jsr tstbl		; check blocktype in MAP
	jmp fdchk

digcnt:	.byte 0

.endif

;------------------------------------------------------------------
; Sprite table 
;------------------------------------------------------------------


; ix+0  = type.
; ix+1  = sprite image number.
; ix+2  = frame.
; ix+3  = y coord.
; ix+4  = x coord.

; ix+5  = new type.
; ix+6  = new image number.
; ix+7  = new frame.
; ix+8  = new y coord.
; ix+9  = new x coord.

; ix+10 = direction.
; ix+11 = parameter 1.
; ix+12 = parameter 2.
; ix+13 = jump pointer low.
; ix+14 = jump pointer high.
; ix+15 = data pointer low.
; ix+16 = data pointer high.

; block NUMSPR * TABSIZ,255

sprtab:	.res NUMSPR*TABSIZ,255
ssprit:	.byte 255,255,255,255,255,255,255,0,192,120,0,0,0,255,255,255,255
roomtb:	.byte 7                      ; start room map offset.

setfgcol:
setbgcol:
	rts

; User routine.  Put your own code in here to be called with USER instruction.
; if USER has an argument it will be passed in the accumulator.

user:
	.include "user.inc"

; Everything below here will be generated by the editors.


        rts
WINDOWTOP = 0
WINDOWLFT = 0
WINDOWHGT = 21
WINDOWWID = 22 ;a
MAPWID = 3
        .byte 255,255,255
mapdat:
        .byte 255,0,255,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
        .byte 255,255,255
stmap:  .byte 1

evnt00:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #1
        cmp vard
        beq *+5
        jmp a00113
        lda #120		; BEEP
        sta sndtyp
        lda #8	; SPAWN
        sta z80_c
        lda #6
        sta z80_b
        jsr spawn
        lda ogptr	; ENDSPRITE
        sta z80_x
        lda ogptr+1
        sta z80_i
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
a00113: lda joyval	; KEY
        and #16
        beq :+
        jmp a00575
:
        lda #2
        ldy #6
        cmp (z80_ix),y
        bne *+5
        jmp a00306
        lda #0
        cmp vara
        beq *+5
        jmp a00182
        lda #0		; ANIMATE
        jsr animsp
a00182: lda #8		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp a00298
:
        lda #DEADLY	; DEADLY
        sta z80_b
        jsr tded
        cmp z80_b
        beq :+
        jmp a00248
:
        lda #1
        sta varb
        jmp a00294
a00248: lda #7 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp a00286
:
        lda #1
        sta varb
        jmp a00294
a00286: lda #2
        sta varb
a00294: jmp a00306
a00298: lda #1
        sta varb
a00306: lda #1
        cmp varb
        beq *+5
        jmp a00377
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp a00363
        lda #3
        ldy #6
        sta (z80_ix),y
        jmp a00377
a00363: lda #4
        ldy #6
        sta (z80_ix),y
a00377: lda #2
        cmp varb
        beq *+5
        jmp a00570
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp a00434
        lda #15
        ldy #6
        sta (z80_ix),y
        jmp a00447
a00434: lda #16
        ldy #6
        sta (z80_ix),y
a00447: lda #0
        cmp vara
        beq *+5
        jmp a00570
        lda #2
        ldy #7
        cmp (z80_ix),y
        beq *+5
        jmp a00570
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp a00536
        lda #4	; SPAWN
        sta z80_c
        lda #18
        sta z80_b
        jsr spawn
        jmp a00560
a00536: lda #4	; SPAWN
        sta z80_c
        lda #19
        sta z80_b
        jsr spawn
a00560: lda #20		; BEEP
        sta sndtyp
a00570: jmp a00583
a00575: lda #0
        sta varb
a00583: lda joyval	; KEY
        and #8
        beq :+
        jmp a00737
:
        lda #0
        ldy #8
        cmp (z80_ix),y
        bcs *+5
        jmp a00655
        jsr scru	; SCREENUP
        lda #152
        ldy #8
        sta (z80_ix),y
        rts		; EXIT
        jmp a00737
a00655: jsr laddu	; CANGOUP
        beq :+
        jmp a00737
:
        lda #2
        ldy #6
        sta (z80_ix),y
        lda #0
        cmp vara
        beq *+5
        jmp a00712
        lda #0		; ANIMATE
        jsr animsp
a00712: ldy #8 		; SPRITEUP
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
a00737: lda joyval	; KEY
        and #4
        beq :+
        jmp a00893
:
        lda #150
        ldy #8
        cmp (z80_ix),y
        bcc *+5
        jmp a00809
        jsr scrd	; SCREENDOWN
        lda #2
        ldy #8
        sta (z80_ix),y
        rts		; EXIT
        jmp a00893
a00809: jsr laddd	; LADDERBELOW
        beq :+
        jmp a00893
:
        lda #2
        ldy #6
        sta (z80_ix),y
        lda #0
        cmp vara
        beq *+5
        jmp a00867
        lda #0		; ANIMATE
        jsr animsp
a00867: ldy #8 		; SPRITEDOWN
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
a00893: lda joyval	; KEY
        and #1
        beq :+
        jmp a01222
:
        lda #0
        cmp scno
        bne *+5
        jmp a01042
        lda #24
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a00956
        jmp a01042
a00956: lda #56
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a00983
        jmp a01042
a00983: lda #80
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01010
        jmp a01042
a01010: lda #136
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01037
        jmp a01042
a01037: rts		; EXIT
a01042: lda joyval	; KEY
        and #16
        beq :+
        jmp a01071
:
        rts		; EXIT
        jmp a01222
a01071: lda #159
        ldy #9
        cmp (z80_ix),y
        bcc *+5
        jmp a01124
        jsr scrr	; SCREENRIGHT
        lda #0
        ldy #9
        sta (z80_ix),y
        rts		; EXIT
        jmp a01222
a01124: jsr cangr	; CANGORIGHT
        beq :+
        jmp a01222
:
        lda #0
        ldy #6
        sta (z80_ix),y
        lda #0
        ldy #11
        sta (z80_ix),y
        lda #0
        cmp vara
        beq *+5
        jmp a01196
        lda #0		; ANIMATE
        jsr animsp
a01196: ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
a01222: lda joyval	; KEY
        and #2
        beq :+
        jmp a01550
:
        lda #0
        cmp scno
        bne *+5
        jmp a01371
        lda #24
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01285
        jmp a01371
a01285: lda #56
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01312
        jmp a01371
a01312: lda #80
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01339
        jmp a01371
a01339: lda #136
        ldy #8
        cmp (z80_ix),y
        beq *+5
        jmp a01366
        jmp a01371
a01366: rts		; EXIT
a01371: lda joyval	; KEY
        and #16
        beq :+
        jmp a01400
:
        rts		; EXIT
        jmp a01550
a01400: lda #0
        ldy #9
        cmp (z80_ix),y
        bcs *+5
        jmp a01453
        jsr scrl	; SCREENLEFT
        lda #160
        ldy #9
        sta (z80_ix),y
        rts		; EXIT
        jmp a01550
a01453: jsr cangl	; CANGOLEFT
        beq :+
        jmp a01550
:
        lda #1
        ldy #6
        sta (z80_ix),y
        lda #1
        ldy #11
        sta (z80_ix),y
        lda #0
        cmp vara
        beq *+5
        jmp a01524
        lda #0		; ANIMATE
        jsr animsp
a01524: ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
a01550: lda #DEADLY	; DEADLY
        sta z80_b
        jsr tded
        cmp z80_b
        beq :+
        jmp a01668
:
        lda #7 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp a01609
:
        jmp a01668
a01609: lda #7
        cmp varr
        beq *+5
        jmp a01668
        lda #0
        cmp varo
        bcc *+5
        jmp a01668
        lda #5		; BEEP
        sta sndtyp
        lda varo
        sec
        sbc #1
        sta varo
a01668: jsr skobj	; DETECTOBJECT
        sta varobj
        lda #255
        cmp varobj
        bne *+5
        jmp a01912
        lda joyval	; KEY
        and #8
        beq :+
        jmp a01912
:
        lda varp
        sta varm
        lda #6
        cmp varobj
        bcs *+5
        jmp a01773
        lda #30
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
        jmp a01798
a01773: lda #20
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
a01798: lda #0
        cmp varobj
        beq *+5
        jmp a01839
        lda #50
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
a01839: lda #100
        cmp varp
        bcc *+5
        jmp a01875
        lda varm
        sta varp
        rts		; EXIT
        jmp a01912
a01875: lda varobj	; GET
        jsr getob
        lda #30		; BEEP
        sta sndtyp
        lda varn
        clc
        adc #1
        sta varn
a01912: lda joyval	; KEY
        and #32
        beq :+
        jmp a02195
:
        lda #2
        ldy #6
        cmp (z80_ix),y
        bne *+5
        jmp a02195
        lda #0
        cmp varn
        bcc *+5
        jmp a02195
        lda #2		; INVENTORY
        tax
        jsr minve
        lda #10
        cmp varopt
        bne *+5
        jmp a02179
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda varopt
        jsr drpob
        lda #30		; BEEP
        sta sndtyp
        lda varn
        sec
        sbc #1
        sta varn
        lda #30
        sta z80_c
        lda varp
        sec
        sbc z80_c
        sta varp
        lda #6
        cmp varopt
        bcc *+5
        jmp a02133
        lda #10
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
a02133: lda #0
        cmp varopt
        beq *+5
        jmp a02174
        lda #50
        sta z80_c
        lda varp
        sec
        sbc z80_c
        sta varp
a02174: jmp a02184
a02179: rts		; EXIT
a02184: lda #10 	; DELAY
        jsr delay
a02195: lda #CUSTOM	; CUSTOM
        sta z80_b
        jsr tded
        cmp z80_b
        beq :+
        jmp a02550
:
        lda #0
        cmp scno
        beq *+5
        jmp a02446
        jsr skobj	; DETECTOBJECT
        sta varobj
        lda #6
        cmp varobj
        bcs *+5
        jmp a02418
        lda varobj	; GET
        jsr getob
        lda varq
        clc
        adc #1
        sta varq
        ldy #8
        lda (z80_ix),y
        sta varj
        ldy #9
        lda (z80_ix),y
        sta vark
        lda #192
        ldy #8
        sta (z80_ix),y
        lda #8
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda varobj
        jsr drpob
        lda varj
        ldy #8
        sta (z80_ix),y
        lda vark
        ldy #9
        sta (z80_ix),y
a02418: lda #7
        cmp varq
        beq *+5
        jmp a02446
        lda #1		; ENDGAME
        sta gamwon
a02446: lda #0
        cmp scno
        bne *+5
        jmp a02550
        lda #7		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp a02491
:
        jmp a02550
a02491: lda #0
        cmp vara
        beq *+5
        jmp a02550
        lda #0
        cmp varo
        bcc *+5
        jmp a02550
        lda #5		; BEEP
        sta sndtyp
        lda varo
        sec
        sbc #1
        sta varo
a02550: lda #0
        cmp varo
        bne *+5
        jmp a02716
        lda #9		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp a02644
:
        lda #2
        cmp varg
        beq *+5
        jmp a02640
        lda #100
        sta z80_c
        lda varo
        clc
        adc z80_c
        sta varo
        lda #1
        sta varg
a02640: jmp a02711
a02644: lda #1
        cmp varg
        beq *+5
        jmp a02711
        lda #100
        cmp varo
        bcc *+5
        jmp a02711
        lda #100
        sta z80_c
        lda varo
        sec
        sbc z80_c
        sta varo
        lda #2
        sta varg
a02711: jmp a02724
a02716: lda #1
        sta vard
a02724: jsr cangd	; CANGODOWN
        beq :+
        jmp a02748
:
        jsr ifall	; TABLEFALL
a02748: jmp grav
evnt01:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp b00058
:
        lda #1
        sta vard
b00058: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp b00159
        jsr cangl	; CANGOLEFT
        beq :+
        jmp b00140
:
        lda #1
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp b00154
b00140: lda #1
        ldy #11
        sta (z80_ix),y
b00154: jmp b00233
b00159: jsr cangr	; CANGORIGHT
        beq :+
        jmp b00219
:
        lda #0
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp b00233
b00219: lda #0
        ldy #11
        sta (z80_ix),y
b00233: lda #0
        cmp vara
        beq *+5
        jmp b00261
        lda #0		; ANIMATE
        jsr animsp
b00261: lda varr
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        rts
evnt02:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp c00159
:
        lda #1
        cmp varb
        beq *+5
        jmp c00151
        lda #50		; BEEP
        sta sndtyp
        lda vare
        sec
        sbc #1
        sta vare
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp c00133
        lda #1
        ldy #11
        sta (z80_ix),y
        jmp c00147
c00133: lda #0
        ldy #11
        sta (z80_ix),y
c00147: jmp c00159
c00151: lda #1
        sta vard
c00159: lda #4 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp c00277
:
        lda skptr	; OTHER
        sta z80_x
        lda skptr+1
        sta z80_i
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        lda ogptr	; ENDSPRITE
        sta z80_x
        lda ogptr+1
        sta z80_i
        lda #50		; BEEP
        sta sndtyp
        lda #2
        sta z80_c
        lda vare
        sec
        sbc z80_c
        sta vare
c00277: lda #1
        cmp vare
        bcs *+5
        jmp c00341
        lda #1
        sta varc
        lda #8	; SPAWN
        sta z80_c
        lda #5
        sta z80_b
        jsr spawn
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
c00341: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp c00442
        jsr cangl	; CANGOLEFT
        beq :+
        jmp c00424
:
        lda #12
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp c00437
c00424: lda #1
        ldy #11
        sta (z80_ix),y
c00437: jmp c00517
c00442: jsr cangr	; CANGORIGHT
        beq :+
        jmp c00503
:
        lda #11
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp c00517
c00503: lda #0
        ldy #11
        sta (z80_ix),y
c00517: lda #0
        cmp vara
        beq *+5
        jmp c00545
        lda #0		; ANIMATE
        jsr animsp
c00545: lda #4
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        rts
evnt03:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp d00159
:
        lda #1
        cmp varb
        beq *+5
        jmp d00151
        lda #50		; BEEP
        sta sndtyp
        lda varf
        sec
        sbc #1
        sta varf
        lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp d00133
        lda #1
        ldy #11
        sta (z80_ix),y
        jmp d00147
d00133: lda #0
        ldy #11
        sta (z80_ix),y
d00147: jmp d00159
d00151: lda #1
        sta vard
d00159: lda #4 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp d00277
:
        lda skptr	; OTHER
        sta z80_x
        lda skptr+1
        sta z80_i
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        lda ogptr	; ENDSPRITE
        sta z80_x
        lda ogptr+1
        sta z80_i
        lda #50		; BEEP
        sta sndtyp
        lda #2
        sta z80_c
        lda varf
        sec
        sbc z80_c
        sta varf
d00277: lda #1
        cmp varf
        bcs *+5
        jmp d00341
        lda #1
        sta varc
        lda #8	; SPAWN
        sta z80_c
        lda #5
        sta z80_b
        jsr spawn
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
d00341: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp d00442
        jsr cangl	; CANGOLEFT
        beq :+
        jmp d00424
:
        lda #14
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp d00437
d00424: lda #1
        ldy #11
        sta (z80_ix),y
d00437: jmp d00517
d00442: jsr cangr	; CANGORIGHT
        beq :+
        jmp d00503
:
        lda #13
        ldy #6
        sta (z80_ix),y
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp d00517
d00503: lda #0
        ldy #11
        sta (z80_ix),y
d00517: lda #0
        cmp vara
        beq *+5
        jmp d00545
        lda #0		; ANIMATE
        jsr animsp
d00545: lda #6
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        rts
evnt04:
        lda #18
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp e00113
        jsr cangr	; CANGORIGHT
        beq :+
        jmp e00097
:
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp e00113
e00097: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
e00113: lda #19
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp e00224
        jsr cangl	; CANGOLEFT
        beq :+
        jmp e00208
:
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp e00224
e00208: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
e00224: ldy #11
        lda (z80_ix),y
        clc
        adc #1
        ldy #11
        sta (z80_ix),y
        lda #12
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp e00289
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
e00289: rts
evnt05:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp f00228
:
        lda #1
        cmp varb
        beq *+5
        jmp f00220
        lda #50		; BEEP
        sta sndtyp
        lda varf
        sec
        sbc #1
        sta varf
        lda #1
        cmp varf
        bcs *+5
        jmp f00161
        lda #1
        sta varc
        lda #8	; SPAWN
        sta z80_c
        lda #5
        sta z80_b
        jsr spawn
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        jmp f00215
f00161: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp f00202
        lda #1
        ldy #11
        sta (z80_ix),y
        jmp f00215
f00202: lda #0
        ldy #11
        sta (z80_ix),y
f00215: jmp f00228
f00220: lda #1
        sta vard
f00228: lda #0
        ldy #11
        cmp (z80_ix),y
        beq *+5
        jmp f00315
        jsr cangl	; CANGOLEFT
        beq :+
        jmp f00297
:
        ldy #9 		; SPRITELEFT
        lda (z80_ix),y
        sec
        sbc #2
        sta (z80_ix),y
        jmp f00311
f00297: lda #1
        ldy #11
        sta (z80_ix),y
f00311: jmp f00376
f00315: jsr cangr	; CANGORIGHT
        beq :+
        jmp f00363
:
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        jmp f00376
f00363: lda #0
        ldy #11
        sta (z80_ix),y
f00376: lda #0
        cmp vara
        beq *+5
        jmp f00404
        lda #0		; ANIMATE
        jsr animsp
f00404: lda #2
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        rts
evnt06:
        lda #14
        cmp scno
        beq *+5
        jmp g00249
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        ldy #9 		; SPRITERIGHT
        lda (z80_ix),y
        clc
        adc #2
        sta (z80_ix),y
        lda #160
        ldy #9
        cmp (z80_ix),y
        bcc *+5
        jmp g00143
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        lda #80
        ldy #9
        sta (z80_ix),y
        lda #6	; SPAWN
        sta z80_c
        lda #10
        sta z80_b
        jsr spawn
g00143: lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp g00193
:
        lda #0
        cmp vara
        beq *+5
        jmp g00193
        lda #1
        sta vard
g00193: lda #0
        cmp vara
        beq *+5
        jmp g00221
        lda #0		; ANIMATE
        jsr animsp
g00221: lda varr
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        jmp g00362
g00249: lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp g00362
:
        lda #7		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp g00302
:
        jmp g00362
g00302: lda #0
        cmp vara
        beq *+5
        jmp g00362
        lda #0
        cmp varo
        bcc *+5
        jmp g00362
        lda #5		; BEEP
        sta sndtyp
        lda varo
        sec
        sbc #1
        sta varo
g00362: rts
evnt07:
        lda #7
        cmp varr
        beq *+5
        jmp h00030
        lda #0		; ANIMATE
        jsr animsp
h00030: lda #0 	; COLLISION
        sta z80_b
        jsr sktyp
        bcs :+
        jmp h00114
:
        lda #7
        cmp varr
        beq *+5
        jmp h00114
        lda #0
        cmp varo
        bcc *+5
        jmp h00114
        lda #5		; BEEP
        sta sndtyp
        lda varo
        sec
        sbc #1
        sta varo
h00114: rts
evnt08:
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0
        cmp vara
        beq *+5
        jmp i00053
        lda #0		; ANIMATE
        jsr animsp
i00053: lda #5
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp i00155
        lda varr
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #0
        cmp varc
        beq *+5
        jmp i00155
        lda #7
        and #7
        sta z80_c
        jsr cspr
        ; SPRITEINK command
        lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
i00155: lda #6
        ldy #6
        cmp (z80_ix),y
        beq *+5
        jmp i00616
        lda vard
        clc
        adc #1
        sta vard
        lda #41
        cmp vard
        bcs *+5
        jmp i00214
        jmp i00616
i00214: lda #255	; REMOVE
        ldy #5
        sta (z80_ix),y
        lda #99
        cmp varo
        bcs *+5
        jmp i00255
        lda #100
        sta varo
i00255: lda #0		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00396
:
        lda #14
        sta scno
        jsr nwscr
        lda #56
        ldy #8
        sta (z80_ix),y
        lda #56
        ldy #9
        sta (z80_ix),y
        lda #80
        sta z80_c
        lda varp
        sec
        sbc z80_c
        sta varp
        lda varn
        sec
        sbc #1
        sta varn
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #0
        jsr drpob
i00396: lda #6 	; REPEAT
        sta loopa
i00406: lda varh		; GOT
        jsr gotob
        cmp #255
        beq :+
        jmp i00508
:
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda varh
        jsr drpob
        lda varn
        sec
        sbc #1
        sta varn
        lda #30
        sta z80_c
        lda varp
        sec
        sbc z80_c
        sta varp
i00508: lda varh
        clc
        adc #1
        sta varh
        dec loopa	; ENDREPEAT
        beq :+
        jmp i00406
:
        lda #1
        sta varh
        lda numlif
        sec
        sbc #1
        sta numlif
        lda #0
        cmp numlif
        beq *+5
        jmp i00593
        lda #1		; ENDGAME
        sta gamwon
i00593: lda #0
        sta scno
        jsr nwscr
        lda #1	; RESTART
        sta restfl
i00616: rts
evnt09:
        lda #1
        ldy #5
        cmp (z80_ix),y
        beq *+5
        jmp j00037
        lda #1
        ldy #11
        sta (z80_ix),y
j00037: rts
evnt10:
        lda #150
        cmp vari
        beq *+5
        jmp k00032
        lda #0
        sta vari
        jmp k00096
k00032: lda vari
        clc
        adc #1
        sta vari
        lda #1
        cmp vari
        bcs *+5
        jmp k00096
        lda #0
        cmp varl
        bcc *+5
        jmp k00096
        lda varl
        sec
        sbc #1
        sta varl
k00096: lda #13
        sta chary
        lda #29
        sta charx
        lda varq	; DISPLAY
        jsr disply
        lda #15
        sta chary
        lda #99
        cmp varp
        bcc *+5
        jmp k00163
        lda #27
        sta charx
        jmp k00283
k00163: lda #9
        cmp varp
        bcc *+5
        jmp k00238
        lda #99
        cmp varp
        bcs *+5
        jmp k00233
        lda #27
        sta charx
        lda #0  	; MESSAGE
        jsr dmsg
        lda #15
        sta chary
        lda #28
        sta charx
k00233: jmp k00283
k00238: lda #15
        sta chary
        lda #27
        sta charx
        lda #1  	; MESSAGE
        jsr dmsg
        lda #15
        sta chary
        lda #29
        sta charx
k00283: lda varp	; DISPLAY
        jsr disply
        lda #17
        sta chary
        lda #99
        cmp varo
        bcc *+5
        jmp k00333
        lda #27
        sta charx
        jmp k00453
k00333: lda #9
        cmp varo
        bcc *+5
        jmp k00408
        lda #99
        cmp varo
        bcs *+5
        jmp k00403
        lda #27
        sta charx
        lda #0  	; MESSAGE
        jsr dmsg
        lda #17
        sta chary
        lda #28
        sta charx
k00403: jmp k00453
k00408: lda #17
        sta chary
        lda #27
        sta charx
        lda #1  	; MESSAGE
        jsr dmsg
        lda #17
        sta chary
        lda #29
        sta charx
k00453: lda varo	; DISPLAY
        jsr disply
        lda #19
        sta chary
        lda #99
        cmp varl
        bcc *+5
        jmp k00503
        lda #27
        sta charx
        jmp k00623
k00503: lda #9
        cmp varl
        bcc *+5
        jmp k00578
        lda #99
        cmp varl
        bcs *+5
        jmp k00574
        lda #27
        sta charx
        lda #0  	; MESSAGE
        jsr dmsg
        lda #19
        sta chary
        lda #28
        sta charx
k00574: jmp k00623
k00578: lda #19
        sta chary
        lda #27
        sta charx
        lda #1  	; MESSAGE
        jsr dmsg
        lda #19
        sta chary
        lda #29
        sta charx
k00623: lda varl	; DISPLAY
        jsr disply
        lda #3
        cmp vara
        beq *+5
        jmp k00664
        lda #0
        sta vara
        jmp k00680
k00664: lda vara
        clc
        adc #1
        sta vara
k00680: lda #0
        cmp varc
        bcc *+5
        jmp k00737
        lda varc
        clc
        adc #1
        sta varc
        lda #32
        cmp varc
        beq *+5
        jmp k00737
        lda #0
        sta varc
k00737: lda #9
        cmp varr
        beq *+5
        jmp k00766
        lda #1
        sta varr
        jmp k00781
k00766: lda varr
        clc
        adc #1
        sta varr
k00781: rts
evnt11:
        rts
evnt12:
        lda #0 	; PAPER
        asl a
        asl a
        asl a
        asl a
        sta tmp
        lda RegF
        and #%00001111
        ora tmp
        sta RegF
        lda #1 	; INK
        sta fontcol
        jsr cls		; CLS
        lda #7		; AT
        sta chary
        lda #1
        sta charx
        lda #3		; PRINT
        jsr dmsg
        lda #9		; AT
        sta chary
        lda #1
        sta charx
        lda #4		; PRINT
        jsr dmsg
        lda #11		; AT
        sta chary
        lda #1
        sta charx
        lda #5		; PRINT
        jsr dmsg
        lda #13		; AT
        sta chary
        lda #1
        sta charx
        lda #6		; PRINT
        jsr dmsg
        lda #15		; AT
        sta chary
        lda #1
        sta charx
        lda #7		; PRINT
        jsr dmsg
        lda #0
        sta vara
m00210: lda #0
        cmp vara
        beq *+5
        jmp m00452
        jsr prskey	; WAITKEY
        ldy #7 	; KEY
        lda keys,y
        jsr ktest
        bcc :+
        jmp m00294
:
        lda #1
        sta varb
        lda #9		; AT
        sta chary
        lda #21
        sta charx
        lda #8		; PRINT
        jsr dmsg
m00294: ldy #8 	; KEY
        lda keys,y
        jsr ktest
        bcc :+
        jmp m00355
:
        lda #1
        sta varc
        lda #11		; AT
        sta chary
        lda #21
        sta charx
        lda #9		; PRINT
        jsr dmsg
m00355: ldy #9 	; KEY
        lda keys,y
        jsr ktest
        bcc :+
        jmp m00416
:
        lda #1
        sta vard
        lda #13		; AT
        sta chary
        lda #21
        sta charx
        lda #10		; PRINT
        jsr dmsg
m00416: ldy #10 	; KEY
        lda keys,y
        jsr ktest
        bcc :+
        jmp m00448
:
        lda #1
        sta vara
m00448: jmp m00210
m00452: rts
evnt13:
        lda #0
        sta varb
        lda #0
        sta varc
        lda #0
        sta vard
        lda #2
        sta varg
        lda #1
        sta varh
        lda #1
        sta vari
        lda #150
        sta varl
        lda #0
        sta varm
        lda #0
        sta varn
        lda #100
        sta varo
        lda #0
        sta varp
        lda #0
        sta varq
        lda #0
        cmp vara
        beq *+5
        jmp n00526
        lda #7
        sta scno
        jsr nwscr
        lda #136
        ldy #8
        sta (z80_ix),y
        lda #56
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #1
        jsr drpob
        lda #1
        sta scno
        jsr nwscr
        lda #112
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #2
        jsr drpob
        lda #15
        sta scno
        jsr nwscr
        lda #64
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #3
        jsr drpob
        lda #8
        sta scno
        jsr nwscr
        lda #144
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #4
        jsr drpob
        lda #10
        sta scno
        jsr nwscr
        lda #80
        ldy #8
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #5
        jsr drpob
        lda #9
        sta scno
        jsr nwscr
        lda #24
        ldy #8
        sta (z80_ix),y
        lda #88
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #6
        jsr drpob
n00526: lda #1
        cmp vara
        beq *+5
        jmp n00967
        lda #2
        sta scno
        jsr nwscr
        lda #136
        ldy #8
        sta (z80_ix),y
        lda #48
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #1
        jsr drpob
        lda #7
        sta scno
        jsr nwscr
        lda #56
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #2
        jsr drpob
        lda #13
        sta scno
        jsr nwscr
        lda #88
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #3
        jsr drpob
        lda #12
        sta scno
        jsr nwscr
        lda #16
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #4
        jsr drpob
        lda #3
        sta scno
        jsr nwscr
        lda #24
        ldy #8
        sta (z80_ix),y
        lda #80
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #5
        jsr drpob
        lda #10
        sta scno
        jsr nwscr
        lda #80
        ldy #8
        sta (z80_ix),y
        lda #144
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #6
        jsr drpob
n00967: lda #2
        cmp vara
        beq *+5
        jmp n01382
        lda #4
        sta scno
        jsr nwscr
        lda #24
        ldy #8
        sta (z80_ix),y
        lda #80
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #1
        jsr drpob
        lda #2
        sta scno
        jsr nwscr
        lda #144
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #2
        jsr drpob
        lda #11
        sta scno
        jsr nwscr
        lda #104
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #3
        jsr drpob
        lda #136
        ldy #8
        sta (z80_ix),y
        lda #16
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #4
        jsr drpob
        lda #6
        sta scno
        jsr nwscr
        lda #144
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #5
        jsr drpob
        lda #15
        sta scno
        jsr nwscr
        lda #64
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #6
        jsr drpob
n01382: lda #3
        cmp vara
        beq *+5
        jmp n01783
        lda #3
        sta scno
        jsr nwscr
        lda #24
        ldy #8
        sta (z80_ix),y
        lda #80
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #1
        jsr drpob
        lda #4
        sta scno
        jsr nwscr
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #2
        jsr drpob
        lda #11
        sta scno
        jsr nwscr
        lda #144
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #3
        jsr drpob
        lda #8
        sta scno
        jsr nwscr
        lda #136
        ldy #8
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #4
        jsr drpob
        lda #7
        sta scno
        jsr nwscr
        lda #56
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #5
        jsr drpob
        lda #13
        sta scno
        jsr nwscr
        lda #88
        ldy #9
        sta (z80_ix),y
        ldy #9
        lda (z80_ix),y	; PUT
        sta dispx
        ldy #8
        lda (z80_ix),y
        sta dispy
        lda #6
        jsr drpob
n01783: lda #0
        sta scno
        jsr nwscr
        lda #1
        cmp varb
        beq *+5
        jmp n01861
        lda #7	; GET
        jsr getob
        lda varn
        clc
        adc #1
        sta varn
        lda #20
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
n01861: lda #1
        cmp vard
        beq *+5
        jmp n01928
        lda #8	; GET
        jsr getob
        lda varn
        clc
        adc #1
        sta varn
        lda #20
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
n01928: lda #1
        cmp varc
        beq *+5
        jmp n01994
        lda #9	; GET
        jsr getob
        lda varn
        clc
        adc #1
        sta varn
        lda #20
        sta z80_c
        lda varp
        clc
        adc z80_c
        sta varp
n01994: rts
evnt14:
        lda #0
        sta vara
        lda #0
        sta varb
        lda #0
        sta varc
        lda #0
        sta vard
        lda #5
        sta vare
        lda #5
        sta varf
        lda #1
        sta varr
        lda #11
        sta chary
        lda #29
        sta charx
        lda numlif	; DISPLAY
        jsr disply
        rts
evnt15:
        rts
evnt16:
        lda numlif
        sec
        sbc #1
        sta numlif
        rts
evnt17:
        rts
evnt18:
        lda #10		; AT
        sta chary
        lda #2
        sta charx
        lda #11		; PRINT
        jsr dmsg
        lda #11		; AT
        sta chary
        lda #2
        sta charx
        lda #12		; PRINT
        jsr dmsg
        lda #12		; AT
        sta chary
        lda #2
        sta charx
        lda #13		; PRINT
        jsr dmsg
        lda #20 	; DELAY
        jsr delay
        jsr prskey	; WAITKEY
        rts
evnt19:
        rts
evnt20:
        rts
ptcusr: rts
data_start:
msgdat:
        .byte "#",138
        .byte "##",138
        .byte "#GEM
#ANKH
#FIGURINE
#VASE
#NECKLACE
#MASK
#SWORD
#SUIT
#SHOTGUN
#CYLINDER
#*#EXIT#*#",138
        .byte "EQUIPMENT SELECTIO",206
        .byte "1. PROTECTIVE SUIT",197
        .byte "2. OXYGEN CYLINDE",210
        .byte "3. PUMP-ACTION GU",206
        .byte "4. START GAM",197
        .byte 216
        .byte 216
        .byte 216
        .byte "  CONGRATULATIONS!  ",160
        .byte "   level completed  ",160
        .byte " continue to level 3",160
nummsg:
        .byte 14
chgfx:
        .byte 0,0,0,0,0,0,0,0
        .byte 0,245,106,181,242,85,162,0
        .byte 0,203,150,8,179,42,21,2
        .byte 0,61,105,52,158,200,129,0
        .byte 0,239,69,146,57,148,72,128
        .byte 125,226,197,104,80,32,0,0
        .byte 107,210,225,178,90,20,8,0
        .byte 215,169,99,86,8,0,0,0
        .byte 92,186,34,112,176,148,89,28
        .byte 237,169,208,98,80,98,52,32
        .byte 203,242,106,50,50,20,28,8
        .byte 25,21,10,10,12,4,0,0
        .byte 52,24,8,8,0,0,0,0
        .byte 62,21,10,0,40,84,58,116
        .byte 104,18,79,62,53,42,52,18
        .byte 79,151,74,228,96,8,60,58
        .byte 124,122,116,40,17,70,125,58
        .byte 13,78,165,240,230,78,29,30
        .byte 40,20,0,102,94,42,68,112
        .byte 244,250,255,126,21,42,80,124
        .byte 50,7,81,234,240,232,210,175
        .byte 96,64,96,79,96,64,96,64
        .byte 6,4,6,244,6,4,6,4
        .byte 0,4,64,0,0,2,16,0
        .byte 255,255,255,255,255,255,255,255
        .byte 96,64,96,79,96,64,96,64
        .byte 6,4,6,244,6,4,6,4
        .byte 31,16,32,32,64,64,136,132
        .byte 255,0,0,0,0,0,0,0
        .byte 255,255,255,255,255,255,255,255
        .byte 240,208,176,176,240,240,112,240
        .byte 0,3,4,23,20,23,7,6
        .byte 255,0,254,0,0,0,126,255
        .byte 166,185,176,208,208,232,244,252
        .byte 0,0,0,62,30,12,0,1
        .byte 0,240,242,151,181,247,245,247
        .byte 18,28,28,0,0,0,0,0
        .byte 60,66,153,165,165,153,66,60
        .byte 255,85,106,0,0,0,0,0
        .byte 255,86,170,0,0,0,0,0
        .byte 117,114,0,0,0,0,0,0
        .byte 0,11,22,22,45,41,89,181
        .byte 0,255,255,255,127,127,191,191
        .byte 0,255,255,255,255,255,255,255
        .byte 0,168,112,252,246,240,255,253
        .byte 0,1,1,2,5,5,11,11
        .byte 181,109,109,221,220,188,188,125
        .byte 223,223,239,239,247,247,123,123
        .byte 128,128,128,192,64,192,160,224
        .byte 15,4,8,8,16,80,96,32
        .byte 124,124,124,124,124,124,124,124
        .byte 188,61,189,77,45,85,37,13
        .byte 240,32,32,16,20,12,8,8
        .byte 8,41,146,84,179,40,90,53
        .byte 32,73,81,74,42,82,109,149
        .byte 254,252,249,250,240,224,195,128
        .byte 127,63,31,15,3,0,0,0
        .byte 254,252,249,194,152,32,128,0
        .byte 255,255,255,231,211,165,0,0
        .byte 127,63,15,7,3,1,0,0
        .byte 255,255,255,255,255,255,255,255
        .byte 255,255,254,233,192,168,195,255
        .byte 213,138,65,0,17,132,203,255
        .byte 255,255,81,130,8,37,222,255
        .byte 255,255,255,191,83,40,151,255
        .byte 0,0,0,0,0,0,0,0
        .byte 0,0,0,1,2,5,14,12
        .byte 229,139,94,40,113,226,196,138
        .byte 60,234,198,56,224,224,64,192
        .byte 229,207,214,172,156,56,81,50
        .byte 16,32,16,66,161,65,4,66
        .byte 128,128,0,0,128,128,64,176
        .byte 0,0,0,0,0,2,51,24
        .byte 30,7,3,1,97,48,130,61
        .byte 96,56,128,220,114,197,97,209
        .byte 1,3,2,3,6,4,6,12
        .byte 98,229,66,200,128,0,19,40
        .byte 8,1,70,49,10,84,34,205
        .byte 100,42,145,90,36,100,174,114
        .byte 56,128,11,164,13,23,15,31
        .byte 250,244,186,85,168,0,0,0
        .byte 247,242,236,228,232,232,209,217
        .byte 32,80,65,164,128,10,5,11
        .byte 200,119,154,8,21,42,86,176
        .byte 158,141,86,99,186,142,199,99
        .byte 224,224,112,248,212,250,222,229
        .byte 178,178,161,180,100,64,105,72
        .byte 104,210,161,192,164,9,148,46
        .byte 244,210,139,212,40,166,85,170
        .byte 57,18,8,196,83,33,19,143
        .byte 117,61,6,1,0,0,0,240
        .byte 0,1,1,1,1,2,2,2
        .byte 174,104,44,116,73,210,65,144
        .byte 169,214,169,84,195,21,168,20
        .byte 255,173,83,175,28,122,240,224
        .byte 249,240,136,64,4,160,50,96
        .byte 103,23,27,11,3,163,7,7
        .byte 2,4,5,4,5,11,10,23
        .byte 165,194,166,203,150,73,151,34
        .byte 201,131,23,162,84,40,144,40
        .byte 128,16,1,128,4,1,10,149
        .byte 105,180,24,52,58,116,250,248
        .byte 240,112,240,224,224,192,64,128
        .byte 22,44,42,85,184,32,65,7
        .byte 133,35,1,40,20,88,181,218
        .byte 125,170,85,42,20,193,79,248
        .byte 244,168,80,224,128,128,0,0
        .byte 244,232,232,213,176,168,112,105
        .byte 128,0,0,128,192,252,162,85
        .byte 255,130,32,0,1,128,5,170
        .byte 255,43,69,11,5,11,85,171
        .byte 254,212,168,0,239,77,138,0
        .byte 255,7,92,90,88,93,92,255
        .byte 250,192,100,160,36,96,100,242
        .byte 214,161,166,160,160,160,160,0
        .byte 32,224,36,224,228,224,228,242
        .byte 255,223,255,255,255,253,255,255
        .byte 96,64,96,79,96,64,96,64
        .byte 6,4,6,244,6,4,6,4
        .byte 0,0,0,0,0,0,0,0
bCol:
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
        .byte 1
bprop:
        .byte 0
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 3
        .byte 3
        .byte 2
        .byte 5
        .byte 3
        .byte 3
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 0
        .byte 6
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 2
        .byte 6
        .byte 3
        .byte 3
        .byte 0
sprgfx:
        .byte 5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,48,8,119,108,6,160,103,230,0,0,3,96,3,176
        .byte 0,0,5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,48,0,119,104,6,160,55,236,0,0,7,56
        .byte 5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,24,0,29,96,0,160,6,224,0,0,12,24,14,28
        .byte 3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,16,12,54,238,5,96,103,230,0,0,6,192,13,192
        .byte 0,0,3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,12,22,238,5,96,55,236,0,0,28,224
        .byte 3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,24,6,184,5,0,7,96,0,0,24,48,56,112
        .byte 5,224,11,240,23,248,23,248,0,0,5,84,10,160,47,244,7,228,32,12,55,232,4,32,3,192,12,0,28,48,0,56
        .byte 0,0,5,224,11,240,23,248,23,248,0,0,42,164,15,240,39,228,48,12,7,224,4,32,3,192,12,48,28,56,0,0
        .byte 5,224,11,240,23,248,23,248,0,0,37,80,10,160,47,244,39,224,48,4,23,236,4,32,3,192,0,48,12,56,28,0
        .byte 5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,48,0,119,96,0,28,111,238,0,28,6,96,7,112
        .byte 0,0,5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,48,0,119,96,0,28,55,238,0,28,7,96
        .byte 2,224,5,244,11,234,11,238,0,0,7,248,7,104,7,104,3,240,24,0,29,96,0,14,54,247,0,14,6,96,7,112
        .byte 3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,12,6,238,56,0,119,246,56,0,6,96,14,224
        .byte 0,0,3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,12,6,238,56,0,119,236,56,0,6,224
        .byte 7,64,47,160,87,208,119,208,0,0,31,224,22,224,22,224,15,192,0,24,6,184,112,0,239,108,112,0,6,96,14,224
        .byte 0,0,0,0,0,128,0,128,9,144,5,96,6,64,8,60,60,16,2,96,6,160,9,144,1,0,1,0,0,0,0,0
        .byte 0,128,64,130,49,140,61,188,18,72,21,168,10,80,244,44,52,47,10,80,21,168,18,72,61,188,49,140,65,2,1,0
        .byte 5,224,10,112,20,184,21,56,0,0,47,244,13,176,45,180,39,228,48,12,22,104,5,160,3,192,0,0,12,48,28,56
        .byte 0,0,0,0,3,160,134,12,192,226,205,237,21,46,25,227,25,233,21,37,205,227,192,238,134,12,3,160,0,0,0,0
        .byte 0,0,0,0,5,192,48,97,119,3,247,179,196,168,167,152,151,152,68,168,183,179,71,3,48,97,5,192,0,0,0,0
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        .byte 255,0,182,206,187,250,239,211,87,255,58,100,14,0,7,90,1,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        .byte 182,206,187,250,239,179,87,255,122,100,30,0,15,0,3,0,1,128,0,224,0,120,0,0,0,0,0,0,0,0,0,0
        .byte 64,0,160,0,240,0,120,0,184,0,92,0,52,0,30,0,30,0,54,0,103,0,13,0,25,0,51,0,2,0,4,0
        .byte 64,0,188,0,251,128,127,80,191,232,85,252,2,124,0,118,0,210,1,146,1,52,0,32,0,0,0,0,0,0,0,0
        .byte 0,0,0,0,0,128,6,0,28,232,123,208,237,14,128,3,200,6,118,152,59,98,12,16,2,128,0,0,0,0,0,0
        .byte 0,0,0,0,1,0,6,4,13,184,63,240,231,28,128,39,192,6,109,204,51,240,25,128,4,32,0,0,0,0,0,0
        .byte 0,0,0,12,0,28,0,18,7,218,56,127,64,227,65,113,38,168,25,164,4,146,31,18,18,16,12,2,0,16,0,0
        .byte 0,12,127,28,128,146,128,90,72,255,55,99,0,209,3,168,5,164,14,146,23,18,25,16,14,2,18,16,12,0,0,0
        .byte 0,0,48,0,56,0,72,0,91,224,254,28,199,2,142,130,21,100,37,152,73,32,72,248,8,72,64,48,8,0,0,0
        .byte 48,0,56,254,73,1,90,1,255,18,198,236,139,0,21,192,37,160,73,112,72,232,8,152,64,112,8,72,0,48,0,0
        .byte 15,0,4,0,15,0,7,0,0,86,28,2,1,88,30,8,0,0,15,128,0,0,13,128,0,0,8,128,16,0,28,224
        .byte 1,224,0,128,1,224,0,224,0,0,3,136,0,100,7,27,0,9,7,192,0,0,12,96,0,0,24,32,32,0,56,56
        .byte 0,240,0,32,0,240,0,224,106,0,64,56,26,128,16,120,0,0,1,240,0,0,1,176,0,0,1,16,0,8,7,56
        .byte 7,128,1,0,7,128,7,0,0,0,17,192,38,0,216,224,144,0,3,224,0,0,6,48,0,0,4,24,0,4,28,28
        .byte 5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,16,0,39,255,48,28,12,192,0,0,24,96,28,112
        .byte 5,192,11,232,23,212,23,220,0,0,15,240,14,208,14,208,7,224,16,0,39,255,49,132,12,48,0,0,24,96,28,112
        .byte 23,0,47,160,95,80,95,112,0,0,63,192,59,64,59,64,31,128,64,0,159,252,198,16,48,192,0,0,24,96,28,112
        .byte 3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,8,255,228,56,12,3,48,0,0,6,24,14,56
        .byte 3,160,23,208,43,232,59,232,0,0,15,240,11,112,11,112,7,224,0,8,255,228,33,140,12,48,0,0,6,24,14,56
        .byte 0,232,5,244,10,250,14,250,0,0,3,252,2,220,2,220,1,248,0,2,63,249,8,99,3,12,0,0,6,24,14,56
        .byte 0,0,3,192,7,32,15,208,15,240,9,144,13,176,15,240,70,98,70,98,35,196,31,248,11,208,82,74,36,36,8,16
        .byte 0,0,3,192,7,32,15,208,15,240,9,144,15,240,15,240,6,96,7,224,99,198,159,249,11,208,50,76,66,66,4,32
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,0
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,192,0,0,0,0,0,0,0,0,0,0,0
frmlst:
        .byte 0,3
        .byte 3,3
        .byte 6,3
        .byte 9,3
        .byte 12,3
        .byte 15,2
        .byte 17,3
        .byte 20,1
        .byte 21,2
        .byte 23,2
        .byte 25,2
        .byte 27,2
        .byte 29,2
        .byte 31,2
        .byte 33,2
        .byte 35,3
        .byte 38,3
        .byte 41,2
        .byte 43,1
        .byte 44,1,45,0
scdat:
        .word 205,303,323,261,293,354,347,327,371,347,311,359,339,372,303,324
        .byte 255,60,54,61,62,63,64,255,60,32,61,62,63,62,63,64,255,60,10,61,62,64,255,60,16,58,255,60,12,58,255,60,7
        .byte 55,65,59,255,60,10,55,0,59,57,56,58,60,60,55,65,65,65,59,255,60,5,55,59,58,55,255,0,6,59,57,65,65
        .byte 0,0,0,59,57,59,60,57,255,0,20,59,255,0,47,27,28,29,30,255,0,9,41,42,43,44,0,0,0,31,32,33,34
        .byte 32,35,255,0,7,65,45,46,47,29,29,48,0,0,36,37,38,39,37,40,255,0,7,65,49,50,51,43,43,52,0,53,54
        .byte 53,54,53,54,53,54,53,54,21,22,53,54,53,54,53,54,53,54,53,54,1,2,3,4,1,2,3,4,4,2,21,22,2
        .byte 3,4,1,2,3,4,1,2,3,255,23,9,13,21,22,17,255,23,18,14,21,22,18,255,23,18,15,21,22,19,255,23,18
        .byte 16,21,22,20,255,23,9
        .byte 255,23,26,13,5,6,9,7,5,6,5,7,10,5,9,8,6,5,7,6,17,255,23,4,14,0,0,12,255,0,7,12,11
        .byte 255,0,4,18,255,23,4,15,255,0,16,19,255,23,4,16,255,0,16,20,255,23,4,2,3,4,21,22,3,4,1,2,3
        .byte 4,1,2,21,22,1,2,3,255,23,6,13,21,22,17,255,23,6,13,21,22,17,255,23,8,14,21,22,18,255,23,6,14
        .byte 21,22,18,23,23,13,10,7,6,7,5,10,21,22,7,10,19,23,23,8,7,5,21,22,10,7,6,14,255,0,6,21,22
        .byte 0,0,20,23,23,14,0,0,21,22,0,0,0,15,255,0,6,21,22,0,0,17,23,23,15,0,0,21,22,0,0,0,16
        .byte 255,0,6,21,22,0,0,18,23,23,15,0,0,21,22,0,0,0,3,4,1,21,22,4,1,2,3,4,1,2,23,23,2
        .byte 1,3,4,2,1,3,4,23,23,13,21,22,17,255,23,18,14,21,22,18,255,23,18,15,21,22,19,255,23,7,13,5,10
        .byte 7,5,6,10,6,5,23,23,16,21,22,20,255,23,7,14,255,116,8,23,23,13,21,22,17,255,23,7,15,255,116,8,23,23
        .byte 14,21,22,18,255,23,7,16,255,116,8,23,23,15,21,22,19,255,23,7,2,3,4,1,2,3,4,1,2,23,23,16,21
        .byte 22,20,255,23,16
        .byte 255,23,9,13,21,22,17,255,23,12,13,8,7,6,9,7,5,21,22,7,10,17,23,23,13,5,7,10,7,23,23,23,14
        .byte 11,0,0,12,0,0,21,22,0,0,18,23,23,14,255,0,4,23,23,23,15,255,0,6,21,22,0,0,19,23,23,15,255,0,4
        .byte 23,23,23,16,255,0,6,21,22,0,0,20,23,23,16,255,0,4,23,23,23,1,2,3,21,22,2,3,4,1,2,3,4
        .byte 23,23,3,4,1,2,3,255,23,5,13,21,22,17,255,23,18,14,21,22,18,255,23,13,7,10,7,6,7,5,21,22,5
        .byte 8,10,9,10,7,7,6,7,7,9,10,17,23,255,0,6,21,22,0,11,0,12,255,0,6,12,0,18,23,255,0,6,21
        .byte 22,255,0,12,19,23,255,0,6,21,22,255,0,12,20,23,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,21
        .byte 22,2,1,3,255,23,16,13,21,22,17,255,23,18,14,21,22,18,23,23,23,5,6,7,5,7,6,7,5,10,6,17,255,23,4
        .byte 15,21,22,19,23,23,23,255,116,10,18,255,23,4,16,21,22,20,23,23,23,255,116,8,112,113,19,255,23,4,13,21,22
        .byte 17,23,23,23,255,116,8,114,115,20,255,23,4,14,21,22,18,23,23,23,1,2,21,22,1,2,3,4,1,2,3,255,23,4
        .byte 15,21,22,19,255,23,4,13,21,22,17,255,23,10,16,21,22,20,23,23,23
        .byte 255,23,22,6,5,7,10,8,7,5,7,9,10,7,6,17,255,23,9,255,0,4,11,0,0,0,12,0,0,0,18,255,23,9
        .byte 255,0,12,18,255,23,9,255,0,12,19,255,23,9,2,3,4,1,21,22,4,1,2,3,4,1,4,255,23,12,20,21,22
        .byte 18,255,23,18,17,21,22,19,255,23,16,15,7,6,21,22,10,6,5,7,10,5,7,10,6,7,5,10,7,6,5,17,23
        .byte 16,116,116,117,118,255,116,15,18,23,17,112,113,117,118,255,116,15,19,23,18,114,115,117,118,255,116,15,20,23,4,1,2
        .byte 3,4,1,2,3,4,1,2,3,4,1,2,3,21,22,1,2,3,255,23,16,16,21,22,20,255,23,18,13,21,22,17,23,23
        .byte 17,6,7,8,9,6,7,5,10,6,7,8,6,7,5,10,7,21,22,10,6,17,18,0,0,11,12,255,0,6,11,255,0,5
        .byte 21,22,0,0,18,19,255,0,16,21,22,0,0,19,20,255,0,16,21,22,0,0,20,1,2,21,22,1,2,3,4,1,2
        .byte 3,21,22,2,3,4,1,2,3,4,1,2,23,14,21,22,15,255,23,5,13,21,22,14,255,23,8
        .byte 23,23,16,21,22,17,255,23,18,13,21,22,18,23,23,23,8,7,5,7,10,5,9,8,6,5,7,10,7,23,23,14,21
        .byte 22,19,23,23,23,14,255,0,5,12,11,255,0,5,23,23,15,21,22,20,23,23,23,15,255,0,12,23,23,16,21,22,17
        .byte 23,23,23,16,255,0,12,23,23,13,21,22,18,23,23,23,2,4,1,2,3,21,22,2,3,4,1,2,3,23,23,14,21
        .byte 22,19,255,23,7,14,21,22,15,255,23,7,15,21,22,20,255,23,7,15,21,22,16,255,23,5,13,10,7,21,22,5,10
        .byte 6,5,7,10,6,10,6,21,22,7,6,17,23,23,23,14,24,24,25,26,255,24,9,25,26,24,24,18,23,23,23,15,24,24
        .byte 25,26,255,24,9,25,26,24,24,19,23,23,23,16,24,24,25,26,255,24,9,25,26,24,24,20,23,23,23,3,4,1,2
        .byte 3,4,1,2,3,4,1,2,3,4,1,2,3,4,2,255,23,52,13,7,5,10,6,7,8,6,7,5,10,7,9,6,10
        .byte 6,5,255,23,5,14,255,0,5,11,255,0,5,12,255,0,4,255,23,5,15,255,0,16,255,23,5,16,255,0,16,255,23,5
        .byte 2,3,4,21,22,3,4,1,2,3,4,1,2,3,4,1,2,255,23,7,17,21,22,14,255,23,11
        .byte 23,14,21,22,18,255,23,10,16,21,22,17,23,23,23,6,5,21,22,8,7,6,9,7,5,17,255,23,4,13,21,22,5
        .byte 7,10,7,0,0,21,22,11,0,0,12,0,0,18,255,23,4,14,25,26,255,24,4,0,0,21,22,255,0,6,19,255,23,4
        .byte 15,25,26,255,24,4,0,0,21,22,255,0,6,20,255,23,4,16,25,26,255,24,4,2,3,4,1,2,3,21,22,2,3
        .byte 4,255,23,4,1,2,3,4,1,2,3,255,23,5,14,21,22,18,255,23,18,15,21,22,19,255,23,13,17,10,7,6,7
        .byte 5,21,22,5,8,10,9,10,7,7,6,7,7,9,10,14,23,18,255,0,5,21,22,0,11,0,12,255,0,6,12,0,15
        .byte 23,19,255,0,5,21,22,255,0,12,16,23,20,255,0,5,21,22,255,0,12,13,23,3,4,1,2,3,4,21,22,3,4
        .byte 1,2,3,4,1,2,21,22,1,2,3,255,23,6,15,21,22,17,255,23,6,14,21,22,19,255,23,8,16,21,22,18,255,23,6
        .byte 15,21,22,20,23,23,23,5,6,7,8,9,6,21,22,10,6,7,8,6,17,23,13,21,22,17,23,23,23,0,0,0,11
        .byte 12,0,21,22,0,0,0,11,0,18,23,14,21,22,18,23,23,23,255,0,6,21,22,255,0,5,19,23,15,21,22,19,23,23,23
        .byte 255,0,6,21,22,255,0,5,20,23,16,21,22,20,23,23,23,1,2,3,4,1,2,3,4,1,2,21,22,1,2,23,13
        .byte 21,22,3,255,23,12,15,21,22,19,23,23,14,21,22,20,23,23,23
        .byte 23,16,21,22,20,255,23,5,15,21,22,19,255,23,8,6,5,21,22,10,7,17,23,23,23,16,21,22,20,255,23,8,24,24
        .byte 25,26,24,24,18,23,23,23,13,21,22,17,255,23,8,24,24,25,26,24,24,19,23,23,23,14,21,22,18,255,23,8,24,24
        .byte 25,26,24,24,20,23,23,23,15,21,22,19,255,23,8,2,3,4,1,2,3,4,23,23,23,16,21,22,20,255,23,18,13
        .byte 21,22,17,255,23,18,14,21,22,18,255,23,9,13,7,6,7,5,10,6,5,8,10,21,22,7,7,6,7,7,9,17,23,23,23
        .byte 14,255,0,7,11,0,21,22,255,0,5,12,18,23,23,23,15,255,0,9,21,22,255,0,6,19,23,23,23,16,255,0,9
        .byte 21,22,255,0,6,20,23,23,23,13,21,22,3,4,1,21,22,4,1,2,3,4,1,2,3,4,2,1,23,23,23,14,21
        .byte 22,17,23,15,21,22,19,255,23,13,15,21,22,18,23,16,21,22,20,255,23,13,16,21,22,19,23,13,21,22,17,23,23,23
        .byte 13,5,10,7,9,6,10,17,23,23,13,21,22,20,23,14,21,22,18,23,23,23,14,0,0,0,12,0,0,18,23,23,14
        .byte 21,22,17,23,15,21,22,19,23,23,23,15,255,0,6,19,23,23,15,21,22,18,23,16,21,22,20,23,23,23,16,255,0,6
        .byte 20,23,23,16,21,22,19,23,13,21,22,17,23,23,23,2,3,4,21,22,3,4,1,23,23,13,21,22,20,23,14,21,22
        .byte 18,255,23,5,15,21,22,20,23,23,23
        .byte 255,23,7,16,21,22,20,255,23,11,15,5,7,10,8,7,6,9,21,22,6,5,7,10,5,9,8,6,17,23,23,23,16
        .byte 0,0,0,11,0,0,12,21,22,255,0,5,12,11,0,18,23,23,23,13,255,0,7,21,22,255,0,8,19,23,23,23,14
        .byte 255,0,7,21,22,255,0,8,20,23,23,23,2,3,4,1,2,3,4,1,2,3,4,1,2,3,21,22,2,3,4,255,23,16
        .byte 13,21,22,18,255,23,18,14,21,22,19,255,23,5,14,10,7,6,7,5,10,6,5,6,10,5,10,7,21,22,7,6,7
        .byte 10,19,23,15,255,116,13,117,118,255,116,4,20,23,16,255,116,13,117,118,116,116,112,113,17,23,13,255,116,13,117,118,116,116
        .byte 114,115,18,23,3,4,1,21,22,4,1,2,3,4,1,2,3,4,21,22,3,4,2,1,3,23,23,23,16,21,22,18,255,23,7
        .byte 15,21,22,17,255,23,7,13,21,22,19,255,23,7,16,21,22,18,255,23,5,13,6,7,21,22,6,7,5,10,17,23,23,23
        .byte 13,21,22,19,255,23,5,14,0,0,21,22,255,0,4,18,23,23,23,14,21,22,20,255,23,5,15,0,0,21,22,255,0,4
        .byte 19,23,23,23,15,21,22,17,255,23,5,16,0,0,21,22,255,0,4,20,23,23,23,16,21,22,18,255,23,5,1,2,3
        .byte 4,1,2,3,4,1,2,23,23,23,13,21,22,19,255,23,18,14,21,22,20,255,23,5
        .byte 255,23,9,16,21,22,20,23,23,13,21,22,17,23,23,23,15,5,7,10,8,7,6,9,7,5,21,22,7,17,23,14,21
        .byte 22,18,23,23,23,16,0,0,0,11,0,0,12,0,0,21,22,0,18,23,15,21,22,19,23,23,23,13,255,0,9,21,22
        .byte 0,19,23,16,21,22,20,23,23,23,14,255,0,9,21,22,0,20,23,13,21,22,17,23,23,23,2,3,4,21,22,3,4
        .byte 1,2,3,4,1,2,3,23,14,21,22,18,255,23,5,13,21,22,17,255,23,9,15,21,22,19,255,23,5,14,21,22,18
        .byte 255,23,9,16,21,22,20,255,23,5,15,21,22,18,23,17,10,7,5,6,10,7,5,6,21,22,17,255,23,5,16,21,22
        .byte 19,23,18,255,116,8,117,118,18,255,23,5,13,21,22,17,23,19,112,113,255,116,6,117,118,19,255,23,5,14,21,22,18
        .byte 23,20,114,115,255,116,6,117,118,20,255,23,5,15,21,22,19,23,2,3,4,21,22,3,4,1,2,3,4,2,255,23,5
        .byte 16,21,22,20,23,23,23,15,21,22,19,255,23,11,13,21,22,17,23,23,23,16,21,22,20,255,23,11,14,21,22,18,23
        .byte 20,10,6,21,22,6,7,5,10,7,9,6,10,17,23,23,23,15,21,22,19,23,17,0,0,21,22,255,0,5,12,0,0
        .byte 18,23,23,23,16,21,22,20,23,18,0,0,21,22,255,0,8,19,23,23,23,13,21,22,17,23,19,0,0,21,22,255,0,8
        .byte 20,23,23,23,14,21,22,18,23,4,1,2,21,22,1,2,3,4,1,2,3,4,1,23,23,23,15,21,22,19,23,23,23
        .byte 14,21,22,18,255,23,9
        .byte 23,14,21,22,18,23,16,21,22,20,255,23,5,13,21,22,17,255,23,4,15,21,22,19,23,13,21,22,5,6,5,7,17
        .byte 23,14,21,22,18,255,23,4,16,21,22,20,23,14,21,22,255,0,4,18,23,15,21,22,19,255,23,4,13,21,22,17,23
        .byte 15,21,22,255,0,4,19,23,16,21,22,20,255,23,4,14,21,22,18,23,16,21,22,255,0,4,20,23,13,21,22,17,255,23,4
        .byte 15,21,22,19,23,4,1,2,3,4,1,2,3,23,14,21,22,18,255,23,4,16,21,22,20,255,23,10,15,21,22,19,255,23,4
        .byte 13,21,22,17,255,23,10,16,21,22,20,255,23,4,14,21,22,7,5,10,6,5,8,10,9,10,7,7,6,21,22,9,10
        .byte 17,23,23,15,21,22,255,0,5,11,0,12,255,0,4,21,22,12,0,18,23,23,16,21,22,255,0,12,21,22,0,0,19
        .byte 23,23,13,21,22,255,0,12,21,22,0,0,20,23,23,4,1,2,3,4,1,2,3,4,1,2,21,22,1,2,3,4,2
        .byte 1,3,255,23,12,14,21,22,18,255,23,18,15,21,22,19,255,23,9,16,8,9,6,7,5,10,6,7,8,21,22,5,10
        .byte 7,9,6,20,255,23,4,13,11,12,255,0,6,11,21,22,0,0,0,12,0,17,255,23,4,14,255,0,9,21,22,255,0,5
        .byte 18,255,23,4,15,255,0,9,21,22,255,0,5,19,255,23,4,3,4,1,2,3,4,21,22,3,4,1,2,3,4,1,2
        .byte 3,4,255,23,9,13,21,22,17,255,23,11
        .byte 255,23,13,15,21,22,18,255,23,10,18,6,9,7,5,6,5,7,10,21,22,8,6,14,255,23,8,19,0,12,255,0,6
        .byte 21,22,11,0,15,255,23,8,20,255,0,8,21,22,0,0,16,255,23,8,17,255,0,8,21,22,0,0,13,255,23,8,3
        .byte 4,1,21,22,4,1,2,3,4,1,2,3,4,255,23,10,14,21,22,19,255,23,18,13,21,22,20,255,23,11,15,10,7
        .byte 6,7,5,10,6,21,22,10,9,10,7,7,19,23,16,9,10,7,6,16,255,0,7,21,22,0,12,0,0,0,20,23,13
        .byte 12,0,0,0,13,255,0,7,21,22,255,0,5,17,23,14,255,0,4,14,255,0,7,21,22,255,0,5,18,23,15,255,0,4
        .byte 3,4,21,22,3,4,1,2,3,4,1,2,3,4,1,2,23,4,2,1,3,4,23,14,21,22,18,255,23,18,15,21,22
        .byte 19,255,23,18,16,21,22,20,23,13,5,10,6,7,8,6,7,5,10,7,9,6,10,17,23,23,13,21,22,17,23,14,255,0,4
        .byte 11,255,0,5,12,0,0,18,23,23,14,21,22,18,23,15,255,0,13,19,23,23,15,21,22,19,23,15,255,0,13,20,23,23
        .byte 16,21,22,20,23,2,4,1,21,22,4,1,2,3,4,21,22,3,4,1,23,23,13,21,22,17,23,23,23,13,21,22,17
        .byte 23,23,23,16,21,22,20,23,23,23
        .byte 23,23,14,21,22,20,23,23,23,16,21,22,20,255,23,11,15,21,22,17,23,19,7,5,21,22,7,10,5,13,23,19,5
        .byte 7,10,7,23,23,16,21,22,18,23,20,24,24,25,26,24,24,24,14,23,20,255,0,4,23,23,13,21,22,19,23,17,24,24
        .byte 25,26,24,24,24,15,23,17,255,0,4,23,23,14,21,22,17,23,18,24,24,25,26,24,24,24,16,23,18,255,0,4,23,23
        .byte 15,21,22,18,23,1,2,3,4,1,2,3,4,1,23,3,4,1,2,3,23,23,16,21,22,19,255,23,18,13,21,22,20
        .byte 255,23,16,7,10,7,21,22,5,10,6,5,8,10,9,18,23,23,15,7,7,9,10,7,6,0,0,0,21,22,255,0,4
        .byte 11,0,12,19,23,23,16,0,0,12,255,0,6,21,22,255,0,7,20,23,23,13,255,0,9,21,22,255,0,7,17,23,23
        .byte 14,255,0,6,3,4,1,2,3,4,1,2,21,22,1,2,3,23,23,2,3,4,21,22,3,4,255,23,7,15,21,22,17
        .byte 255,23,6,14,21,22,18,255,23,8,16,21,22,18,255,23,6,15,21,22,19,23,23,14,7,6,10,6,7,5,21,22,7
        .byte 5,17,23,23,13,7,10,21,22,6,5,23,15,255,24,6,25,26,24,24,17,23,23,14,24,24,25,26,24,24,23,16,255,24,6
        .byte 25,26,24,24,18,23,23,15,24,24,25,26,24,24,23,13,255,24,6,25,26,24,24,19,23,23,16,24,24,25,26,24,24
        .byte 23,2,3,4,1,2,3,4,1,2,3,4,1,23,23,4,1,2,3,4,1,2,255,23,22
        .byte 255,23,7,16,21,22,20,255,23,11,6,5,7,10,20,23,15,9,21,22,6,5,7,10,5,9,8,6,5,17,23,23,255,0,4
        .byte 17,23,16,12,21,22,255,0,5,12,11,0,0,18,23,23,255,0,4,18,23,13,0,21,22,255,0,9,19,23,23,255,0,4
        .byte 19,23,14,0,21,22,255,0,9,20,23,23,2,3,21,22,2,23,4,1,2,3,4,1,2,3,4,1,21,22,4,1,23,23,23
        .byte 14,21,22,18,255,23,10,15,21,22,19,255,23,4,15,21,22,19,255,23,10,16,21,22,20,23,23,23,7,10,21,22,7
        .byte 5,10,6,5,8,10,9,10,7,7,6,21,22,9,10,7,20,0,0,21,22,255,0,5,11,0,12,255,0,4,21,22,12
        .byte 0,0,17,0,0,21,22,255,0,12,21,22,0,0,0,18,0,0,21,22,255,0,12,21,22,0,0,0,19,3,4,1,2
        .byte 3,4,1,2,3,4,1,2,3,4,1,2,21,22,2,1,3,4,255,23,15,13,21,22,18,255,23,18,14,21,22,19,23,23,23
        .byte 5,6,7,5,18,23,23,16,10,6,7,10,6,7,5,10,21,22,6,10,18,23,255,24,4,19,23,23,13,255,116,8,117
        .byte 118,116,116,19,23,255,24,4,20,23,23,14,255,116,8,117,118,112,113,20,23,255,24,4,17,23,23,15,255,116,8,117,118
        .byte 114,115,17,23,1,2,3,4,1,23,23,4,1,2,21,22,1,2,3,4,1,2,3,4,1,255,23,10,15,21,22,20,255,23,9
        .byte 23,13,21,22,17,23,23,23,14,21,22,18,23,23,23,15,21,22,19,255,23,4,14,21,22,18,23,23,23,15,21,22,19
        .byte 23,23,20,9,21,22,5,7,10,7,23,15,21,22,19,23,23,23,16,21,22,20,23,23,17,12,21,22,255,0,4,23,16
        .byte 21,22,20,23,23,23,13,21,22,17,23,23,18,0,21,22,255,0,4,23,13,21,22,17,23,23,23,14,21,22,18,23,23
        .byte 19,0,21,22,255,0,4,23,14,21,22,18,23,23,23,15,21,22,19,23,23,3,1,21,22,4,1,2,3,23,15,21,22
        .byte 19,23,23,23,16,21,22,20,23,23,23,13,21,22,17,255,23,4,16,21,22,20,23,23,23,13,21,22,17,23,23,23,14
        .byte 21,22,18,23,23,23,16,10,21,22,7,5,10,6,5,21,22,7,10,19,23,15,21,22,19,23,23,23,13,24,25,26,255,24,5
        .byte 25,26,24,24,20,23,16,21,22,20,23,23,23,14,24,25,26,255,24,5,25,26,24,24,17,23,13,21,22,17,23,23,23
        .byte 15,24,25,26,255,24,5,25,26,24,24,18,23,14,21,22,18,23,23,23,3,4,1,2,3,4,1,2,3,4,1,2,3
        .byte 4,23,15,21,22,19,255,23,18,16,21,22,20,255,23,18,13,21,22,17,255,23,6,14,9,6,7,5,10,6,7,8,6
        .byte 7,5,10,21,22,6,10,20,255,23,4,15,12,255,0,6,11,255,0,4,21,22,0,0,17,255,23,4,13,255,0,12,21
        .byte 22,0,0,18,255,23,4,14,255,0,12,21,22,0,0,19,255,23,4,4,1,2,3,4,1,2,3,4,1,2,3,4,1
        .byte 2,3,4,1,255,23,23
        .byte 255,23,22,6,5,7,10,8,7,6,9,7,5,6,10,7,10,9,8,6,5,6,7,10,7,255,0,4,11,0,0,12,255,0,6
        .byte 12,11,255,0,50,1,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,21,22,3,255,111,19,21,22,111,111
        .byte 255,0,17,111,21,22,111,111,255,0,17,111,21,22,111,111,109,110,109,110,109,110,109,110,109,110,109,21,22,109,110,109,110
        .byte 111,21,22,111,111,255,0,11,21,22,255,0,4,111,21,22,111,111,0,0,66,67,68,255,0,6,21,22,255,0,4,111,21
        .byte 22,111,111,0,0,69,70,71,72,73,74,0,0,0,21,22,255,0,4,111,21,22,111,111,0,75,76,77,78,79,80,119,119
        .byte 0,0,21,22,255,0,4,111,21,22,111,111,0,81,82,83,84,85,0,119,119,0,0,21,22,255,0,4,111,21,22,111,111
        .byte 0,86,87,88,89,90,119,119,0,0,0,21,22,255,0,4,111,21,22,111,111,91,92,93,94,95,96,119,119,0,0,0,21
        .byte 22,255,0,4,111,21,22,111,111,97,98,99,100,101,102,255,0,5,21,22,255,0,5,21,22,111,111,103,104,105,106,107,108
        .byte 255,0,5,21,22,255,0,5,21,22,111,109,110,109,110,109,110,109,110,109,110,109,110,109,110,109,110,109,110,109,110,109,110
        .byte 255,23,22
        .byte 255,23,9,15,21,22,19,255,23,9,6,5,7,10,8,19,23,23,23,16,21,22,20,255,23,9,255,0,4,11,20,23,23,23
        .byte 13,21,22,17,255,23,9,255,0,5,17,23,23,23,14,21,22,18,255,23,9,255,0,5,18,23,23,23,15,21,22,19,255,23,9
        .byte 2,3,4,21,22,19,23,23,23,16,21,22,20,255,23,11,15,21,22,20,23,23,23,13,21,22,17,255,23,11,16,21,22
        .byte 17,23,23,23,14,21,22,18,255,23,11,13,21,22,18,23,23,16,7,21,22,10,7,7,9,7,18,255,23,6,14,21,22
        .byte 19,23,23,13,0,21,22,0,0,0,12,0,19,255,23,6,15,21,22,20,23,23,14,0,21,22,255,0,5,20,255,23,6
        .byte 16,21,22,17,23,23,15,0,21,22,255,0,5,17,255,23,6,13,21,22,18,23,23,3,4,1,2,3,4,21,22,3,4
        .byte 255,23,6,14,21,22,19,255,23,7,14,21,22,18,255,23,7,15,21,22,20,255,23,7,15,21,22,19,255,23,5,13,6
        .byte 7,21,22,6,7,5,10,6,7,10,6,7,21,22,5,6,18,23,23,23,14,24,24,25,26,255,24,9,25,26,24,24,19
        .byte 23,23,23,15,24,24,25,26,255,24,9,25,26,24,24,20,23,23,23,16,24,24,25,26,255,24,9,25,26,24,24,17,23,23,23
        .byte 1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,255,23,25
numsc:
        .byte 16
nmedat:
        .byte 0,0,104,56,255
        .byte 2,11,24,40,255
        .byte 1,0,80,32,255
        .byte 6,7,80,32,3,13,80,80,1,0,136,104,255
        .byte 5,17,80,8,2,11,136,96,7,7,80,24,7,7,80,112,255
        .byte 3,13,80,8,7,7,24,128,255
        .byte 1,0,80,16,7,7,24,16,255
        .byte 3,13,80,8,6,7,80,112,1,0,24,80,255
        .byte 3,13,24,8,6,7,80,128,255
        .byte 1,0,136,24,1,0,80,72,255
        .byte 3,13,136,88,2,11,80,8,255
        .byte 7,7,24,80,7,7,136,64,7,7,136,144,255
        .byte 2,11,80,80,6,7,136,128,255
        .byte 1,0,136,32,5,17,80,40,7,7,80,16,7,7,80,72,255
        .byte 7,8,104,64,7,9,120,56,6,10,104,80,1,0,56,8,255
        .byte 5,17,136,64,7,7,136,24,7,7,136,112,255
NUMOBJ = 11
objdta:
        .byte 0,0,0,0,0,0,0,0,0,0,7,224,15,240,23,232,35,196,36,36,40,20,28,56,15,240,7,224,3,192,1,128,1,14,64,64,14,64,64
        .byte 1,128,3,192,6,96,4,32,4,32,34,68,63,252,53,84,33,132,1,0,1,128,1,0,1,128,1,0,3,64,6,160,1,255,32,144,255,32,144
        .byte 1,128,3,192,5,168,5,172,7,228,6,100,6,108,27,216,55,160,35,192,35,128,51,192,23,160,0,0,7,160,15,80,1,255,32,64,255,32,64
        .byte 0,0,127,254,64,2,63,196,63,196,26,168,21,88,31,136,31,8,15,48,3,64,1,128,3,64,3,192,6,32,7,224,1,255,88,24,255,88,24
        .byte 3,192,4,32,8,16,16,8,16,8,16,8,16,8,24,24,20,40,11,208,5,160,10,80,10,80,2,64,8,16,2,64,1,255,112,88,255,112,88
        .byte 7,224,8,16,31,248,63,252,113,198,238,187,121,78,191,125,191,125,58,94,87,234,44,52,23,232,13,80,2,160,1,64,1,255,88,88,255,88,88
        .byte 0,0,0,0,0,0,60,0,34,0,49,0,56,128,28,128,14,64,7,36,1,152,0,208,0,104,0,84,0,138,0,4,1,255,8,8,255,8,8
        .byte 7,160,15,208,31,232,24,16,26,136,25,80,28,40,15,208,0,0,15,240,27,88,3,128,19,72,2,128,12,48,28,56,1,254,8,8,254,8,8
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,63,255,64,0,74,172,59,224,96,0,160,0,192,0,0,0,1,254,8,8,254,8,8
        .byte 1,128,12,0,3,64,7,160,0,0,7,160,7,160,7,160,7,160,7,160,7,160,7,160,7,160,7,160,7,160,3,64,1,254,8,8,254,8,8
        .byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,8,8,255,8,8
font:
        .byte 0,0,0,0,0,0,0,0
        .byte 0,48,48,48,48,48,0,48
        .byte 0,108,108,72,0,0,0,0
        .byte 0,0,0,0,0,0,0,0
        .byte 0,24,126,120,126,30,126,24
        .byte 0,230,236,24,48,110,206,0
        .byte 0,48,120,48,126,204,126,0
        .byte 0,24,8,16,0,0,0,0
        .byte 0,12,24,24,24,24,24,12
        .byte 0,96,48,48,48,48,48,96
        .byte 0,0,36,24,126,24,36,0
        .byte 0,0,24,24,126,24,24,0
        .byte 0,0,0,0,0,24,8,16
        .byte 0,0,0,0,60,0,0,0
        .byte 0,0,0,0,0,0,24,24
        .byte 0,0,6,12,24,48,96,0
        .byte 0,126,66,66,130,132,132,252
        .byte 0,2,2,2,2,4,4,4
        .byte 0,62,2,2,254,128,128,248
        .byte 0,62,2,2,126,4,4,124
        .byte 0,66,66,66,126,4,4,4
        .byte 0,124,64,64,124,4,4,124
        .byte 0,124,64,64,252,132,132,252
        .byte 0,62,2,2,2,4,4,4
        .byte 0,126,66,66,254,132,132,252
        .byte 0,126,66,66,126,4,4,124
        .byte 0,48,48,0,0,48,48,0
        .byte 0,48,48,0,0,48,16,32
        .byte 0,0,12,24,48,24,12,0
        .byte 0,0,0,126,0,126,0,0
        .byte 0,0,48,24,12,24,48,0
        .byte 0,60,126,70,12,24,0,24
        .byte 0,56,68,146,170,190,64,60
        .byte 0,124,254,238,254,254,238,238
        .byte 0,252,254,206,252,206,254,252
        .byte 0,124,254,238,224,238,254,124
        .byte 0,252,254,238,238,238,254,252
        .byte 0,254,254,224,254,224,254,254
        .byte 0,254,254,224,252,224,224,224
        .byte 0,124,254,224,238,230,254,124
        .byte 0,238,238,254,254,254,238,238
        .byte 0,254,254,56,56,56,254,254
        .byte 0,254,254,28,28,220,252,120
        .byte 0,238,238,252,248,252,238,238
        .byte 0,224,224,224,224,224,254,254
        .byte 0,238,254,254,254,254,214,214
        .byte 0,206,238,254,254,254,238,230
        .byte 0,124,254,254,238,254,254,124
        .byte 0,252,254,238,254,252,224,224
        .byte 0,124,254,230,222,222,252,118
        .byte 0,252,254,238,252,248,252,238
        .byte 0,126,254,240,124,30,254,252
        .byte 0,254,254,56,56,56,56,56
        .byte 0,238,238,238,254,254,254,124
        .byte 0,238,238,238,124,124,56,56
        .byte 0,214,214,254,254,254,254,238
        .byte 0,238,238,254,56,254,238,238
        .byte 0,238,254,124,124,56,56,56
        .byte 0,254,254,30,56,240,254,254
        .byte 0,60,48,48,48,48,48,60
        .byte 0,0,192,96,48,24,12,0
        .byte 0,120,24,24,24,24,24,120
        .byte 0,48,120,252,48,48,48,0
        .byte 0,0,0,0,0,0,0,255
        .byte 0,60,102,248,96,96,254,0
        .byte 0,0,120,12,124,204,124,0
        .byte 0,96,96,124,102,102,124,0
        .byte 0,0,60,96,96,96,60,0
        .byte 0,12,12,124,204,204,124,0
        .byte 0,0,120,204,248,192,124,0
        .byte 0,28,48,56,48,48,48,0
        .byte 0,0,124,204,204,124,12,120
        .byte 0,192,192,248,204,204,204,0
        .byte 0,48,0,112,48,48,120,0
        .byte 0,12,0,12,12,12,108,56
        .byte 0,96,120,112,112,120,108,0
        .byte 0,48,48,48,48,48,28,0
        .byte 0,0,248,252,252,252,252,0
        .byte 0,0,248,204,204,204,204,0
        .byte 0,0,120,204,204,204,120,0
        .byte 0,0,248,204,204,248,192,192
        .byte 0,0,124,204,204,124,12,14
        .byte 0,0,60,96,96,96,96,0
        .byte 0,0,120,192,120,12,248,0
        .byte 0,48,120,48,48,48,28,0
        .byte 0,0,204,204,204,204,120,0
        .byte 0,0,204,204,120,120,48,0
        .byte 0,0,204,252,252,252,120,0
        .byte 0,0,204,120,48,120,204,0
        .byte 0,0,204,204,204,124,12,120
        .byte 0,0,252,24,48,96,252,0
        .byte 0,30,24,112,24,24,30,0
        .byte 0,24,24,24,24,24,24,0
        .byte 0,240,48,28,48,48,240,0
        .byte 0,60,120,0,0,0,0,0
        .byte 126,195,153,161,161,153,195,126
jtab:
        .byte 248,250,252,254,254,255,255,255,0,0,0,1,1,1,2,2,4,6,8,8,8,99
keys:   .byte 101,20,81,16,52,35,35,112,0,113,1