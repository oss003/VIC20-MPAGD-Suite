;----------------------------------------------------------------------
; VIC20 Platform Specific functions
;----------------------------------------------------------------------

;============================================================
; PLATFORM VARIABLES
;============================================================
;
; Memory map:
;
;	$0000-$01ff	 0.5kB	CPU WS
;	$0200-$03ff	 0.5kB	VIDEO RAM
;	$0400-$0fff	 3,0kB	RAM
;	$1000-$1fff	 4,0kB	CHAR RAM
;	$2000-$7fff	24,0kB	RAM
;	$8000-$8fff	 4,0kB	CHAR ROM
;	$9000-$93ff	 1,0kB	VIC VIA's
;	$9600-$97ff	 1,0kB	COLOUR RAM
;	$9800-$9fff	 2,0kB	I/O
;	$a000-$bfff	 8,0kB	RAM
;	$c000-$dfff	 8,0kB	BASIC ROM
;	$e000-$ffff	 8,0kB	KERNAL ROM
;
; Keyboard matrix:
;                          	  $9121
;          $9120	b7  b6  b5  b4  b3  b2  b1  b0
; 	127-0111 1111 	F7  Hom  -   0   8   6   4   2
;               ASCII   ??  ??  45  48  56  54  52  50
;                 VIC   07  06  05  04  03  02  01  00
;
; 	191-1011 1111 	F5       @   O   U   T   E   Q
;               ASCII   ??  ??  64  79  85  84  69  81
;                 VIC   17  16  15  14  13  12  11  10
;
; 	223-1101 1111 	F3   =   :   K   H   F   S  Com
;               ASCII   ??  61  58  75  72  70  83  ??
;                 VIC   27  26  25  24  23  22  21  20
;
; 	239-1110 1111 	F1  RSh  .   M   B   C   Z  Spc
;               ASCII   ??  ??  46  77  66  67  90  32
;                 VIC   37  36  35  34  33  32  31  30
;
; 	247-1111 0111	Cur  /   ,   N   V   X  LSh Run
;               ASCII   ??  47  44  78  86  88  ??  ??
;                 VIC   47  46  45  44  43  42  41  40
;
; 	251-1111 1011	Cur  ;   L   J   G   D   A  Ctr
;               ASCII   ??  59  76  74  71  68  65  ??
;                 VIC   57  56  55  54  53  52  51  50
;
; 	253-1111 1101	Ret  *   P   I   Y   R   W
;               ASCII   13  42  80  73  89  82  87
;                 VIC   67  66  65  64  63  62  61  60
;
; 	254-1111 1110	Del Pnd  +   9   7   5   3   1
;               ASCII   08  ??  43  57  55  53  51  49
;                 VIC   77  76  75  74  73  72  71  70
;
;	 P =$65,O=$14,A=$51,Q=$10
;	SPC=$30,N=$44,M=$34
;	 1 =$70,2=$00,3=$71,4=$01
;
; Joysticks:
;
;               b7    b6    b5    b4    b3    b2    b1    b0
;	$9111	-     -     Fire  Left  Down  Up    -     -
;	$9120	Right -     -     -     -     -     -     -
;
;	$9122	DDRB = Data Direction Register port $9120
;               0 = input, 1 = output
;		Bit7 should be 0 to read joystick $9120
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
;	$9004	TV raster beam line
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
;
; Sound registers:
;
;	$900a	Frequency for oscillator 1 (low)
;		(on: 128-255)
;	$900b	Frequency for oscillator 2 (medium)
;		(on: 128-255)
;	$900c	Frequency for oscillator 3 (high)
;		(on: 128-255)
;	$900d	Frequency of noise source
;	$900e	bit 0-3 sets volume of all sound
;		bits 4-7 are auxiliary color information
;	$900f	Screen and border color register
;		bits 4-7 select background color
;		bits 0-2 select border color
;		bit 3 selects inverted or normal mode
;
; I/O:
;	VIA1 -VIA2
;-----------------------------------------------------------
;	$9110,$9120 	Port B output register
;	$9111,$9121 	Port A output register
;			(PA0) Bit 0=Serial CLK IN
;			(PA1) Bit 1=Serial DATA IN
;			(PA2) Bit 2=Joy 0
;			(PA3) Bit 3=Joy 1
;			(PA4) Bit 4=Joy 2
;			(PA5) Bit 5 = Lightpen/Fire button
;			(PA6) Bit 6=Cassette switch sense
;			(PA7) Bit 7=Serial ATN out
;	$9112,$9122	Data direction register B
;	$9113,$9123	Data direction register A
;	$9114,$9124	Timer 1 low byte
;	$9115,$9125	Timer 1 high byte & counter
;	$9116,$9126	Timer 1 low byte
;	$9117,$9127	Timer 1 high byte
;	$9118,$9128	Timer 2 low byte
;	$9119,$9129	Timer 2 high byte
;	$911A,$912A	Shift register
;	$911B,$912B	Auxiliary control register
;	$911C,$912C	Peripheral control register
;			(CA1, CA2, CB1, CB2)
;			CA1 = restore key (Bit 0)
;			CA2 = cassette motor control (Bits 1-3)
;			CB1 = interrupt signal for received
;			RS-232 data (Bit 4)
;			CB2=transmitted RS-232 data (Bits 5-7)
;	$911D,$912D	Interrupt flag register
;	$911E,$912E	Interrupt enable register
;	$911F,$912F	Port A (Sense cassette switch)

