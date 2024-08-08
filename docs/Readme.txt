Game Boy Color Picker

A color picker that runs on Super Game Boy, Game Boy Color, and Game Boy Advance.

Colors on the Game Boy Color (GBC), Game Boy Advance, and Super Game Boy (Super Nintendo) are very different from typical monitors, and even from each other. It can take a surprising amount of time just to find colors that look good when run on original hardware and in emulators that try to be accurate to hardware. There are PC tools to help approximate what colors will look like under various conditions, but they are no substitute for seeing your choices as they will really look.

Game Boy Color Picker is a simple program that runs anywhere Game Boy games can run, including from a flash cart on GBC, Game Boy Advance, Super Game Boy, and emulators. It allows you to choose four colors at a time from the possible 32,768. As you change the colors you can see them in GBC/SNES 15 bit format, ready to use in your project and also in HTML color code format, ready to use in a graphics program.

Controls are: Left and right move between the three components (red, green, and blue) of four different colors, and Up and Down change them with the effect immediately visible. Select switches between HTML color code display and decimal display of colors.

The selected palette is saved to SRAM so your .sav file can be used by programs that read binary GBC palettes.

Game Boy Color Picker is written in C and compiles with GBDK. Source code is available at https://github.com/xenophile127/GameBoyColorPicker

What's new in this release (2024-01-30):

* Press Select to show decimal RGB values.
* Super Game Boy support.
* Save the palette in the .sav file.

-xenophile 2024-01-30