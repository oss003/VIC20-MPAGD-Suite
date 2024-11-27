;----------------------------------------------------------------------
; AGD 6502 Engine Zero Page Variables
;----------------------------------------------------------------------

; Variables start here.

scno:	.res 1			; present screen number.
numlif:	.res 1			; number of lives.

vara:	.res 1			; general-purpose variable.
varb:	.res 1			; general-purpose variable.
varc:	.res 1			; general-purpose variable.
vard:	.res 1			; general-purpose variable.
vare:	.res 1			; general-purpose variable.
varf:	.res 1			; general-purpose variable.
varg:	.res 1			; general-purpose variable.
varh:	.res 1			; general-purpose variable.
vari:	.res 1			; general-purpose variable.
varj:	.res 1			; general-purpose variable.
vark:	.res 1			; general-purpose variable.
varl:	.res 1			; general-purpose variable.
varm:	.res 1			; general-purpose variable.
varn:	.res 1			; general-purpose variable.
varo:	.res 1			; general-purpose variable.
varp:	.res 1			; general-purpose variable.
varq:	.res 1			; general-purpose variable.
varr:	.res 1			; general-purpose variable.
vars:	.res 1			; general-purpose variable.
vart:	.res 1			; general-purpose variable.
varu:	.res 1			; general-purpose variable.
varv:	.res 1			; general-purpose variable.
varw:	.res 1			; general-purpose variable.
varz:	.res 1			; general-purpose variable.

charx:	.res 1			; cursor x position.
chary:	.res 1			; cursor y position.

clock:	.res 1			; last clock reading.
varrnd:	.res 1	        ; last random number.
varobj:	.res 1  	    ; last object number.
varopt:	.res 1     		; last option chosen from menu.
varblk:	.res 1  		; block type.
nexlev:	.res 1			; next level flag.
restfl:	.res 1			; restart screen flag.
deadf:	.res 1			; dead flag.
gamwon:	.res 1			; game won flag.
dispx:	.res 1			; cursor x position.
dispy:	.res 1			; cursor y position.

dummy:  .res 5

contrl:	.res 1			; control 0 = keyboard, 1 = Joystick.
joyval:	.res 1			; joystick reading.
frmno:	.res 1			; selected frame.

;----------------------------------------------------
; Missing vars 
;----------------------------------------------------

loopa:	    .res 1
loopb:	    .res 1
loopc:	    .res 1
FontPtr:    .res 2

; Local vars

scraddr:    .res 2
fntaddr:    .res 2
tileaddr:   .res 2
bufaddr:    .res 2

tmp:        .res 2
.if sflag
scr_l:      .res 2
scr_r:      .res 2
scr_txt:    .res 2
scr_tmp:    .res 2
.endif

xtmp:	    .res 1
spcnt:	    .res 1
spptr:	    .res 2		; spawned sprite pointer.
seed:	    .res 1		; seed for random numbers.

ccnt:       .res 1
flag:	    .res 1
rcol:	    .res 1
rrow:	    .res 1

combyt:	    .res 1		; byte type compressed.
comcnt:	    .res 1		; compression counter.
prtmod:	    .res 1      ; print mode, 0 = standard, 1 = double-height.
qscnt:	    .res 1

sprptr:	    .res 1      ; not a ptr
sprcnt:	    .res 1

skptr:	    .res 2		; search pointer.
sktptr:	    .res 1      ; not a ptr
tmproom:    .res 1
ogptr:	    .res 2		; original sprite pointer.

spr:	    .res 3      ; static sprite data
vsync_count:.res 1
colpatt:    .res 1
sprtmp:     .res 1
sprtmp2:    .res 1
sprshft:    .res 1

spriteink:  .res 1
colour_byte:.res 1
colour_xor: .res 1
objcol:	    .res 1

.if mflag
TmpAddr:    .res 2
bwid:	    .res 1     ; box/menu width.
blen:	    .res 1     ; box/menu height.
btop:	    .res 1     ; box coordinates.
blft:	    .res 1
.endif

.if pflag
shrctr:	    .res 1
explcnt:    .res 1
seed3:	    .res 1
.endif

.if aflag
pbbuf:      .res 2
pbptr:      .res 2
.endif

tmp1:       .res 1
tmp2:       .res 1

SOUNDS_STATUS:      .res 1 

;----------------------------------------------------
; Music player variables
;----------------------------------------------------

;SONG_POS:		.res 1	; = $90
;PATTERN_POS:		.res 1	; = $91
;FRAME_COUNT:		.res 1	; = $92
;FRAME_LENGTH:		.res 1	; = $93
;SONG_NEXT_TRUE:		.res 1	; = $94
;SONG_END_TRUE:		.res 1	; = $95

; 2 byte address of pattern data
;CH1_PATTERN_POS:	.res 2	; = $96
;CH2_PATTERN_POS:	.res 2	; = $98
;CH3_PATTERN_POS:	.res 2	; = $9a
;CH4_PATTERN_POS:	.res 2	; = $9c

; #$00 == inactive mode
;CH1_ACTIVE:		.res 1	; = $a0
;CH2_ACTIVE:		.res 1	; = $a1
;CH3_ACTIVE:		.res 1	; = $a2
;CH4_ACTIVE:		.res 1	; = $a3

;TEMP_GUY:		.res 1	; = $a4
;CHAN_NOTE_DATA:		.res 1	; = $a5
;CHANS_INACTIVE:		.res 1	; = $a6
;FAST_COUNTER:		.res 1	; = $a7
;MEGA_COUNTER:		.res 1	; = $a8