; VIC constants

	ScreenSize	= ScrWidth * ScrHeight * 8	; Screen size bytes
	ScreenRowBytes	= ScrWidth * 16			; bytes/line
	SpriteMaxY	= 161				; maxlines - 16 + 1

; VIC variables:

	fontcol:	.byte 0		; Frontcolour text
	mode:		.byte 0		; 0=MPAGD, 1=MiniPaint
.if modelflag				; Set crtflag to identify machine, NTSC or PAL
	crtflag:	.byte 1		; PAL
.else
	crtflag:	.byte 0		; NTSC
.endif

; Addresses

	ScreenAddr	= $0200		; VIDEO RAM
	MapAddr		= $0400		; RAM
	CharAddress	= $1000		; CHAR RAM
	Reg0		= $9000		; Horizontal centering
	Reg1		= $9001		; Vertical centering
	Reg2		= $9002		; Set # of columns
	Reg3		= $9003		; Set # of rows
	Reg4		= $9004		; Vsync signal
	Reg5		= $9005		; Start of character memory
	RegA		= $900a		; Colour and auxcolour register
	RegB		= $900b		; Colour and auxcolour register
	RegC		= $900c		; Colour and auxcolour register
	RegD		= $900d		; Colour and auxcolour register
	RegE		= $900e		; Colour and auxcolour register
	RegF		= $900f		; Screen and border color register

	JoyAddr1	= $9111		; Joystick address 1
	Timer11_Low  	= $9114		; Timer 1 low byte
	Timer11_High 	= $9115		; Timer 1 high byte
	Timer12_Low  	= $9118		; Timer 2 low byte
	Timer12_High 	= $9119		; Timer 2 high byte
	AUX1		= $911b		; Auxiliary control register
	IFR1		= $911d		; Interrupt Flag Register
	IER1		= $911e		; Interrupt Enable Register

	KeyColAddr	= $9120		; Keyboard column (write)
	JoyAddr2	= $9120		; Joystick address 2
	KeyRowAddr	= $9121		; Keyboard row (read)
	DDRB		= $9122		; Bit7 must be 0 for joystick read
	Timer21_Low  	= $9124		; Timer 1 low byte
	Timer21_High 	= $9125		; Timer 1 high byte
	AUX2		= $912b		; Auxiliary control register
	IFR2		= $912d		; Interrupt Flag Register
	IER2		= $912e		; Interrupt Enable Register

	ColorAttr	= $9600		; Color attributes
	HiRAM		= $a000		; Hi RAM 8kB

;----------------------------------------------------------------------
; VIC20 video hardware fns
;----------------------------------------------------------------------

init:

	lda #0
	sta crtflag
	lda $ede4
	cmp #5
	beq Ntsc_System
	lda #1
	sta crtflag
Ntsc_System:	

; Disable interrupts

	lda #$7f
	sta IER1
	sta IFR2
	sta IER2

; Set audio volume and flag to 0 to check when disabling
; beep and crash

	lda #0
	sta RegA
	sta RegB
	sta RegC
	sta RegD
	lda #$00
	sta SOUNDS_STATUS

	lda RegE
	ora #$0f		; Set volume max
	sta RegE

; Init screen for NTSC
; this sets the background and border color of the screen
; in this case border black and background black

	lda #%00011000			
	sta RegF

; Reset fontcolor
	lda #0		; Black
	sta fontcol


	lda #0		; Mode 0 = MPAGD
	jsr SetReg

; Init screencharacters
; Fill screen with characters from left to right, up to down

	ldx #0
	ldy #0
scrinit:
	txa
	sta ScreenAddr,x
	tya
	sta ColorAttr,x
	inx
	cpx#242
	bne scrinit

; Reset fontcolor

	lda #0			; Black
	sta fontcol

; Init music

;	jsr initmusic

; Init VIA1 timer11 for timeout 1/25 sec

	lda #$e0		; T1 continuous
	sta AUX1
	lda #$7f		; No int
	sta IER1
	lda #0
	sta IFR1		; Clear interrupt flags
	lda #<timervalue	; Timer 1/25 sec
	sta Timer11_Low
	lda #>timervalue
	sta Timer11_High

; Pre-shifted byte tables if parameter 'r' given
; 4 tables are created which 0,2,4 ad 6 bits pre-shifted
; The bits are shifted to the right and overflow bits are
; inserted to the left. So 1 byte contains a left and right part

.if rflag
shift0 = HiRAM
shift2 = HiRAM+256
shift4 = HiRAM+512
shift6 = HiRAM+768

; Pre-shifted table 0 pixels

	ldx #0
shf0:
	txa
	sta shift0,x
	inx
	bne shf0

; Pre-shifted table 2 pixels

	ldx #0
shf2:
	ldy #2
	txa
