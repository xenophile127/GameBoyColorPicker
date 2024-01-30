# If you move this project you can change the directory 
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
GBDK_HOME = ../../../

PROJECTNAME    = GBColorPicker

# Compile options
CC = $(GBDK_HOME)bin/lcc

# CLFAGS options:
# -yt3: MBC1+RAM+BATTERY. MBC1 chosen for SRAM support and small size of SRAM (because only 8 bytes are needed).
# -ya1: 8KiB of SRAM.
# -yc: Game Boy Color support.
# -ys: Super Game Boy support.
# --max-allocs-per-node50000: Recommended setting, but does slow down compilation considerably.
CFLAGS = -Wm-yt3 -Wm-ya1 -Wm-yc -Wm-ys -Wf--max-allocs-per-node50000

BINS	    = $(PROJECTNAME).gb
CSOURCES   := $(wildcard *.c)

all:	$(BINS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROJECTNAME).gb:	gbcolorpicker.o
	$(CC) $(CFLAGS) -Wm-yn"$(PROJECTNAME)" -o $@ $<

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
