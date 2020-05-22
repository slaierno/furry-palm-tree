#pragma once

#include <cstdint>
#include "memory.hpp"

/*
 * Each frame is made by 8x8 tiles, for a total of WxH tiles, where
 * W = WIDTH/8
 * H = HEIGTH/8
 *          8p
 *      /--------/
 *    / +--------+--------+- - - - +--------+
 *    | |        |        |        |        |
 *    | |        |        |        |        |
 *    | |        |        |        |        |
 *  8 | |  Tile  |  Tile  |        |  Tile  |
 *  p | |  0,0   |  0,1   |        |  0,W-1 |
 *    | |        |        |        |        |
 *    | |        |        |        |        |
 *    | |        |        |        |        |
 *    / +------------------- - - - +--------+
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |  Tile  |        |        |  Tile  |
 *      |  1,0   |        |        |  1,W-1 |
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      +---------- - - - +- - - - +--------+
 *      |        |        |        |        |
 *  
 *      |        |        |        |        |
 *  
 *      |        |        |        |        |
 *  
 *      |        |        |        |        |
 *  
 *      +--------+--------+- - - - ---------+
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |  Tile  |  Tile  |        |  Tile  |
 *      |  H-1,0 |  H-1,1 |        | H-1,W-1|
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      |        |        |        |        |
 *      +--------+--------+--------+--------+
 */

/*
 * Each tile has 64 pixels, each represented by a single bit where
 * 0 : background color
 * 1 : foreground color
 * 
 * +---------------+
 * |0|0|0|1|1|0|0|0|      /: Background
 * +---------------+      #: Foreground
 * |0|1|1|0|0|1|1|0|
 * +---------------+        +--------+
 * |0|1|0|0|0|0|1|0|        |///##///|
 * +---------------+        |/##//##/|
 * |1|0|0|0|0|0|0|1|        |/#////#/|
 * +---------------+ +----> |#//////#|
 * |1|0|0|0|0|0|0|1|        |#//////#|
 * +---------------+        |/#////#/|
 * |0|1|0|0|0|0|1|0|        |/##//##/|
 * +---------------+        |///##///|
 * |0|1|1|0|0|1|1|0|        +--------+
 * +---------------+
 * |0|0|0|1|1|0|0|0|
 * +---------------+
 */

/* For each tile there are 2 bytes in memory which represent
 * background and foreground color.
 * The video-dedicated memory area is made like this:
 
    i := start_index
    for x in [0 .. H-1]
      for y in [0 .. W-1]
        memory[i++] := Tile[x][y] BG Color | Tile[x][y] FG Color
      for ln in 0 .. 7
        for px in [0, 2, 4 .. W-2]
          memory[i++] := Pixel[x*8 + ln][px..px+15]

 * Or, in english, we store for every tile line the tile color 
 * pairs for the, each pair stored in a single word.
 * After the tile color info, we store the pixel BG/FG info,
 * 16 pixels (1 bit per pixel) per word, for the next 8 lines
 */

/* The video memory needs to be (in words):
 * (WIDTH*HEIGTH)/16 + WIDTH*HEIGTH/(TILE_SIDE*TILE_SIDE) = 5000 words = 10000 bytes
 * 
 * We therefore reserve 0x1400 addresses. An idea is the area from 0xEA00 to 0xFDFF.
 * The new map should be:
 *
 *   0x0000 +----------------------+
 *          |  Trap Vector Table   |
 *   0x00FF +----------------------+
 *   0x0100 +----------------------+
 *          |Interrupt Vector Table|
 *   0x01FF +----------------------+
 *   0x0200 +----------------------+
 *          |                      |
 *          |                      |
 *          | Operating system and |
 *          =                      =
 *          |   Supervisor Stack   |
 *          |                      |
 *          |                      |
 *   0x2FFF +----------------------+
 *   0x3000 +----------------------+
 *          |                      |
 *          |                      |
 *          |                      |
 *          |    Available for     |
 *          =                      =
 *          |    user programs     |
 *          |                      |
 *          |                      |
 *          |                      |
 *   0xE9FF +----------------------+
 *   0xEA00 +----------------------+
 *          |     Video memory     |
 *          =                      =
 *          |      addresses       |
 *   0xFDFF +----------------------+
 *   0xFE00 +----------------------+
 *          |   Device register    |
 *          |      addresses       |
 *   0xFFFF +----------------------+
 * 
 * With a 320x200 screen, we are left with 120 extra words (240 extra bytes).
 * TODO: include extra info for sprites, palettes etc.
 */

#define BG(x) ((uint8_t)(x >> 8))
#define FG(x) ((uint8_t)(x & 0xFF))

#ifndef WIDTH
    #define WIDTH 320
#endif
#ifndef HEIGTH
    #define HEIGTH 200
#endif
#ifdef TILE_SIDE //should never happen
    #undef TILE_SIDE
#endif
#define TILE_SIDE 8
#define TWIDTH WIDTH/TILE_SIDE
#define THEIGTH HEIGTH/TILE_SIDE

struct LC3Screen {
    /* The next array represents the monitor screen */
    std::array<uint8_t, WIDTH*HEIGTH> screen;

    enum {
        BASE_VIDEO_MEMORY = 0xEA00
    };

    void mem_to_screen() {
        uint16_t address = BASE_VIDEO_MEMORY;
        uint16_t color_buffer[TWIDTH];
        for (int y = 0; y < HEIGTH; y++) {
            if((y % 8) == 0) {
                for(int x = 0; x < TWIDTH; x++) { //tile coordinates
                    color_buffer[x] = mem_read(address++);
                }
            }
            for(int x = 0; x < WIDTH; x+=16) {
                uint16_t pixel16 = mem_read(address++);
                int p = 15;
                for(; p >= 8; p--) {
                    const uint16_t color = color_buffer[x/8];
                    if((pixel16 >> p) & 0x1) {
                        screen[y*WIDTH + x + (15-p)] = FG(color);
                    } else {
                        screen[y*WIDTH + x + (15-p)] = BG(color);
                    }
                }
                for(; p >=0; p--) {
                    const uint16_t color = color_buffer[x/8+1];
                    if((pixel16 >> p) & 0x1) {
                        screen[y*WIDTH + x + (15-p)] = FG(color);
                    } else {
                        screen[y*WIDTH + x + (15-p)] = BG(color);
                    }
                }
            }
        }
    }
};