shf2loop:
	lsr a
	bcc :+
	ora #$80
:
	dey
	bne shf2loop
	sta shift2,x
	inx
	bne shf2

; Pre-shifted table 4 pixels

	ldx #0
shf4:
	ldy #4
	txa
shf4loop:
	lsr a
	bcc :+
	ora #$80
:
	dey
	bne shf4loop
	sta shift4,x
	inx
	bne shf4

; Pre-shifted table 6 pixels

	ldx #0
shf6:
	ldy #6
	txa
shf6loop:
	lsr a
	bcc :+
	ora #$80
:
	dey
	bne shf6loop
	sta shift6,x
	inx
	bne shf6
.endif

	rts

;========================================================
; KEY AND JOYSTICK ROUTINES
;--------------------------------------------------------
; Keys
;
; Out: joyval=x65FUDLR (bit cleared if key pressed)
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
;--------------------------------------------------------

;----------------------------------------------------
; Interrupt service
;----------------------------------------------------

intserv:
	inc $9601	; scroll colour char topleft
	pla
	tay
	pla
	tax
	pla
	rti

;--------------------------------------------------------
; PRSKEY
;
; Wait for keypress.
;--------------------------------------------------------

prskey:
	tya
	pha
prsloop:
	jsr vsync
	jsr joykey
	ora #$80
	cmp #$ff
	bne :+
	jsr readkey
	cmp #255
	beq prsloop		; wait until key pressed
:
prsrelease:
	jsr vsync
	jsr joykey
	ora #$80
	cmp #$ff
	beq :+
	jsr readkey
	cmp #255
	bne prsrelease		; wait until key released
:
	pla
	tay
	rts

;--------------------------------------------------------
; KTEST
;
; Keyboard test routine with key in A.
; Scan key and clear carry if key pressed
;--------------------------------------------------------

ktest:
	sta z80_a		; save key

	lsr a
	lsr a
	lsr a
	lsr a
	tax
	lda colmask,x
	sta KeyColAddr		; set row

	lda z80_a
	and #$0f
	tax			; get column

	lda KeyRowAddr		; read column
	and keymask,x		; check if key pressed
	beq pressed		; Skip if pressed
	sec			; key not pressed
	rts
pressed:
	clc			; key pressed
	rts

colmask:	.byte 127,191,223,239,247,251,253,254
keymask:	.byte $01,$02,$04,$08,$10,$20,$40,$80

;---------------------------------------------------------------
; KGET
; 
; Read keyboard matrix if a key is pressed.
; Return A = high nibble=row and low nibble=col
;---------------------------------------------------------------

kget:
	stx xtmp		; Save x-reg
kget0:
	ldx #0			; reset row	
kget1:
	lda colmask,x		; Get mask
	sta KeyColAddr		; Set row
	lda KeyRowAddr		; Read columns
	sta tmp			; Save colums
	cmp #$ff		; Key pressed?
	bne kget2		; Yes, skip
	inx			; Next row
	cpx #8			; All rows scanned?
	bne kget1		; No, repeat
	jmp kget0		; No key pressed, a=$ff
kget2:
	ldy #0			; Reset column
kget3:
	lsr tmp			; Test bit
	bcc kget4		; If clear, key pressed
	iny			; Next column
	jmp kget3		; Repeat
kget4:
	stx tmp			; Get row
	asl tmp			; Make high nibble
	asl tmp
	asl tmp
	asl tmp
	tya			; Get column
	clc
	adc tmp			; Add row as low nibble
	sta tmp
kget5:
	lda KeyRowAddr		; Read columns
	cmp #$ff
	bne kget5

	lda tmp			; Return keycode
	ldx xtmp		; Restore X-reg
	rts			; Return

;-------------------------------------------------------------
; READKEY
;
; Check keyboard matrix if a key is pressed
; Return keymask in Y
;-------------------------------------------------------------

readkey:
	lda #$ff		; Keymask
	sta tmp
	ldx #7
scanrow:
	lda colmask,x
	sta KeyColAddr		; set row
	lda KeyRowAddr		; read column
	and tmp			; Reset bit is key pressed
	sta tmp
	dex
	bpl scanrow
	lda tmp
	tay
	rts

;-------------------------------------------------------------
; JOY1SCAN
;
; Initializes joystick
;-------------------------------------------------------------

joyinit:
	rts

;-------------------------------------------------------------
; JOY1SCAN
;
; Scan joystick1 and return key in z80_e
; Up, down, left, right and fire are read from the joystick
; Fire2 and fire3 are read from the keyboard
;-------------------------------------------------------------

joy1scan:
	lda #$ff
	sta z80_e

	lda JoyAddr1		; Read joystick input
	lsr a
	lsr a

	lsr a
	rol z80_e		; Up
	lsr a
	rol z80_e		; Down
	lsr a
	rol z80_e		; Left

	lda #%01111111
	sta DDRB		; Bit7 = input joystick b7
	lda JoyAddr2
	asl a
	rol z80_e		; Right
	lda #%11111111
	sta DDRB		; Reset  

	lda JoyAddr1
	and #%00100000
	bne :+
	lda z80_e
	and #%11101111		; Fire
	sta z80_e
