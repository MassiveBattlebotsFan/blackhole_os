$RADIX 16

;org 8000h
;prog: DB 05, 48, 14, 00, 05, 45, 14, 00, 05, 4C, 14
;DB 00, 14, 00, 05, 4F, 14, 00, 00, 00
;enddata: DB 0FFh

org 0F000h
init:ld hl,prnt
ld a,0C3h
out (38h),a ;page out rom
ld ix,0020h
ld (ix+0),a
ld (ix+1),l
ld (ix+2),h
ld ix,0008h
ld hl,trnsble
ld (ix+0),a
ld (ix+1),l
ld (ix+2),h
ld hl,inpt
ld ix,0018h
ld (ix+0),a
ld (ix+1),l
ld (ix+2),h
ld ix,0038h
ld (ix+0),a
ld (ix+1),00h
ld (ix+2),01h
;transpile the program
ld de,0100h
ld hl,8000h
rst 8h ;call transpiler
call start
ld a,1
out (30h),a
ret

trnsble:ld ix,trans
loop:
ld a,(hl)
cp 0FFh
jr z,cdisble
ld (tgt+2),a
tgt:ld a,(ix+0)
ld (de),a
inc hl
inc de
ld a,(hl)
ld (de),a
inc hl
inc de
jr loop
cdisble:
inc hl
ld a,(hl)
cp 0FFh
ret z
inc hl
ld c,a
ld b,0
ldir
jr loop
start:
ld a,0C9h
ld (de),a
ld a,040h
rst 20h ;print
ld de,0100h
ld a,000h
rst 38h
ret

prnt:
ld c,a
prtlp:
in a,(80h)
bit 1,a
jr z,prtlp
ld a,c
out (81h),a
ret

inpt:
in a,(80h)
bit 0,a
jr z,inpt
in a,(81h)
ret

trans: DB 00, 23, 2Bh, 2E, 26, 3E, 0C6, 0D6
DB 0EEh, 0F6h, 77, 7E, 36, 0F5, 0F1, 0E6, 0E9, 20, 28, 0FE, 0E7, 0DF
DB 06Fh, 67, 07Dh, 7Ch, 47, 78, 18, 0E5h, 0E1h, 0EBh, 0C9h, 80
DB 90, 30, 38, 00