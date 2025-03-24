-----------------------------------------------------------------------
MUSIC PLAYER
-----------------------------------------------------------------------

Create music player program and includes music data
Rules:
- Program should be loaded at $a900
- Program should start with 3 API calls
	jmp music_init
	jmp music_on
	jmp music_off
- API calls can be called from MPAGD with commands:
	MUSICINIT
	MUSICON
	MUSICOFF

Music player will be copied to the music folder
Change musicdata in player1.asm after label songpos:

	.incbin "songdata_pal.bin"

Type: build player1

Rename player1.prg to <gamename>.mus and copy it to:
	..\Suite VIC20 Bitmap\Music
E.g. music for slasher should be named slasher.mus

 