:
	lda keys+6		; Scan keys for Fire3
	jsr ktest
	bcs :+
	lda z80_e
	and #%10111111
	sta z80_e
:
	lda keys+5		; Scan keys for Fire2
	jsr ktest
	bcs :+
	lda z80_e
	and #%11011111
	sta z80_e
:
	rts

;-------------------------------------------------------------
; JOY2SCAN
;
; TODO, Scan joystick2 and return key in z80_e
;-------------------------------------------------------------

joy2scan:
	rts



;========================================================
; HARDWARE DEPENDING ROUTINES
;========================================================

;----------------------------------------------------
; DELAY
;
; Delay routine A * 1/50 sec
;----------------------------------------------------

delay:
	sta xtmp		; set delaycounter
del_loop:
	lda Reg4
	bne del_loop		; wait for flyback (1/60 sec)

	ldy #208		; wait 208 x 16 = 3328 usec
delay1:
	adc ($80,x)		;	 6 usec
	adc ($80,x)		;	 6 usec
	dey			;	 2 usec
	bne delay1		;	 2 usec
	nop			; 2 usec
	nop			; 2 usec
				; tot: 20000 usec
	dec xtmp		; Next 1/50 sec delay
	bne del_loop
	rts

;-------------------------------------------------------------
; VSYNC
;
; - read joystick/keyboard
; - sync framerate with timer 25Hz
; - handle shrapnel every even frame
; - handle sound
;-------------------------------------------------------------

vsync:
	pha			; save registers
	tya
	pha
	txa
	pha

;-------------------------------------------------------------
; Read joystick/keyboard.
;-------------------------------------------------------------

	jsr joykey

;-------------------------------------------------------------
; Sync framerate with timer 25Hz
;-------------------------------------------------------------

	lda #$40		; set Timer1 expired bit
vsync1:
	bit IFR1		; check Interrupt Flag Register
	beq vsync1		; wait if timer not expired
	lda Timer11_Low		; clear Timer interrupt

rasterwait:
	lda Reg4
	bne rasterwait

;-------------------------------------------------------------
; Handle sound command
;-------------------------------------------------------------

	jsr make_sound2

;-------------------------------------------------------------
; Handle shrapnel every even frame
;-------------------------------------------------------------

	lda clock		; get clock
	and #1			; 2 frames expired
	bne:+
	jsr proshr		; process shrapnel
:

;-------------------------------------------------------------
; Handle sound BEEP and CRASH
;-------------------------------------------------------------

	lda sndtyp
	beq sndskip

sndloop:
	ldy sndtyp
sndwait:
	dey
	bne sndwait

	lda sndtyp
	bmi crashaudio

; BEEP command:
; enable sound on channel 2
; range 0-127

beepaudio:
	lda RegE
	ora #$0f
	sta RegE

	lda SOUNDS_STATUS
	ora #%01000000
	sta SOUNDS_STATUS
	lda #$ff
	sec
	sbc sndtyp
	lsr
	lsr
	ora #%11000000
	sta RegC
	jmp endaudiotype

; CRASH command:
; enable sound on channel 3
; range 128-255

crashaudio:
	lda RegE
	ora #$0f
	sta RegE
	lda SOUNDS_STATUS
	ora #%10000000
	sta SOUNDS_STATUS
	lda #$ff
	sec
	sbc sndtyp
	ora #%10000000
	sta RegD

endaudiotype:
	dec sndtyp
	beq sndskip
	cmp #128
	beq sndskip
	jmp sndloop

; disable sound

sndskip: 
	bit SOUNDS_STATUS
	bvc :+
	lda SOUNDS_STATUS
	and #%10111111
	sta SOUNDS_STATUS
	lda #$00
	sta RegC
:
	bit SOUNDS_STATUS
	bpl :+

	lda SOUNDS_STATUS
	and #%01111111
	sta SOUNDS_STATUS
	lda #$00
	sta RegD
:
	pla
	tax
	pla
	tay
	pla
	rts

sndtyp:		.byte 0		; Beep sound parameter

;========================================================
; SPRITE HANDLING ROUTINES
;========================================================

;----------------------------------------------------
; SPRITE
; 
; Draw object sprite
;----------------------------------------------------

.if oflag
sprite:
	stx xtmp		; Save X-reg
	jsr scadd 		; get screen address in scraddr.

	lda dispx 		; x position.
	and #7 			; position straddling cells.
	sta z80_b		; store in b register.

	lda z80_l		; store sprite graphic address.
	sta sprit1+1
	sta sprit2+1
	lda z80_h
	sta sprit1+2
	sta sprit2+2

	ldx #0			; pixel height.
	ldy #0
sprit1:
	lda objdta,x		; fetch first byte.
	sta spr
	inx
sprit2:
	lda objdta,x
	sta spr+1

	lda #0
	sta spr+2
	jsr sprit7		; shift sprite

	lda spr			; fetch graphic.
	ldy #0			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	lda spr+1		; fetch graphic.
	ldy #16			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	lda spr+2		; fetch graphic.
	ldy #32			; _BEEB
	eor (scraddr),y		; merge with screen image.
	sta (scraddr),y		; write to screen.

	jsr nline

	inx			; next source byte.
	cpx #32
	bne sprit1		; repeat

	ldx xtmp		; retreive X-reg
	rts

