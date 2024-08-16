;============================================================
; PLATFORM VARIABLES
;============================================================
;
; Memory map:
;
;	$0000-$03ff	 1.0kB	CPU WS
;	$0400-$0fff	 3,0kB	RAM
;	$1000-$17ff	 2,0kB	VIDEO RAM
;	$1800-$1fff	 2,0kB	CHAR RAM
;	$2000-$7fff	24,0kB	RAM
;	$8000-$8fff	 4,0kB	CHAR ROM
;	$9000-$93ff	 1,0kB	VIC VIA's
;	$9400-$97ff	 1,0kB	COLOUR RAM
;	$9800-$9fff	 2,0kB	I/O
;	$a000-$bfff	 8,0kB	RAM
;	$c000-$dfff	 8,0kB	BASIC ROM
;	$e000-$ffff	 8,0kB	KERNAL ROM
;
; Video registers:
;
;	$9000	horizontal centering
;		bits 0-6 horizontal centering
;		bit  7 sets interlace scan
;	$9001	vertical centering
;	$9002	set # of columns
;		bits 0-6 set # of columns
;		bit 7 is part of video matrix address
;	$9003	set # of rows
;		bit 0 sets 8x8 or 16x8 chars
;		bits 1-6 set # of rows
;	$9005	start of character memory
;		bits 0-3 start of character memory (default = 0)
;		bits 4-7 is rest of video address (default= F)
;		BITS 3,2,1,0 CM startinq address
;			     HEX   DEC
;		0000   ROM   8000  32768
;		0001	     8400  33792
;		0010	     8800  34816
;		0011	     8C00  35840
;		1000   RAM   0000  0000
;		1001  xxxx
;		1010  xxxx   unavail.
;		1011  xxxx
;		1100	     1000  4096
;		1101	     1400  5120
;		1110	     1800  6144
;		1111	     1C00  7168
;	$900f	Screen and border color register
;		bits 4-7 select background color
;		bits 0-2 select border color
;		bit 3 selects inverted or normal mode
;------------------------------------------------------------

; Addresses

	MapAddr		= $400		; RAM
	ScreenAddr	= $1000		; VIDEO RAM
	CharAddress	= $1800		; CHAR RAM
	Reg0		= $9000		; Horizontal centering
	Reg1		= $9001		; Vertical centering
	Reg2		= $9002		; Set # of columns
	Reg3		= $9003		; Set # of rows
	Reg5		= $9005		; Start of character memory
	RegF		= $900f		; Screen and border color register
	ColorAttr	= $9400		; Color attributes

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; System constants Atom, needs to be updated for VIC usage 

;	ScreenAddr  	= $8000		; Startaddress video RAM
	ScrSelAddr  	= $b000		; Screen selection address
	KeyRowAddr  	= $b000		; PIA key matrix row
	KeyColAddr  	= $b001		; PIA key matrix column
	SpeakerBit  	= $b002		; Speaker bit
	CmdReg      	= $b400		; Command register AtoMMC
	DatReg      	= $b401		; Data register AtoMMC 
	Timer2_Low  	= $b808		; Timer 2 low byte
	Timer2_High 	= $b809		; Timer 2 high byte
	Timer1_Low  	= $b804		; Timer 1 low byte
	Timer1_High 	= $b805		; Timer 1 high byte

	green		= $00		; Colour definition filters
	yellow		= $55
	blue		= $aa
	red		= $ff

; System calls Atom

	SCRSYNC	 	= $fe66		; wait for next CRT field flyback 60 Hz
	SCRSYNC1 	= $fe6b		; wait for CRT field flyback 60 Hz
	OSRDCH   	= $fe94		; get key subroutine
        READKEY  	= $fe71		; scan key matrix subroutine
	OSWRCH	 	= $fff4		; write character
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;============================================================
; PLATFORM ROUTINES
;============================================================

;------------------------------------------------------------
; INIT, Initialise system
;
; - Disable interrupts
; - Init NTSC screen
; - Copy MPAGD font (96 chars = 768 bytes) to CHAR RAM (CHARcode 0-95)
; - Copy MPAGD tiles to CHAR RAM starting at $1b00 (CHARcode 96-) 
; - Set up screen line offset address table.
;------------------------------------------------------------

Init:

; Disable interrupts

;	lda #$7f
;	sta $911e
;	sta $912e

; Init screen for NTSC
; this sets the background and border color of the screen

; in this case border black and background white
	lda #%00011000			
	sta RegF

; max num of columns (24)
	lda Reg2
	and #%10000000
	ora #24
	sta Reg2

