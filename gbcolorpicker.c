#include <gb/gb.h>
#include <gb/bcd.h>
#include <gb/cgb.h>
#include <gb/sgb.h>
#include <gbdk/console.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define PALETTE_COLOR_1 0U
#define PALETTE_COLOR_2 1U
#define PALETTE_COLOR_3 2U
#define PALETTE_COLOR_4 3U

#define TILE_ID_HIGHLIGHT1 0x66U
#define TILE_ID_HIGHLIGHT2 0x67U
#define TILE_ID_TOPNORMAL 0x68U
#define TILE_ID_TOPINVERT 0x69U
#define TILE_ID_COLOR 0x6aU

typedef uint8_t uint5_t;
#define UINT5_MIN (0x00U)
#define UINT5_MAX (0x1fU)

// Turns a five bit color value into an eight bit color value by repeating the three most significant bits in the three least.
#define EXTEND(byte) ((byte) << 3) | ((byte) >> 2)

// Wait some number of frames.
void wait(uint8_t frames)
{
    for (frames; frames != 0; --frames)
        wait_vbl_done();
}

// Convert a hex digit to a tile number.
uint8_t hex(const uint8_t nibble)
{
    if (nibble < 10) {
        return (nibble + 0x10);
    } else {
        return (nibble - 10 + 0x41);
    }
}

// SHow an ascii character at a given coordinate.
void show_char_xy(const uint8_t x, const uint8_t y, const char what)
{
    gotoxy(x, y);
    setchar(what);
}

// Show a byte in hexadecimal at the given coordinate and the one to its right.
void show_hex_byte_xy(const uint8_t x, const uint8_t y, const uint8_t byte)
{
    static uint8_t digit;
    
    digit = hex((byte >> 4) & 0xf);
    set_tile_xy(x, y, digit);

    digit = hex(byte & 0x0f);
    set_tile_xy(x+1, y, digit);

    // The font goes to the very top of the tile, so add a one px line across the top.
    set_tile_xy(x, y-1, TILE_ID_TOPNORMAL);
    set_tile_xy(x+1, y-1, TILE_ID_TOPNORMAL);
}

// A highlighted tile is displayed inverted.
// To do this a copy is needed.
void transfer_highlighted_tile(const uint8_t src, const uint8_t dst)
{
    static uint8_t i, v;
    static uint8_t *p, *q;

    p = _VRAM9000 + 0x10 * src;
    q = _VRAM9000 + 0x10 * dst;

    for (i = 0; i < 0x10; i += 1) {
      v = get_vram_byte(p + i);
      set_vram_byte(q + i, ~v);
      i += 1;
      v = get_vram_byte(p + i);
      set_vram_byte(q + i, ~v);
    }
}

// Show a byte in hexadecimal at the given coordinate and the one to its right.
// It will be inverted showing as white digits on a black background.
void show_hex_byte_xy_highlighted(const uint8_t x, const uint8_t y, const uint8_t byte)
{
    static uint8_t digit;
    static uint8_t i, b;
    static uint8_t *p, *q;

    // Create highlighted tiles for the two digits that need to be displayed
    digit = hex((byte >> 4) & 0x0f);
    transfer_highlighted_tile(digit, TILE_ID_HIGHLIGHT1);

    digit = hex(byte & 0x0f);
    transfer_highlighted_tile(digit, TILE_ID_HIGHLIGHT2);

    // Use the tiles that were just created
    set_tile_xy(x, y, TILE_ID_HIGHLIGHT1);
    set_tile_xy(x+1, y, TILE_ID_HIGHLIGHT2);

    // The font goes to the very top of the tile, so add a one px line across the top.
    set_tile_xy(x, y-1, TILE_ID_TOPINVERT);
    set_tile_xy(x+1, y-1, TILE_ID_TOPINVERT);
}
        
// Print the passed in color formatted as a CGB 15 bit color.
inline void print_raw(const uint8_t x, const uint8_t y, const palette_color_t raw)
{
    show_hex_byte_xy(x, y, (raw >> 8) & 0xff);
    show_hex_byte_xy(x+2, y, raw & 0xff);
}