.endif

;------------------------------------------------------------------
; SCADD
;
; This routine returns a screen address for X,Y in scraddr.
;------------------------------------------------------------------

scadd:
	lda dispx		; get x
	and #$f8		; reset bit 0-2 for get block pos
	sta scraddr		; set lb scraddr
	lda #0
	sta scraddr+1		; set hb scraddr
	asl scraddr		; multiply X by 2
	rol scraddr+1

; Add line address Y pos to X

	ldx dispy		; get row address Y pos
	clc
	lda scraddr
	adc SCADTB_lb,x
	sta scraddr
	lda scraddr+1
	adc SCADTB_hb,x	
	sta scraddr+1

	rts

;------------------------------------------------------------------
; SSPRIA
;
; Erase single sprite, old (ix+0).
;------------------------------------------------------------------

sspria:
	jsr gsprad		; get old sprite address.
sspri2:
.if rflag
	lda z80_e		; 3c
	sta dline_spraddr1+1	; 4c
	sta dline_spraddr2+1	; 4c
	lda z80_d		; 3c
	sta dline_spraddr1+2	; 4c
	sta dline_spraddr2+2	; 4c

	ldx sprshft
	lda shift_table,x
	sta dline_shift1+1
	sta dline_shift2+1
	lda shift_table+1,x
	sta dline_shift1+2
	sta dline_shift2+2

	ldx #0			; byte counter.
sspri0:
	jsr dline		; draw a line.
  .if bflag
	cpx #48			; big sprites have 24 bytes
  .else
	cpx #32			; normal sprites have 32 bytes
  .endif
	bne sspri0		; repeat 16 times x 2 bytes
	rts
.else
  .if bflag
	lda #24			; big sprites have 24 lines
  .else
	lda #16			; normal sprites have 16 lines.
  .endif
	sta acnt
sspri0:
	jsr dline		; draw a line.
	dec acnt
	bne sspri0		; repeat 16 times
	rts

acnt:	.byte 0			; line counter

.endif

;------------------------------------------------------------------
; SSPRIB
;
; Display single sprite, new (ix+5).
;------------------------------------------------------------------

ssprib:
	jsr gspran 		; get new sprite address.
	jmp sspri2		; Display sprite

;------------------------------------------------------------------
; SSPRIC
;
; Erase old (ix+0) and display new (ix+5) sprite line by line.
;------------------------------------------------------------------

sspric:
	lda fontcol			; Background colour
	sta z80_c
	jsr cspr_old

	ldy #17
	lda (z80_ix),y
	sta z80_c
	jsr cspr_new

.if rflag
	jsr gsprad 		; get old sprite address.

	lda z80_e		; 3c
	sta ddline_spraddr3+1	; 4c
	sta ddline_spraddr4+1	; 4c

	lda z80_d		; 3c
	sta ddline_spraddr3+2	; 4c
	sta ddline_spraddr4+2	; 4c

	ldx sprshft		; set spriteshift table addresses
	lda shift_table,x
	sta ddline_shift3+1
	sta ddline_shift4+1
	lda shift_table+1,x
	sta ddline_shift3+2
	sta ddline_shift4+2

	jsr exx  		; store addresses.
	jsr gspran 		; get new sprite addresses.

	lda z80_e		; 3c
	sta ddline_spraddr1+1	; 4c
	sta ddline_spraddr2+1	; 4c

	lda z80_d		; 3c
	sta ddline_spraddr1+2	; 4c
	sta ddline_spraddr2+2	; 4c

	ldx sprshft
	lda shift_table,x
	sta ddline_shift1+1
	sta ddline_shift2+1
	lda shift_table+1,x
	sta ddline_shift1+2
	sta ddline_shift2+2

	ldx #0
lloop:
	jsr ddline 		; draw a line.
  .if bflag
	cpx #48
  .else
	cpx #32
  .endif
	bne lloop
	rts

shift_table:
	.word shift0
	.word shift2
	.word shift4
	.word shift6

;------------------------------------------------------------------
; DLINE
;
; Draw a sprite line, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;------------------------------------------------------------------

dline:

dline_spraddr1:
	ldy $ffff,x 		; get graphic data as pointer.
dline_shift1:
	lda $ffff,y		; get pre-shifted data
	sta sprtmp		; save byte
	and z80_c 		; mask right for left byte

	ldy #0
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; display left byte

; carry to next screen byte

	lda sprtmp		; fetch data.
	and z80_b 		; mask for left part
	sta sprtmp

; middle screen byte

	inx
dline_spraddr2:
	ldy $ffff,x 		; second bit of data.
dline_shift2:
	lda $ffff,y
	sta sprtmp2
	and z80_c 		; mask away what's not needed.
	ora sprtmp
	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; display middle bye.

; carry to last screen byte

	lda sprtmp2
	and z80_b 		; mask away unwanted

	ldy #32
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; display right byte.

	inx
