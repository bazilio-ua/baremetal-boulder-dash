#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>
#include "data_sprites.h"

#define TILE_SIZE_IN_SPRITES 2
#define TILE_SIZE (SPRITE_SIZE*TILE_SIZE_IN_SPRITES)
#define BORDER_SIZE_IN_TILES 1
#define BORDER_SIZE (TILE_SIZE*BORDER_SIZE_IN_TILES)

// Viewport is the whole screen area except the border.
#define VIEWPORT_WIDTH 256
#define VIEWPORT_HEIGHT 192
#define VIEWPORT_WIDTH_IN_TILES (VIEWPORT_WIDTH/TILE_SIZE)
#define VIEWPORT_HEIGHT_IN_TILES (VIEWPORT_HEIGHT/TILE_SIZE)

#define VIEWPORT_X_MIN BORDER_SIZE
#define VIEWPORT_Y_MIN BORDER_SIZE
#define VIEWPORT_X_MAX (VIEWPORT_X_MIN + VIEWPORT_WIDTH - 1)
#define VIEWPORT_Y_MAX (VIEWPORT_Y_MIN + VIEWPORT_HEIGHT - 1)

#define VIEWPORT_X_MIN_IN_TILES BORDER_SIZE_IN_TILES
#define VIEWPORT_Y_MIN_IN_TILES BORDER_SIZE_IN_TILES
#define VIEWPORT_X_MAX_IN_TILES (VIEWPORT_X_MIN_IN_TILES + VIEWPORT_WIDTH_IN_TILES - 1)
#define VIEWPORT_Y_MAX_IN_TILES (VIEWPORT_Y_MIN_IN_TILES + VIEWPORT_HEIGHT_IN_TILES - 1)

#define BACKBUFFER_WIDTH (VIEWPORT_WIDTH + BORDER_SIZE*2)
#define BACKBUFFER_HEIGHT (VIEWPORT_HEIGHT + BORDER_SIZE*2)
#define BACKBUFFER_PIXELS (BACKBUFFER_WIDTH*BACKBUFFER_HEIGHT)
// 4 bits per pixel.
#define BACKBUFFER_BYTES (BACKBUFFER_PIXELS*sizeof(uint8_t)/2)

#define PALETTE_COLORS 5

#define WINDOW_SCALE 3
#define WINDOW_WIDTH (BACKBUFFER_WIDTH * WINDOW_SCALE)
#define WINDOW_HEIGHT (BACKBUFFER_HEIGHT * WINDOW_SCALE)

void initGraphics(HDC deviceContext);
void displayBackbuffer();
void setPixel(int x, int y, uint8_t colorIndex);
void drawFilledRect(int left, int top, int right, int bottom, uint8_t colorIndex);
void drawSprite(const Sprite sprite, int outRow, int outCol, uint8_t fgColor, uint8_t bgColor);
void drawTile(const Sprite spriteA, const Sprite spriteB, const Sprite spriteC, const Sprite spriteD,
              int tileRow, int tileCol, uint8_t fgColor, uint8_t bgColor);

#endif