// Print the passed in color formatted as an HTML color code.
inline void print_html(const uint8_t x, const uint8_t y, const uint8_t *p)
{
    show_hex_byte_xy(x+0, y, EXTEND(p[0]));
    show_hex_byte_xy(x+2, y, EXTEND(p[1]));
    show_hex_byte_xy(x+4, y, EXTEND(p[2]));
}

// Print the passed in color formatted as decimal R, G, B.
inline void print_decimal(const uint8_t x, const uint8_t y, const uint8_t *p)
{
    static uint8_t i;
    static uint8_t extended;
    static BCD bcd;

    show_char_xy(x, y, 'R');
    show_char_xy(x, y+1, 'G');
    show_char_xy(x, y+2, 'B');

    for (i = 0; i < 3; i++) {
        // Extend from 5 bit to 8 bit and store as 16 bit for BCD.
        extended = (p[i] << 3) | (p[i] >> 2);

        // Built in BCD conversion is for uint16_t so this is a little inefficient.
        uint2bcd(extended, &bcd);

        // Hundreds place 
        set_tile_xy(x+1, y+i, hex((bcd >> 8) & 0x0f));
        // Tens place.
        // This cast is to convince the compiler to use one swp instruction instead of four 16 bit shifts.
        set_tile_xy(x+2, y+i, hex(((*((uint8_t*)(&bcd))) >> 4) & 0x0f));
        // Ones place
        set_tile_xy(x+3, y+i, hex(bcd & 0x0f));
    }

    // The font goes to the very top of the tile, so add a one px line across the top.
    set_tile_xy(x, y-1, TILE_ID_TOPNORMAL);
    set_tile_xy(x+1, y-1, TILE_ID_TOPNORMAL);
    set_tile_xy(x+2, y-1, TILE_ID_TOPNORMAL);
    set_tile_xy(x+3, y-1, TILE_ID_TOPNORMAL);
}

// Print the build date.
void print_date()
{
    gotoxy(5,16);
    puts("2024-01-05");
}