.else

	jsr gsprad 		; get old sprite address.
	jsr exx  		; store addresses.
	jsr gspran 		; get new sprite addresses.

.if bflag
	lda #23
.else
	lda #15
.endif
	sta ccnt
lloop:
	jsr dline 		; draw a line.
	jsr exx  		; restore old addresses.
	jsr dline 		; delete a line.
	jsr exx  		; flip to new sprite addresses.
	dec ccnt
	bne lloop

	jsr dline		; draw a line.
	jsr exx 		; restore old addresses.

;-------------------------------------------------------------
; Drop through.
; Line drawn, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;-------------------------------------------------------------

dline:
	ldy #0
	lda (z80_de),y 		; graphic data.
	pha
	and z80_c 		; mask away what's not needed.
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; left byte on screen.

	pla
	and z80_b 		; mask away unwanted bits.
	ldy #32
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; right byte on screen

	inc z80_e 		; next graphic.
	bne :+
	inc z80_d
:
; _BEEB screen arrangement
;	sec
;	lda scraddr 		; one character cell to the left.
;	sbc #8
;	sta scraddr
;	lda scraddr+1
;	sbc #0
;	sta scraddr+1
	ldy #0
	lda (z80_de),y 		; second bit of data.
	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; middle byte on screen.

	inc z80_e 		; point to next line of data.
	bne :+
	inc z80_d
:
; _BEEB screen arrangement
;	sec
;	lda scraddr 		; another char left.
;	sbc #8
;	sta scraddr
;	lda scraddr+1
;	sbc #0
;	sta scraddr+1
.endif

;----------------------------------------------------------------------
; Line drawn, now work out next target address.
;----------------------------------------------------------------------

; _BEEB screen arrangement

nline:
	lda scraddr 			; get low byte of address.
	and #15				
	cmp #15				; is this last line of row?
	beq beeb_next_row

	; within same row
	inc scraddr			; new low byte of address.
	rts

beeb_next_row:
	clc
	lda scraddr
	adc #<(ScreenRowBytes-15)
	sta scraddr			; new low byte of address.
	lda scraddr+1
	adc #>(ScreenRowBytes-15)
	sta scraddr+1			; new high byte of address.
	rts

.if rflag
;-------------------------------------------------------------
; Drop through.
; Line drawn, now work out next target address.
;
; Input:
;  B  = right mask
;  C  = left mask
;  DE = spriteaddress
;  scraddr = screen address
;-------------------------------------------------------------

ddline:

; NEW SPRITE

; first screen byte

ddline_spraddr1:
	ldy $ffff,x 		; graphic data.
ddline_shift1:
	lda $ffff,y
	sta sprtmp
	and z80_c 			; mask away what's not needed.

	ldy #0
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to next screen byte

	lda sprtmp			; fetch data.
	and z80_b 			; mask away unwanted
	sta sprtmp

; middle screen byte

	inx
ddline_spraddr2:
	ldy $ffff,x 		; second bit of data.
ddline_shift2:
	lda $ffff,y
	sta sprtmp2
	and z80_c 			; mask away what's not needed.
	ora sprtmp

	ldy #16
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; carry to last screen byte

	lda sprtmp2
	and z80_b 			; mask away unwanted

	ldy #32
	eor (scraddr),y 	; XOR with what's there.
	sta (scraddr),y 	; bung it in.

; _BEEB next row

	lda scraddr 		; get low byte of address.
	and #15				
	cmp #15				; is this last line of row?
	beq :+

	; within same row can't increment page
	inc scraddr			; new low byte of address.
	bne ddline2

:
	clc
	lda scraddr
	adc #<(ScreenRowBytes-15)
	sta scraddr			; new low byte of address.
	lda scraddr+1
	adc #>(ScreenRowBytes-15)
	sta scraddr+1		; new high byte of address.

; OLD SPRITE

ddline2:

; still first row but different data!

	dex

; first screen byte

ddline_spraddr3:
	ldy $ffff,x			; graphic data.
ddline_shift3:
	lda $ffff,y
	sta sprtmp
	and z80_cp 			; mask away what's not needed.

	ldy #0
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; carry to next screen byte

	lda sprtmp			; fetch data.
	and z80_bp 			; mask away unwanted
	sta sprtmp

; middle screen byte

	inx
ddline_spraddr4:
	ldy $ffff,x 		; second bit of data.
ddline_shift4:
	lda $ffff,y
	sta sprtmp2
	and z80_cp 			; mask away what's not needed.
	ora sprtmp

	ldy #16
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; last screen byte

	lda sprtmp2			; fetch data.
	and z80_bp 			; mask away unwanted

	ldy #32
	eor (z80_hlp),y 	; XOR with what's there.
	sta (z80_hlp),y 	; bung it in.

; next row of sprite data

	inx

; _BEEB screen arrangement

	lda z80_lp 			; get low byte of address.
	and #15				
	cmp #15				; is this last line of row?
	beq :+

	; within same row can't increment page
	inc z80_lp			; new low byte of address.
	rts

