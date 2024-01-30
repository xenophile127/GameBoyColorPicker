# If you move this project you can change the directory 
# to match your GBDK root directory (ex: GBDK_HOME = "C:/GBDK/"
GBDK_HOME = ../../../

PROJECTNAME    = GBColorPicker

# Compile options
CC = $(GBDK_HOME)bin/lcc

# Recommended setting, but does slow down compilation considerably.
CFLAGS = -Wf--max-allocs-per-node50000

BINS	    = $(PROJECTNAME).gb
CSOURCES   := $(wildcard *.c)

all:	$(BINS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROJECTNAME).gb:	gbcolorpicker.o
	$(CC) $(CFLAGS) -Wm-yn"$(PROJECTNAME)" -Wm-yc -Wm-ys -o $@ $<

clean:
	rm -f *.o *.lst *.map *.gb *.ihx *.sym *.cdb *.adb *.asm