void main()
{
    static uint8_t sgb;
    static uint8_t sgb_pal01[] = {SGB_PAL_01 << 3 | 1, 0xff,0x7f, 0xff,0x7f, 0xff,0x7f, 0,0,  0x1f,0, 0xff,0x7f, 0,0, 0};
    static uint8_t sgb_pal23[] = {SGB_PAL_23 << 3 | 1, 0xff,0x7f, 0xe0,0x03, 0xff,0x7f, 0,0,  0,0x7c, 0xff,0x7f, 0,0, 0};

    // Defines color rectangles for the four quadrants of the screen.
    static const uint8_t const sgb_attr_blk[] = {SGB_ATTR_BLK << 3 | 2, 3, 1,1, 10,0,19,8, 1,2, 0,9,9,17, 1,3,\
                                                  10,9,19,17, 0,0,0,0,0,0,0,0,0,0,0,0};

    // Font tiles are loaded in by GBDK. That is most of what is needed, but there are a few more to load.
    static const uint8_t const toptile_invert[] = {0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0xff};
    static const uint8_t const toptile[] = {0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0, 0xff};
    static const uint8_t const colortile[] = {0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0, 0xff, 0};

    // Locations for the different info to print.
    static const uint8_t const loc_x[] = {1, 11, 1, 11};
    static const uint8_t const loc_y[] = {1, 1, 16, 16};
    static const uint8_t const raw_loc_x[] = {3, 13, 3, 13};
    static const uint8_t const raw_loc_y[] = {5, 5, 12, 12};
    static const uint8_t const html_loc_x[] = {2, 12, 2, 12};
    static const uint8_t const html_loc_y[] = {3, 3, 14, 14};
    static const uint8_t const decimal_loc_x[] = {3, 13, 3, 13};
    static const uint8_t const decimal_loc_y[] = {3, 3, 12, 12};

    static palette_color_t raw_colors[4];

    // Set default colors
    static uint5_t colors[][] = {
        {UINT5_MAX, UINT5_MAX, UINT5_MAX}, // white
        {UINT5_MAX, UINT5_MIN, UINT5_MIN}, // red
        {UINT5_MIN, UINT5_MAX, UINT5_MIN}, // green
        {UINT5_MIN, UINT5_MIN, UINT5_MAX}  // blue
    };

    static uint8_t selected_color, selected_component;
    static uint8_t new_buttons, old_buttons;
    static bool color_changed = false;
    static uint8_t i, j;
    static uint8_t *p;

    static uint8_t mode;

    // Load a couple tiles so the top of numbers do not touch the colors
    set_bkg_data(TILE_ID_TOPNORMAL, 1, toptile);
    set_bkg_data(TILE_ID_TOPINVERT, 1, toptile_invert);
    // Load a tile of solid color 1 for the background to avoid the SGB shared color.
    set_bkg_data(TILE_ID_COLOR, 1, colortile);


    // Turn everything on. Sprites are not used.
    SHOW_BKG;
    DISPLAY_ON;

    // PAL SGB requires a four frame delay before sending packets.
    if (!DEVICE_SUPPORTS_COLOR) {
        wait(4);
        sgb = sgb_check();
    }

    // Supports GBC, GBA, and SGB
    if (!DEVICE_SUPPORTS_COLOR && !sgb) {
        puts("\n\n  GB Color Picker\n  does not run\n  in B&W. Sorry!");
        print_date();
        return;
    }

    // The four color palettes are mostly setup by the color picker its self, but they need black and white for text
    for (i = PALETTE_COLOR_1; i <= PALETTE_COLOR_4; i++) {
        set_bkg_palette_entry(i, 2, RGB_WHITE);
        set_bkg_palette_entry(i, 3, RGB_BLACK);
    }

    // Display title and usage
    puts("\n  GB Color Picker\n  ---------------\n\n Use left/right to\n choose a component\n and up/down to\n change it.\n\n Select switches\n between hex and\n decimal colors.\n\n Press any button\n to continue.");
    print_date();
    do {
      wait(1);
    } while (joypad() == 0);
    wait(12);

    // clear screen.
    cls();

    // Initialize color areas. Each quadrant of the screen gets a different palette.
    if (sgb) {
        // Initial color values are baked into the sgb_pal data.
        sgb_transfer(sgb_pal01);
        sgb_transfer(sgb_pal23);
        sgb_transfer(sgb_attr_blk);
    } else {
        VBK_REG = 1;
        fill_rect( 0, 0, 10, 9, PALETTE_COLOR_1);
        fill_rect(10, 0, 10, 9, PALETTE_COLOR_2);
        fill_rect( 0, 9, 10, 9, PALETTE_COLOR_3);
        fill_rect(10, 9, 10, 9, PALETTE_COLOR_4);
        VBK_REG = 0;
        for (i = 0; i < 4; i++) {
            set_bkg_palette_entry(PALETTE_COLOR_1 + i, 1, RGB(colors[i][0], colors[i][1], colors[i][2]));
        }
    }

    // Fill everything with the tile that will show the selected colors.
    fill_rect( 0, 0, 20, 18, TILE_ID_COLOR);

    // Main loop
    while(true) {
        // For each of the four colors...
        for (i = 0; i < 4; i++) {
            // Calculate the GBC 15 bit blue-green-red ordered color
            raw_colors[i] = RGB(colors[i][0], colors[i][1], colors[i][2]);

            if (mode == 0) {
                // Display the hex value of the newly calculated color
                print_raw(raw_loc_x[i], raw_loc_y[i], raw_colors[i]);

                // Display it formatted as a 24 bit red-green-blue ordered hex value, like the commonly used HTML color codes
                print_html(html_loc_x[i], html_loc_y[i], colors[i]);
            } else {
                // Display it formatted as decimal components
                print_decimal(decimal_loc_x[i], decimal_loc_y[i], colors[i]);
            }

            // Display it as three separate 5 bit hex formatted components
            for (j = 0; j < 3; j++) {
                if ((i == selected_color) && (j == selected_component)) {
                    // If it is the selected component, give it a special palette
                    show_hex_byte_xy_highlighted(loc_x[i] + 3 * j, loc_y[i], colors[i][j]);
                } else {
                    show_hex_byte_xy(loc_x[i] + 3 * j, loc_y[i], colors[i][j]);
                }
            }
        }

        if (color_changed) {
            if (sgb) {
                switch (selected_color) {
                    case 0:
                    sgb_pal01[3] = raw_colors[0] & 0xff;
                    sgb_pal01[4] = (raw_colors[0] >> 8) & 0xff;
                    break;

                    case 1:
                    sgb_pal01[9] = raw_colors[1] & 0xff;
                    sgb_pal01[10] = (raw_colors[1] >> 8) & 0xff;
                    break;

                    case 2:
                    sgb_pal23[3] = raw_colors[2] & 0xff;
                    sgb_pal23[4] = (raw_colors[2] >> 8) & 0xff;
                    break;

                    case 3:
                    sgb_pal23[9] = raw_colors[3] & 0xff;
                    sgb_pal23[10] = (raw_colors[3] >> 8) & 0xff;
                    break;
                }
                if (selected_color <= 1) {
                    sgb_transfer(sgb_pal01);
                } else {
                    sgb_transfer(sgb_pal23);
                }
            } else {
            // Set the newly calculated color into the second entry of a palette. Each selected color gets its own palette.
            // Using the second entry because of SGB transparency.
                set_bkg_palette_entry(PALETTE_COLOR_1 + selected_color, 1, raw_colors[selected_color]);
            }
        }

        // A repeat delay
        // SGB is slow enough to change colors as it is.
        if (!sgb || !color_changed) {
            wait(1);
        }

        color_changed = false;

        // Add an initial delay before repeat when a button is newly pressed so you can make precise changes
        if (old_buttons != new_buttons) {
            old_buttons = new_buttons;
            for (i = 20; i != 0; --i) {
                if (joypad()) {
                    wait(1);
                } else {
                    old_buttons = 0;
                    break;
                }
            }
        }

        do {
            wait(1);
            new_buttons = joypad();
            new_buttons &= J_LEFT | J_RIGHT | J_UP | J_DOWN | J_SELECT; // Only D-pad and select are used
        } while (!(new_buttons || old_buttons));

        // D-pad is pressed. Update.
        switch (new_buttons) {
            case J_LEFT:
            // De-select the current component
            show_hex_byte_xy(loc_x[selected_color] + 3 * selected_component, loc_y[selected_color], colors[selected_color][selected_component]);

            // Cycle to select the previous color component or color
            if (selected_component > 0) {
                selected_component -= 1;
            } else {
                selected_component = 2;
                if (selected_color > 0) {
                    selected_color -= 1;
                } else {
                    selected_color = 3;
                }
            }

            // Highlight the new component
            show_hex_byte_xy_highlighted(loc_x[selected_color] + 3 * selected_component, loc_y[selected_color], colors[selected_color][selected_component]);

            // A slightly longer delay for switching components than changing a component
            wait(1);
            break;

            case J_RIGHT:
            // De-select the current component
            show_hex_byte_xy(loc_x[selected_color] + 3 * selected_component, loc_y[selected_color], colors[selected_color][selected_component]);

            // Cycle to select the previous color component or color
            if (selected_component < 2) {
                selected_component += 1;
            } else {
                selected_component = 0;
                if (selected_color < 3) {
                    selected_color += 1;
                } else {
                    selected_color = 0;
                }
            }

            // Highlight the new component
            show_hex_byte_xy_highlighted(loc_x[selected_color] + 3 * selected_component, loc_y[selected_color], colors[selected_color][selected_component]);

            // A slightly longer delay for switching components than changing a component
            wait(1);
            break;

            case J_UP:
            // Increment the currently selected component if not already maxed
            if (colors[selected_color][selected_component] < UINT5_MAX) {
                colors[selected_color][selected_component] += 1;
                color_changed = true;
            }
            break;

            case J_DOWN:
            // Decrement the currently selected component if not already zero
            if (colors[selected_color][selected_component] > UINT5_MIN) {
                colors[selected_color][selected_component] -= 1;
                color_changed = true;
            }
            break;

            case J_SELECT:
            // Switch what is displayed
            if (mode == 1) {
                mode = 0;
            } else {
                mode += 1;
            }
            // Clear upper left
            fill_rect(2, 2, 6, 4, TILE_ID_COLOR);

            // Clear upper right
            fill_rect(12, 2, 6, 4, TILE_ID_COLOR);

            // Clear lower left
            fill_rect(2, 11, 6, 4, TILE_ID_COLOR);

            // Clear lower right
            fill_rect(12, 11, 6, 4, TILE_ID_COLOR);

            break;
        }
    }
}