:
	clc
	lda z80_lp
	adc #<(ScreenRowBytes-15)
	sta z80_lp			; new low byte of address.
	lda z80_lp+1
	adc #>(ScreenRowBytes-15)
	sta z80_lp+1		; new high byte of address.
	rts
.endif

;----------------------------------------------------
; SPRINTEINK
;----------------------------------------------------

cspr:
	lda z80_c
	ldy #17
	sta (z80_ix),y
	jsr cspr_new
	rts

cspr_new:
	ldy #8
	lda (z80_ix),y			; Get sprite y
	jmp cspr_go
cspr_old:
	ldy #3
	lda(z80_ix),y
cspr_go:
	sta dispy
	cmp #8*ScrHeight+1		; Check if on screen
	bcc :+
	rts
:
	iny
	lda (z80_ix),y			; Get sprite x
	sta dispx

; Calculate colour attribte address in scraddr

	jsr coladdr
 
; Set number of rows

	lda #2
	sta z80_b
	lda dispy
	and #15
	bne :+
	dec z80_b
:
; Loop to colour row blocks

cspr0:
	ldy #0
	lda z80_c
	sta (scraddr),y
cspr6:
	ldy #1
	sta (scraddr),y
cspr7:
	lda dispx
	and #7
	beq cspr1

	ldy #2
	lda z80_c
	sta (scraddr),y
cspr1:
	lda scraddr
	clc
	adc #22
	sta scraddr
	bcc :+
	inc scraddr+1
:
	dec z80_b
	bne cspr0
	rts

; Calculate colour attribte address
; scraddr = COLOR + dispy/16 + dispx/8

coladdr:
	lda dispy
	lsr a
	lsr a
	lsr a
	lsr a
	tay
	lda COLATTTB_hb,y
	sta scraddr+1
	lda COLATTTB_lb,y
	sta scraddr
	lda dispx
	lsr a
	lsr a
	lsr a
	clc
	adc scraddr
	sta scraddr
	bcc :+
	inc scraddr+1
:
	rts

;----------------------------------------------------
; Display character in A at dispx,dispy.
;
; Input:
;  A 	   = character to print
;----------------------------------------------------

pchar:
	sta fntaddr
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
	sta fntaddr		; that's the low byte.
	lda fntaddr+1
	adc #>(FONT-256)
	sta fntaddr+1		; add displacement.

pchark:
	jsr gaadd		; set colour attribute
	lda fontcol
	ldy #0
	sta (bufaddr),y

	jsr gprad		; get screen address.
	ldx #7			; lines to write.
pchar0:
	ldy #0
	lda (fntaddr),y 	; get image byte.
	ldy scrtab,x		; Get rowoffset
.if iflag
	eor #TxtInvert		; Invert
.endif
	sta (scraddr),y 	; copy to screen.
	inc fntaddr		; next image byte.
	bne :+
	inc fntaddr+1
:
	dex			; next screen row down.
	bpl pchar0		; repeat.

	rts

scrtab:		.byte $07,$06,$05,$04,$03,$02,$01,$00

;----------------------------------------------------
; LoadFile
;----------------------------------------------------

loadfile:
	tax
	lda #<msgdat		; pointer to messages.
	sta z80_l
	lda #>msgdat
	sta z80_h
	jsr getwrd		; get message number.

; Init screen for NTSC
; this sets the background and border color of the screen
; in this case border black and background black

	lda #%00001000			
	sta RegF

; z80_hl should point to the textstring now.
; Read filename +length into buffer

	ldy #0
nameloop:
	lda (z80_hl),y
	bmi lastchar
	sta filename,y
	iny
	jmp nameloop
lastchar:
	and #$7f
	sta filename,y
	iny
	sty filelen

; Which device to use (in A the file num 1 is OK - x with 8 is the type of device
;                       usually the first drive, and y set to 1 reads from the header
;                       of the file the location where to save it)
	lda #1
	ldx #8		; Disk device
	ldy #1
	jsr $ffba

; Load the file name (in accumulator the size of the string
;                      containing the name, x low address for FileName,
;                      y high address for FileName)
	lda filelen
	ldx #<filename
	ldy #>filename
	jsr $ffbd

; load from disk to ram
	lda #0		; accumulator must be set to 0 to load the image
	ldx #<$ff	; target location low not set (got from header)
	ldy #>$ff	; target location high (got from header)
	jsr $ffd5

	rts

filename:
	.byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
filelen:
	.byte 0

;----------------------------------------------------
; Sound
;----------------------------------------------------

; Actual memory locations

CHANNEL1_ENABLED  = $ea
CHANNEL2_ENABLED  = $eb
CHANNEL3_ENABLED  = $ec
CHANNEL4_ENABLED  = $ed
MUSIC_ON		  = $ee

SetSoundVars:		.byte $0a, $19, $10, $40
SetSoundDuration:	.byte $0a, $19, $10, $40
SetSoundTone: 		.byte $de

;-------------------------------------------- 
;                      SOUND    0   1   2   3 
;-------------------------------------------- 
SndTone:		.byte 200,200,222,220
SndChannel:		.byte   1,  0,  3,  1
SndDuration:		.byte  60, 60, 60, 40

