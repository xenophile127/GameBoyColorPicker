# If you move this project you can change the directory 
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
GBDK_HOME = ../../../

PROJECTNAME    = GBColorPicker

# Compile options
CC = $(GBDK_HOME)bin/lcc

# CLFAGS options:
CFLAGS += -Wm-yt3       # MBC1+RAM+BATTERY. MBC1 chosen for small size of SRAM (because only 8 bytes are needed).
CFLAGS += -Wm-ya1       # 8KiB of SRAM.
CFLAGS += -Wm-yc        # Game Boy Color support.
CFLAGS += -Wm-ys        # Super Game Boy support.
CFLAGS += -Wl-j -Wm-yS  # Generate a .sym file with labels in a format commonly supported by emulators.
CFLAGS += -Wf--max-allocs-per-node50000  # Recommended setting, but does slow down compilation considerably.

BINS	    = $(PROJECTNAME).gb
CSOURCES   := $(wildcard *.c)

all:	$(BINS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROJECTNAME).gb:	gbcolorpicker.o
	$(CC) $(CFLAGS) -Wm-yn"$(PROJECTNAME)" -o $@ $<

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