; max num of lines (mult by 2 - 24) for 8x8 chars
	lda Reg3
	and #%01000001
	ora #48		; number of rows multipled by two
	sta Reg3

; horizontal and vertical position must be set for 
; non standard screen resolution (standard 22x23)
; same games even allow the user to change them
; these values are for PAL for NTSC should be different

; move horizontal position in the screen
	lda Reg0
	and #%10000000
	ora #2
	sta Reg0

; move vertical position in the screen
	lda #$18
	sta Reg1

; Set character data pointer to CHAR RAM at $1800
	lda Reg5
	and #%11110000
	ora #%1110
	sta Reg5

; Copy MPAGD font (96 chars = 768 bytes) to CHAR RAM (CHARcode 0-95)

	ldx #0
@loop:
	lda font,x
	sta CharAddress,x
	lda font + $100,x
	sta CharAddress +$100,x
	lda font + $200,x
	sta CharAddress + $200,x
	inx
	bne @loop

; Copy MPAGD tiles to CHAR RAM starting at $1b00 (CHARcode 96-) 

; Set source = MPAGD tileaddress

	lda #<chgfx
	sta tileaddr
	lda #>chgfx
	sta tileaddr+1

; Set destination = CHAR RAM address

	lda #<(CharAddress + $300)
	sta bufaddr
	lda #>(CharAddress + $300)
	sta bufaddr+1

; Copy all MPAGD tiles into CHAR RAM

	ldy #0
@bloop:
	lda (tileaddr),y
	sta (bufaddr),y
	inc tileaddr
	bne :+
	inc tileaddr+1
:	inc bufaddr
	bne :+
	inc bufaddr+1
:	lda tileaddr
	cmp #<bprop		; Check for last tileaddress
	bne @bloop
	lda tileaddr+1
	cmp #>bprop
	bne @bloop

; Set up screen line offset address table.
; Save lb and hb of 24 lines in a table for reference

setsat:
	lda #0			; start
	sta scraddr
	sta scraddr+1

	ldy #0			; vertical lines on screen.
setsa0:
	lda scraddr
	sta SCROFF_lb,y		; write low byte.
	lda scraddr+1
	sta SCROFF_hb,y		; write high byte.
	jsr nline		; next line down.
	iny			; next position in table.
	cpy #24
	bne setsa0
	rts

;----------------------------------------------------------------------
; CLS, Clear screen routine.
;
; Fill screenmem and colourmem  with space
;----------------------------------------------------------------------

cls:
	ldx #0
clsloop:
	lda #0
	sta ScreenAddr,x	; Screen RAM
	sta ScreenAddr+$100,x
	sta ScreenAddr+$200,x
	sta ColorAttr,x		; Colour RAM
	sta ColorAttr+$100,x
	sta ColorAttr+$200,x
	inx
	bne clsloop
	rts

;----------------------------------------------------------------------
; SETPAL, Set colour palette
;
; Not used at the moment 
;----------------------------------------------------------------------

setpal:
	rts

;----------------------------------------------------------------------
; PCHAR, Display characte on screen
;----------------------------------------------------------------------

pchar:
	sec
	sbc #32
	sta fntaddr
pchark:
	jsr gprad		; get screen address.
	ldx #7			; lines to write.
pchar0:
	lda fntaddr
	ldy #0
	sta (scraddr),y 	; copy to screen.
	rts

;----------------------------------------------------------------------
; GETOFFSET, Calculate bufaddr = Y*screen width + X 
;----------------------------------------------------------------------

getoffset:
	ldy dispy
	lda SCROFF_hb,y
	sta bufaddr+1
	lda SCROFF_lb,y
	clc
	adc dispx
	sta bufaddr
	bcc :+
	inc bufaddr+1
:	rts

;-------------------------------------------------------------
; VSYNC, synchronize to 25 frames/sec
;
;  - read joystick/keyboard
;  - handle sound
;  - sync framerate with timer 25Hz
;  - handle shrapnel every even frame
;-------------------------------------------------------------

vsync:
	pha
	tya
	pha
	txa
	pha
	jsr joykey		; read joystick/keyboard.

	ldx $a2			; Set Timer1 expired bit
vsync1:
	cpx $a2			; Check Interrupt Flag Register
	beq vsync1

	lda clock
	and #1
	bne:+
	jsr proshr		; handle shrapnel every even frame
:
	lda sndtyp
	beq sndskip
sndloop:
	lda SpeakerBit		; handle sound
	ldy sndtyp
sndwait:
	dey
	bne sndwait
	eor #4
;	sta SpeakerBit
	dec sndtyp
	bne sndloop
sndskip: 
	pla
	tax
	pla
	tay
	pla
	rts