;-------------------------------------------- 
; Called by SOUND with parameter in A
;-------------------------------------------- 

playsound:
	tay
	lda SndTone,y		; lda $a0
	sta SetSoundTone	; sta SetSoundTone
	lda SndDuration,y	; lda $a2
	ldx SndChannel,y	; ldx $a1
	sta SetSoundDuration,x	; sta SetSoundDuration,x
	lda SndTone,y		; lda $a0
	sta SetSoundVars,x	; sta SetSoundVars,x
	rts

;-------------------------------------------- 
; Make sound
; Called by vsync every frame
;-------------------------------------------- 

make_sound2:
	ldy #$00
	ldx SetSoundTone
	lda SetSoundVars
	cmp SetSoundDuration
	bcs l0366

	lda SetSoundVars
	cmp SetSoundDuration
	bne :+
	lda MUSIC_ON
	sta CHANNEL1_ENABLED	; Enable channel 1
	jmp l0374
:

	lda #$0
	sta CHANNEL1_ENABLED	; Enable channel 1
	inc SetSoundVars
	inc RegA
	jmp l0374

l0366:	sty RegA
	cpx SetSoundVars
	bne l0374
	sty SetSoundVars
	stx RegA

l0374:	lda SetSoundVars+1
	cmp SetSoundDuration+1
	bne :+
	lda MUSIC_ON
	sta CHANNEL2_ENABLED	; Enable channel 1
	jmp l0392
:
	lda #$0
	sta CHANNEL2_ENABLED	; Enable channel 2
	lda SetSoundVars+1
	cmp SetSoundDuration+1
	bcs l0384
	inc SetSoundVars+1
	inc RegB
	jmp l0392

l0384:	sty RegB
	cpx SetSoundVars+1
	bne l0392
	sty SetSoundVars+1
	stx RegB

l0392:	lda SetSoundVars+2
	cmp SetSoundDuration+2
	bne :+
	lda MUSIC_ON
	sta CHANNEL3_ENABLED	; Enable channel 1
	jmp l03b0
:
	lda #$0
	sta CHANNEL3_ENABLED	; Enable channel 3
	lda SetSoundVars+2
	cmp SetSoundDuration+2
	bcs l03a2 
	inc SetSoundVars+2
	dec RegC
	jmp l03b0

l03a2:	sty RegC
	cpx SetSoundVars+2
	bne l03b0
	sty SetSoundVars+2
	stx RegC

l03b0:	lda SetSoundVars+3
	cmp SetSoundDuration+3
	bne :+
	lda MUSIC_ON
	sta CHANNEL4_ENABLED	; Enable channel 1
	jmp l03d3
:

	; lda SetSoundVars+3
	; cmp SetSoundDuration+3
	; beq l03e1	

	lda #$0
	sta CHANNEL4_ENABLED	; Enable channel 4

	lda SetSoundVars+3
    cmp SetSoundDuration+3 ; $03b3 $03b4
    bcs l03d3

	inc SetSoundVars+3
	lda SetSoundVars+3
	cmp #$16
	bne l03c8
	lda #$b0
	sta RegD
	beq l03e1
l03c8:	cmp #$2b
	bne l03e1
	lda #$a0
	sta RegD
	beq l03e1
l03d3:	sty RegD
	cpx SetSoundVars+3
	bne l03e1
	sty SetSoundVars+3
	stx RegD

l03e1:	rts

;----------------------------------------------------
; Set video registers
;----------------------------------------------------

SetReg:
	sta mode
	asl a
	clc
	adc crtflag
	tax
	lda centreX,x
	sta Reg0
	lda centreY,x
	sta Reg1
	lda scrW,x
	sta Reg2
	lda scrH,x
	sta Reg3
	lda RAMset,x
	sta Reg5
	rts

centreX:	.byte $04,$0c,$06,$0e
centreY:	.byte $1b,$26,$18,$24
scrW:		.byte $96,$96,$14,$14
scrH:		.byte $17,$17,$19,$19
RAMset:		.byte $8c,$8c,$cc,$cc
BottomScr:	.byte $84,$a0,$84,$a0

;----------------------------------------------------
; Display functions
;
; DISPLAYON   resets CentreY
; DISPLAYOFF  locates screen below bottom
; DISPLAYUP   scrolls screen up until default CentreY
; DISPLAYDOWN scrolls screen down until below bottom
;----------------------------------------------------

dispon:
	lda mode
	asl a
	clc
	adc crtflag
	tax
	lda centreY,x
	sta Reg1
	rts

dispoff:
	lda mode
	asl a
	clc
	adc crtflag
	tax
	lda BottomScr,x
	sta Reg1
	rts

dispup:
	lda mode
	asl a
	clc
	adc crtflag
	tax
	lda centreY,x
	sta tmp
scr_down_loop:
	lda #1
	jsr delay
	dec Reg1
	lda Reg1
	cmp tmp
	bne scr_down_loop
	rts

dispdown:
	lda mode
	asl a
	clc
	adc crtflag
	tax
	lda #1
	jsr delay
	inc Reg1
	lda Reg1
	cmp BottomScr,x
	bne dispdown
	rts
