#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "data_sprites.h"
#include "data_caves.h"

// Developer options
#define DEV_IMMEDIATE_STARTUP 1
#define DEV_NEAR_OUTBOX 0
#define DEV_SINGLE_DIAMOND_NEEDED 0
#define DEV_CHEAP_BONUS_LIFE 0
#define DEV_CAMERA_DEBUGGING 0
#define DEV_SLOW_TICK_DURATION 0
#define DEV_QUICK_OUT_OF_TIME 0
#define DEV_SINGLE_LIFE 0

// Gameplay constants
#define START_CAVE CAVE_B
#define TICKS_PER_TURN 5
#define ROCKFORD_TURNS_TILL_BIRTH 12
#define CELL_COVER_TURNS 40
#define TILE_COVER_TICKS (32*TICKS_PER_TURN)
#define BONUS_LIFE_COST (DEV_CHEAP_BONUS_LIFE ? 5 : 500)
#define BONUS_LIFE_FLASHING_TURNS 10
#define MAX_LIVES 9
#define COVER_PAUSE 2
#define TICKS_PER_CAVE_SECOND (7*TICKS_PER_TURN)
#define OUT_OF_TIME_ON_TURNS 25
#define OUT_OF_TIME_OFF_TURNS 42
#define TURNS_TILL_GAME_RESTART 10
#define TURNS_TILL_EXITING_CAVE 12

// Keys
#define KEY_FIRE VK_SPACE
#define KEY_RIGHT VK_RIGHT
#define KEY_LEFT VK_LEFT
#define KEY_DOWN VK_DOWN
#define KEY_UP VK_UP
#define KEY_FAIL 'Q'
#define KEY_QUIT VK_ESCAPE

// Cave map consists of cells, each cell contains 4 (2x2) tiles
#define TILE_SIZE 8
#define CELL_SIZE (TILE_SIZE*2)

#define BORDER_SIZE CELL_SIZE
#define STATUS_BAR_HEIGHT CELL_SIZE

// Viewport is the whole screen area except the border
#define VIEWPORT_WIDTH 256
#define VIEWPORT_HEIGHT 192
#define VIEWPORT_LEFT BORDER_SIZE
#define VIEWPORT_TOP BORDER_SIZE
#define VIEWPORT_RIGHT (VIEWPORT_LEFT + VIEWPORT_WIDTH - 1)
#define VIEWPORT_BOTTOM (VIEWPORT_TOP + VIEWPORT_HEIGHT - 1)

// Playfield is the whole viewport except the status bar
#define PLAYFIELD_WIDTH VIEWPORT_WIDTH
#define PLAYFIELD_HEIGHT (VIEWPORT_HEIGHT - STATUS_BAR_HEIGHT)
#define PLAYFIELD_LEFT VIEWPORT_LEFT
#define PLAYFIELD_TOP (VIEWPORT_TOP + STATUS_BAR_HEIGHT)
#define PLAYFIELD_RIGHT (PLAYFIELD_LEFT + PLAYFIELD_WIDTH - 1)
#define PLAYFIELD_BOTTOM (PLAYFIELD_TOP + PLAYFIELD_HEIGHT - 1)

#define PLAYFIELD_HEIGHT_IN_TILES (PLAYFIELD_HEIGHT/TILE_SIZE)
#define PLAYFIELD_WIDTH_IN_TILES (PLAYFIELD_WIDTH/TILE_SIZE)

#define CAMERA_START_LEFT (PLAYFIELD_LEFT + 6*TILE_SIZE)
#define CAMERA_STOP_LEFT (PLAYFIELD_LEFT + 14*TILE_SIZE)
#define CAMERA_START_TOP (PLAYFIELD_TOP + 4*TILE_SIZE)
#define CAMERA_STOP_TOP (PLAYFIELD_TOP + 9*TILE_SIZE)
#define CAMERA_START_RIGHT (PLAYFIELD_RIGHT - 6*TILE_SIZE + 1)
#define CAMERA_STOP_RIGHT (PLAYFIELD_RIGHT - 13*TILE_SIZE + 1)
#define CAMERA_START_BOTTOM (PLAYFIELD_BOTTOM - 4*TILE_SIZE + 1)
#define CAMERA_STOP_BOTTOM (PLAYFIELD_BOTTOM - 9*TILE_SIZE + 1)

#define CAMERA_X_MIN 0
#define CAMERA_Y_MIN 0
#define CAMERA_X_MAX (CAVE_WIDTH*CELL_SIZE - PLAYFIELD_WIDTH)
#define CAMERA_Y_MAX (CAVE_HEIGHT*CELL_SIZE - PLAYFIELD_HEIGHT)

#define CAMERA_STEP TILE_SIZE

// Backbuffer has 4 bits per pixel
#define BACKBUFFER_WIDTH (VIEWPORT_WIDTH + BORDER_SIZE*2)
#define BACKBUFFER_HEIGHT (VIEWPORT_HEIGHT + BORDER_SIZE*2)
#define BACKBUFFER_BYTES (BACKBUFFER_WIDTH*BACKBUFFER_HEIGHT/2)

typedef enum {
  OBJ_SPACE = 0x00,
  OBJ_DIRT = 0x01,
  OBJ_BRICK_WALL = 0x02,
  OBJ_MAGIC_WALL = 0x03,
  OBJ_PRE_OUTBOX = 0x04,
  OBJ_FLASHING_OUTBOX = 0x05,
  OBJ_STEEL_WALL = 0x07,
  OBJ_FIREFLY_LEFT = 0x08,
  OBJ_FIREFLY_UP = 0x09,
  OBJ_FIREFLY_RIGHT = 0x0A,
  OBJ_FIREFLY_DOWN = 0x0B,
  OBJ_FIREFLY_LEFT_SCANNED = 0x0C,
  OBJ_FIREFLY_UP_SCANNED = 0x0D,
  OBJ_FIREFLY_RIGHT_SCANNED = 0x0E,
  OBJ_FIREFLY_DOWN_SCANNED = 0x0F,
  OBJ_BOULDER_STATIONARY = 0x10,
  OBJ_BOULDER_STATIONARY_SCANNED = 0x11,
  OBJ_BOULDER_FALLING = 0x12,
  OBJ_BOULDER_FALLING_SCANNED = 0x13,
  OBJ_DIAMOND_STATIONARY = 0x14,
  OBJ_DIAMOND_STATIONARY_SCANNED = 0x15,
  OBJ_DIAMOND_FALLING = 0x16,
  OBJ_DIAMOND_FALLING_SCANNED = 0x17,
  OBJ_EXPLODE_TO_SPACE_0 = 0x1B,
  OBJ_EXPLODE_TO_SPACE_1 = 0x1C,
  OBJ_EXPLODE_TO_SPACE_2 = 0x1D,
  OBJ_EXPLODE_TO_SPACE_3 = 0x1E,
  OBJ_EXPLODE_TO_SPACE_4 = 0x1F,
  OBJ_EXPLODE_TO_DIAMOND_0 = 0x20,
  OBJ_EXPLODE_TO_DIAMOND_1 = 0x21,
  OBJ_EXPLODE_TO_DIAMOND_2 = 0x22,
  OBJ_EXPLODE_TO_DIAMOND_3 = 0x23,
  OBJ_EXPLODE_TO_DIAMOND_4 = 0x24,
  OBJ_PRE_ROCKFORD_1 = 0x25,
  OBJ_PRE_ROCKFORD_2 = 0x26,
  OBJ_PRE_ROCKFORD_3 = 0x27,
  OBJ_PRE_ROCKFORD_4 = 0x28,
  OBJ_BUTTERFLY_DOWN = 0x30,
  OBJ_BUTTERFLY_LEFT = 0x31,
  OBJ_BUTTERFLY_UP = 0x32,
  OBJ_BUTTERFLY_RIGHT = 0x33,
  OBJ_BUTTERFLY_DOWN_SCANNED = 0x34,
  OBJ_BUTTERFLY_LEFT_SCANNED = 0x35,
  OBJ_BUTTERFLY_UP_SCANNED = 0x36,
  OBJ_BUTTERFLY_RIGHT_SCANNED = 0x37,
  OBJ_ROCKFORD = 0x38,
  OBJ_ROCKFORD_SCANNED = 0x39,
  OBJ_AMOEBA = 0x3A,
  OBJ_AMOEBA_SCANNED = 0x3B,
} Object;

#define CAVE_HEIGHT 22
#define CAVE_WIDTH 40
#define NUM_DIFFICULTY_LEVELS 5
#define NUM_RANDOM_OBJECTS 4

typedef struct {
  uint8_t caveNumber;
  uint8_t magicWallMillingTime; // also max amoeba time at 3% growth
  uint8_t initialDiamondValue;
  uint8_t extraDiamondValue;
  uint8_t randomiserSeed[NUM_DIFFICULTY_LEVELS];
  uint8_t diamondsNeeded[NUM_DIFFICULTY_LEVELS];
  uint8_t caveTime[NUM_DIFFICULTY_LEVELS];
  uint8_t backgroundColor1;
  uint8_t backgroundColor2;
  uint8_t foregroundColor;
  uint8_t unused[2];
  uint8_t randomObject[NUM_RANDOM_OBJECTS];
  uint8_t objectProbability[NUM_RANDOM_OBJECTS];
} CaveInfo;

typedef enum {
  CAVE_A,
  CAVE_B,
  CAVE_C,
  CAVE_D,
  INTERMISSION_1,
  CAVE_E,
  CAVE_F,
  CAVE_G,
  CAVE_H,
  INTERMISSION_2,
  CAVE_I,
  CAVE_J,
  CAVE_K,
  CAVE_L,
  INTERMISSION_3,
  CAVE_M,
  CAVE_N,
  CAVE_O,
  CAVE_P,
  INTERMISSION_4,
  CAVE_COUNT,
} CaveName;

typedef enum {
  COLOR_BLACK,
  COLOR_GRAY,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_YELLOW,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_PURPLE,
  COLOR_CYAN,
  COLOR_COUNT,
} Color;

typedef struct {
  Color boulderSteelWallOutboxFg;
  Color brickWallFg;
  Color brickWallBg;
  Color dirtFg;
  Color flyFg;
  Color flyBg;
} CaveColors;

typedef enum {
  TURN_LEFT,
  STRAIGHT_AHEAD,
  TURN_RIGHT,
} Turning;

//
// Global variables
//

uint8_t *backbuffer;
uint8_t map[CAVE_HEIGHT][CAVE_WIDTH];
CaveInfo *caveInfo;
int turnsSinceRockfordSeenAlive;
bool isOutOfTime;

//
//
//

#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(*array))

void debugPrint(char *format, ...) {
  va_list argptr;
  va_start(argptr, format);
  char str[1024];
  vsprintf_s(str, sizeof(str), format, argptr);
  va_end(argptr);
  OutputDebugString(str);
}

void setPixel(int x, int y, Color color) {
  assert(color < COLOR_COUNT);
  assert((color & 0xF0) == 0);

  int pixelOffset = y*BACKBUFFER_WIDTH + x;
  int byteOffset = pixelOffset / 2;

  assert(byteOffset >= 0 && byteOffset < BACKBUFFER_BYTES);

  Color oldColor = backbuffer[byteOffset];
  Color newColor;
  if (pixelOffset % 2 == 0) {
    newColor = (color << 4) | (oldColor & 0x0F);
  } else {
    newColor = (oldColor & 0xF0) | color;
  }
  backbuffer[byteOffset] = newColor;
}

void drawRect(int left, int top, int right, int bottom, Color color) {
  for (int x = left; x <= right; ++x) {
    setPixel(x, top, color);
    setPixel(x, bottom, color);
  }
  for (int y = top; y <= bottom; ++y) {
    setPixel(left, y, color);
    setPixel(right, y, color);
  }
}

void drawFilledRect(int left, int top, int right, int bottom, Color color) {
  for (int y = top; y <= bottom; ++y) {
    for (int x = left; x <= right; ++x) {
      setPixel(x, y, color);
    }
  }
}

void drawTile(uint8_t *tile, int dstX, int dstY, Color fgColor, Color bgColor, int vOffset) {
  for (uint8_t bmpY = 0; bmpY < TILE_SIZE; ++bmpY) {
    int y = dstY + bmpY;
    uint8_t byte = tile[(bmpY + vOffset) % TILE_SIZE];

    for (uint8_t bmpX = 0; bmpX < TILE_SIZE; ++bmpX) {
      int x = dstX + bmpX;
      uint8_t mask = 1 << ((TILE_SIZE-1) - bmpX);
      uint8_t bit = byte & mask;
      Color color = bit ? fgColor : bgColor;

      setPixel(x, y, color);
    }
  }
}

void drawSprite(uint8_t *sprite, int frame, int dstX, int dstY, Color fgColor, Color bgColor, int vOffset) {
  int frames = sprite[0];
  int size = sprite[1];
  int bytesPerFrame = size*size*TILE_SIZE;
  int bytesPerRow = size*TILE_SIZE;

  for (int row = 0; row < size; ++row) {
    for (int col = 0; col < size; ++col) {
      int x = dstX + col*TILE_SIZE;
      int y = dstY + row*TILE_SIZE;

      if (x >= VIEWPORT_LEFT && (x+TILE_SIZE-1) <= VIEWPORT_RIGHT &&
          y >= VIEWPORT_TOP && (y+TILE_SIZE-1) <= VIEWPORT_BOTTOM) {
        uint8_t *data = sprite + 2 + (frame%frames)*bytesPerFrame + row*bytesPerRow + col*TILE_SIZE;
        drawTile(data, x, y, fgColor, bgColor, vOffset);
      }
    }
  }
}

void nextRandom(int *randSeed1, int *randSeed2) {
  int tempRand1 = (*randSeed1 & 0x0001) * 0x0080;
  int tempRand2 = (*randSeed2 >> 1) & 0x007F;

  int result = (*randSeed2) + (*randSeed2 & 0x0001) * 0x0080;
  int carry = (result > 0x00FF);
  result = result & 0x00FF;

  result = result + carry + 0x13;
  carry = (result > 0x00FF);
  *randSeed2 = result & 0x00FF;

  result = *randSeed1 + carry + tempRand1;
  carry = (result > 0x00FF);
  result = result & 0x00FF;

  result = result + carry + tempRand2;
  *randSeed1 = result & 0x00FF;
}

void placeObjectLine(Object object, int row, int col, int length, int direction) {
  int ldx[8] = { 0,  1, 1, 1, 0, -1, -1, -1 };
  int ldy[8] = { -1, -1, 0, 1, 1,  1,  0, -1 };

  for (int i = 0; i < length; i++) {
    map[row + i*ldy[direction]][col + i*ldx[direction]] = object;
  }
}

void placeObjectFilledRect(Object object, int row, int col, int width, int height, Object fillObject) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      if ((j == 0) || (j == height - 1)) {
        map[row+j][col+i] = object;
      } else {
        map[row+j][col+i] = fillObject;
      }
    }
    map[row][col+i] = object;
    map[row+height-1][col+i] = object;
  }
}

void placeObjectRect(Object object, int row, int col, int width, int height) {
  for (int i = 0; i < width; i++) {
    map[row][col+i] = object;
    map[row+height-1][col+i] = object;
  }
  for (int i = 0; i < height; i++) {
    map[row+i][col] = object;
    map[row+i][col+width-1] = object;
  }
}

void decodeCave(uint8_t caveIndex) {
  uint8_t *caves[CAVE_COUNT] = {
    caveA, caveB, caveC, caveD, intermission1,
    caveE, caveF, caveG, caveH, intermission2,
    caveI, caveJ, caveK, caveL, intermission3,
    caveM, caveN, caveO, caveP, intermission4,
  };

  assert(caveIndex < CAVE_COUNT);

  caveInfo = (CaveInfo *)caves[caveIndex];

  // Clear out the map
  for (int row = 0; row < CAVE_HEIGHT; row++) {
    for (int col = 0; col < CAVE_WIDTH; col++) {
      map[row][col] = OBJ_STEEL_WALL;
    }
  }

  // Decode random map objects
  {
    int randSeed1 = 0;
    int randSeed2 = caveInfo->randomiserSeed[0];

    for (int row = 1; row < CAVE_HEIGHT; row++) {
      for (int col = 0; col < CAVE_WIDTH; col++) {
        Object object = OBJ_DIRT;
        nextRandom(&randSeed1, &randSeed2);
        for (int i = 0; i < NUM_RANDOM_OBJECTS; i++) {
          if (randSeed1 < caveInfo->objectProbability[i]) {
            object = caveInfo->randomObject[i];
          }
        }
        map[row][col] = object;
      }
    }
  }

  // Decode explicit map data
  {
    uint8_t *explicitData = caves[caveIndex] + sizeof(CaveInfo);
    int uselessTopBorderHeight = 2;

    for (int i = 0; explicitData[i] != 0xFF; i++) {
      Object object = (explicitData[i] & 0x3F);

      switch (3 & (explicitData[i] >> 6)) {
        case 0: {
          int col = explicitData[++i];
          int row = explicitData[++i] - uselessTopBorderHeight;
          map[row][col] = object;
          break;
        }
        case 1: {
          int col = explicitData[++i];
          int row = explicitData[++i] - uselessTopBorderHeight;
          int length = explicitData[++i];
          int direction = explicitData[++i];
          placeObjectLine(object, row, col, length, direction);
          break;
        }
        case 2: {
          int col = explicitData[++i];
          int row = explicitData[++i] - uselessTopBorderHeight;
          int width = explicitData[++i];
          int height = explicitData[++i];
          Object fill = explicitData[++i];
          placeObjectFilledRect(object, row, col, width, height, fill);
          break;
        }
        case 3: {
          int col = explicitData[++i];
          int row = explicitData[++i] - uselessTopBorderHeight;
          int width = explicitData[++i];
          int height = explicitData[++i];
          placeObjectRect(object, row, col, width, height);
          break;
        }
      }
    }
  }

  // Steel bounds
  placeObjectRect(OBJ_STEEL_WALL, 0, 0, CAVE_WIDTH, CAVE_HEIGHT);
}

bool isKeyDown(uint8_t virtKey) {
  return GetFocus() && (GetKeyState(virtKey) & 0x8000);
}

void explodeCell(int row, int col, bool toDiamonds, int explosionStage) {
  if (map[row][col] != OBJ_STEEL_WALL) {
    if (toDiamonds) {
      if (explosionStage == 0) {
        map[row][col] = OBJ_EXPLODE_TO_DIAMOND_0;
      } else {
        map[row][col] = OBJ_EXPLODE_TO_DIAMOND_1;
      }
    } else {
      if (explosionStage == 0) {
        map[row][col] = OBJ_EXPLODE_TO_SPACE_0;
      } else {
        map[row][col] = OBJ_EXPLODE_TO_SPACE_1;
      }
    }
  }
}

bool isObjectRound(Object object) {
  return object == OBJ_BOULDER_STATIONARY || object == OBJ_DIAMOND_STATIONARY || object == OBJ_BRICK_WALL;
}

bool isObjectExplosive(Object object) {
  return object == OBJ_ROCKFORD ||
    object == OBJ_FIREFLY_LEFT ||
    object == OBJ_FIREFLY_UP ||
    object == OBJ_FIREFLY_RIGHT ||
    object == OBJ_FIREFLY_DOWN ||
    object == OBJ_BUTTERFLY_DOWN ||
    object == OBJ_BUTTERFLY_LEFT ||
    object == OBJ_BUTTERFLY_UP ||
    object == OBJ_BUTTERFLY_RIGHT;
}

bool explodesToDiamonds(Object object) {
  assert(isObjectExplosive(object));
  return object == OBJ_BUTTERFLY_DOWN || object == OBJ_BUTTERFLY_LEFT || object == OBJ_BUTTERFLY_UP || object == OBJ_BUTTERFLY_RIGHT;
}

void updateBoulderAndDiamond(int row, int col, Object fallingScannedObj, Object stationaryScannedObj, bool isFalling) {
  if (map[row+1][col] == OBJ_SPACE) {
    map[row+1][col] = fallingScannedObj;
    map[row][col] = OBJ_SPACE;
  } else if (isObjectRound(map[row+1][col])) {
    // Try to roll off
    if (map[row][col-1] == OBJ_SPACE && map[row+1][col-1] == OBJ_SPACE) {
      // Roll left
      map[row][col-1] = fallingScannedObj;
      map[row][col] = OBJ_SPACE;
    } else if (map[row][col+1] == OBJ_SPACE && map[row+1][col+1] == OBJ_SPACE) {
      // Roll right
      map[row][col+1] = fallingScannedObj;
      map[row][col] = OBJ_SPACE;
    } else {
      map[row][col] = stationaryScannedObj;
    }
  } else if (isFalling && isObjectExplosive(map[row+1][col])) {
    bool toDiamonds = explodesToDiamonds(map[row+1][col]);

    explodeCell(row, col, toDiamonds, 1);
    explodeCell(row, col-1, toDiamonds, 1);
    explodeCell(row, col+1, toDiamonds, 0);

    explodeCell(row+1, col, toDiamonds, 0);
    explodeCell(row+1, col-1, toDiamonds, 0);
    explodeCell(row+1, col+1, toDiamonds, 0);

    explodeCell(row+2, col, toDiamonds, 0);
    explodeCell(row+2, col-1, toDiamonds, 0);
    explodeCell(row+2, col+1, toDiamonds, 0);
  } else {
    map[row][col] = stationaryScannedObj;
  }
}

bool isFailed() {
  return turnsSinceRockfordSeenAlive >= 16 || isOutOfTime;
}

bool checkFlyExplode(Object object) {
  return object == OBJ_ROCKFORD || object == OBJ_ROCKFORD_SCANNED || object == OBJ_AMOEBA;
}

void getNewFireflyPosition(int curRow, int curCol, Object curDirection, Turning turning, int *newRow, int *newCol, Object *newDirection) {
  *newRow = curRow;
  *newCol = curCol;

  switch(curDirection) {
    case OBJ_FIREFLY_UP:
      switch (turning) {
        case TURN_LEFT:      (*newCol)--; *newDirection = OBJ_FIREFLY_LEFT_SCANNED;  break;
        case STRAIGHT_AHEAD: (*newRow)--; *newDirection = OBJ_FIREFLY_UP_SCANNED;    break;
        case TURN_RIGHT:     (*newCol)++; *newDirection = OBJ_FIREFLY_RIGHT_SCANNED; break;
      }
      break;
    case OBJ_FIREFLY_DOWN:
      switch (turning) {
        case TURN_LEFT:      (*newCol)++; *newDirection = OBJ_FIREFLY_RIGHT_SCANNED; break;
        case STRAIGHT_AHEAD: (*newRow)++; *newDirection = OBJ_FIREFLY_DOWN_SCANNED;  break;
        case TURN_RIGHT:     (*newCol)--; *newDirection = OBJ_FIREFLY_LEFT_SCANNED;  break;
      }
      break;
    case OBJ_FIREFLY_LEFT:
      switch (turning) {
        case TURN_LEFT:      (*newRow)++; *newDirection = OBJ_FIREFLY_DOWN_SCANNED; break;
        case STRAIGHT_AHEAD: (*newCol)--; *newDirection = OBJ_FIREFLY_LEFT_SCANNED; break;
        case TURN_RIGHT:     (*newRow)--; *newDirection = OBJ_FIREFLY_UP_SCANNED;   break;
      }
      break;
    case OBJ_FIREFLY_RIGHT:
      switch (turning) {
        case TURN_LEFT:      (*newRow)--; *newDirection = OBJ_FIREFLY_UP_SCANNED;    break;
        case STRAIGHT_AHEAD: (*newCol)++; *newDirection = OBJ_FIREFLY_RIGHT_SCANNED; break;
        case TURN_RIGHT:     (*newRow)++; *newDirection = OBJ_FIREFLY_DOWN_SCANNED;  break;
      }
      break;
  }
}

LRESULT CALLBACK wndProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(wnd, msg, wparam, lparam);
  }
  return 0;
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmdLine, int cmdShow) {
  UNREFERENCED_PARAMETER(prevInst);
  UNREFERENCED_PARAMETER(cmdLine);

  WNDCLASS wndClass = {0};
  wndClass.style = CS_HREDRAW | CS_VREDRAW;
  wndClass.lpfnWndProc = wndProc;
  wndClass.hInstance = inst;
  wndClass.hCursor = LoadCursor(0, IDC_ARROW);
  wndClass.lpszClassName = "Boulder Dash";
  RegisterClass(&wndClass);

  int windowScale = 3;
  int windowWidth = BACKBUFFER_WIDTH * windowScale;
  int windowHeight = BACKBUFFER_HEIGHT * windowScale;

  RECT crect = {0};
  crect.right = windowWidth;
  crect.bottom = windowHeight;

  DWORD wndStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  AdjustWindowRect(&crect, wndStyle, 0);

  HWND wnd = CreateWindowEx(0, wndClass.lpszClassName, "Boulder Dash", wndStyle, 300, 100,
                            crect.right - crect.left, crect.bottom - crect.top,
                            0, 0, inst, 0);
  ShowWindow(wnd, cmdShow);
  UpdateWindow(wnd);

  //
  // Initialize graphics
  //

  HDC deviceContext = GetDC(wnd);
  backbuffer = malloc(BACKBUFFER_BYTES);

  BITMAPINFO *bitmapInfo = malloc(sizeof(BITMAPINFOHEADER) + (COLOR_COUNT * sizeof(RGBQUAD)));
  bitmapInfo->bmiHeader.biSize = sizeof(bitmapInfo->bmiHeader);
  bitmapInfo->bmiHeader.biWidth = BACKBUFFER_WIDTH;
  bitmapInfo->bmiHeader.biHeight = -BACKBUFFER_HEIGHT;
  bitmapInfo->bmiHeader.biPlanes = 1;
  bitmapInfo->bmiHeader.biBitCount = 4;
  bitmapInfo->bmiHeader.biCompression = BI_RGB;
  bitmapInfo->bmiHeader.biClrUsed = COLOR_COUNT;

  RGBQUAD black  = {0x00, 0x00, 0x00, 0x00};
  RGBQUAD red    = {0x00, 0x00, 0xCC, 0x00};
  RGBQUAD green  = {0x00, 0xCC, 0x00, 0x00};
  RGBQUAD yellow = {0x00, 0xCC, 0xCC, 0x00};
  RGBQUAD blue   = {0xCC, 0x00, 0x00, 0x00};
  RGBQUAD purple = {0xCC, 0x00, 0xCC, 0x00};
  RGBQUAD cyan   = {0xCC, 0xCC, 0x00, 0x00};
  RGBQUAD gray   = {0xCC, 0xCC, 0xCC, 0x00};
  RGBQUAD white  = {0xFF, 0xFF, 0xFF, 0x00};

  bitmapInfo->bmiColors[COLOR_BLACK] = black;
  bitmapInfo->bmiColors[COLOR_RED] = red;
  bitmapInfo->bmiColors[COLOR_GREEN] = green;
  bitmapInfo->bmiColors[COLOR_YELLOW] = yellow;
  bitmapInfo->bmiColors[COLOR_BLUE] = blue;
  bitmapInfo->bmiColors[COLOR_PURPLE] = purple;
  bitmapInfo->bmiColors[COLOR_CYAN] = cyan;
  bitmapInfo->bmiColors[COLOR_GRAY] = gray;
  bitmapInfo->bmiColors[COLOR_WHITE] = white;

  //
  // Clock
  //

  float dt = 0.0f;
  float targetFps = 60.0f;
  float maxDt = 1.0f / targetFps;
  LARGE_INTEGER perfcFreq = {0};
  LARGE_INTEGER perfc = {0};
  LARGE_INTEGER perfcPrev = {0};

  QueryPerformanceFrequency(&perfcFreq);
  QueryPerformanceCounter(&perfc);

  //
  // Initialize cave colors
  //

  CaveColors caveColors[CAVE_COUNT] = {0};

  caveColors[CAVE_A].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[CAVE_A].brickWallFg = COLOR_GRAY;
  caveColors[CAVE_A].brickWallBg = COLOR_RED;
  caveColors[CAVE_A].dirtFg = COLOR_RED;

  caveColors[CAVE_B].boulderSteelWallOutboxFg = COLOR_BLUE;
  caveColors[CAVE_B].brickWallFg = COLOR_BLUE;
  caveColors[CAVE_B].brickWallBg = COLOR_YELLOW;
  caveColors[CAVE_B].dirtFg = COLOR_PURPLE;
  caveColors[CAVE_B].flyFg = COLOR_BLUE;
  caveColors[CAVE_B].flyBg = COLOR_YELLOW;

  caveColors[CAVE_C].boulderSteelWallOutboxFg = COLOR_PURPLE;
  caveColors[CAVE_C].brickWallFg = COLOR_GRAY;
  caveColors[CAVE_C].brickWallBg = COLOR_PURPLE;
  caveColors[CAVE_C].dirtFg = COLOR_GREEN;

  caveColors[CAVE_D].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[CAVE_D].dirtFg = COLOR_BLUE;
  caveColors[CAVE_D].flyFg = COLOR_CYAN;
  caveColors[CAVE_D].flyBg = COLOR_BLACK;

  caveColors[INTERMISSION_1].boulderSteelWallOutboxFg = COLOR_PURPLE;
  caveColors[INTERMISSION_1].dirtFg = COLOR_BLUE;
  caveColors[INTERMISSION_1].flyFg = COLOR_BLUE;
  caveColors[INTERMISSION_1].flyBg = COLOR_BLACK;

  caveColors[CAVE_E].boulderSteelWallOutboxFg = COLOR_PURPLE;
  caveColors[CAVE_E].dirtFg = COLOR_RED;
  caveColors[CAVE_E].flyFg = COLOR_RED;
  caveColors[CAVE_E].flyBg = COLOR_YELLOW;

  caveColors[CAVE_F].boulderSteelWallOutboxFg = COLOR_PURPLE;
  caveColors[CAVE_F].brickWallFg = COLOR_PURPLE;
  caveColors[CAVE_F].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_F].dirtFg = COLOR_BLUE;
  caveColors[CAVE_F].flyFg = COLOR_PURPLE;
  caveColors[CAVE_F].flyBg = COLOR_GRAY;

  caveColors[CAVE_G].boulderSteelWallOutboxFg = COLOR_PURPLE;
  caveColors[CAVE_G].brickWallFg = COLOR_RED;
  caveColors[CAVE_G].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_G].dirtFg = COLOR_RED;
  caveColors[CAVE_G].flyFg = COLOR_PURPLE;
  caveColors[CAVE_G].flyBg = COLOR_YELLOW;

  caveColors[CAVE_H].boulderSteelWallOutboxFg = COLOR_CYAN;
  caveColors[CAVE_H].brickWallFg = COLOR_CYAN;
  caveColors[CAVE_H].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_H].dirtFg = COLOR_RED;
  caveColors[CAVE_H].flyFg = COLOR_BLUE;
  caveColors[CAVE_H].flyBg = COLOR_CYAN;

  caveColors[INTERMISSION_2].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[INTERMISSION_2].dirtFg = COLOR_BLUE;
  caveColors[INTERMISSION_2].flyFg = COLOR_RED;
  caveColors[INTERMISSION_2].flyBg = COLOR_GRAY;

  caveColors[CAVE_I].boulderSteelWallOutboxFg = COLOR_BLUE;
  caveColors[CAVE_I].brickWallFg = COLOR_BLUE;
  caveColors[CAVE_I].brickWallBg = COLOR_CYAN;
  caveColors[CAVE_I].dirtFg = COLOR_PURPLE;

  caveColors[CAVE_J].boulderSteelWallOutboxFg = COLOR_RED;
  caveColors[CAVE_J].brickWallFg = COLOR_BLUE;
  caveColors[CAVE_J].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_J].dirtFg = COLOR_BLUE;
  caveColors[CAVE_J].flyFg = COLOR_RED;
  caveColors[CAVE_J].flyBg = COLOR_GRAY;

  caveColors[CAVE_K].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[CAVE_K].brickWallFg = COLOR_GRAY;
  caveColors[CAVE_K].brickWallBg = COLOR_YELLOW;
  caveColors[CAVE_K].dirtFg = COLOR_GREEN;
  caveColors[CAVE_K].flyFg = COLOR_GREEN;
  caveColors[CAVE_K].flyBg = COLOR_GRAY;

  caveColors[CAVE_L].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[CAVE_L].brickWallFg = COLOR_YELLOW;
  caveColors[CAVE_L].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_L].dirtFg = COLOR_GREEN;
  caveColors[CAVE_L].flyFg = COLOR_GREEN;
  caveColors[CAVE_L].flyBg = COLOR_GRAY;

  caveColors[INTERMISSION_3].boulderSteelWallOutboxFg = COLOR_GREEN;
  caveColors[INTERMISSION_3].flyFg = COLOR_GREEN;
  caveColors[INTERMISSION_3].flyBg = COLOR_RED;

  caveColors[CAVE_M].boulderSteelWallOutboxFg = COLOR_RED;
  caveColors[CAVE_M].brickWallFg = COLOR_BLACK;
  caveColors[CAVE_M].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_M].dirtFg = COLOR_BLUE;
  caveColors[CAVE_M].flyFg = COLOR_YELLOW;
  caveColors[CAVE_M].flyBg = COLOR_BLACK;

  caveColors[CAVE_N].boulderSteelWallOutboxFg = COLOR_YELLOW;
  caveColors[CAVE_N].dirtFg = COLOR_PURPLE;
  caveColors[CAVE_N].flyFg = COLOR_YELLOW;
  caveColors[CAVE_N].flyBg = COLOR_BLACK;

  caveColors[CAVE_O].boulderSteelWallOutboxFg = COLOR_BLUE;
  caveColors[CAVE_O].brickWallFg = COLOR_BLACK;
  caveColors[CAVE_O].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_O].dirtFg = COLOR_PURPLE;
  caveColors[CAVE_O].flyFg = COLOR_BLUE;
  caveColors[CAVE_O].flyBg = COLOR_GRAY;

  caveColors[CAVE_P].boulderSteelWallOutboxFg = COLOR_CYAN;
  caveColors[CAVE_P].brickWallFg = COLOR_PURPLE;
  caveColors[CAVE_P].brickWallBg = COLOR_GRAY;
  caveColors[CAVE_P].dirtFg = COLOR_RED;
  caveColors[CAVE_P].flyFg = COLOR_PURPLE;
  caveColors[CAVE_P].flyBg = COLOR_GRAY;

  caveColors[INTERMISSION_4].boulderSteelWallOutboxFg = COLOR_GREEN;
  caveColors[INTERMISSION_4].brickWallFg = COLOR_GRAY;
  caveColors[INTERMISSION_4].brickWallBg = COLOR_GREEN;
  caveColors[INTERMISSION_4].dirtFg = COLOR_YELLOW;

  //
  // Initialize game
  //

  bool cellCover[CAVE_HEIGHT][CAVE_WIDTH];
  bool tileCover[PLAYFIELD_HEIGHT_IN_TILES][PLAYFIELD_WIDTH_IN_TILES];
  char statusBarText[PLAYFIELD_WIDTH_IN_TILES];
  CaveColors currentCaveColors;

  int turn = 0;
  int tick = 0;
  float tickTimer = 0;

  bool isGameStart = true;
  int turnsTillGameRestart = 0;
  int turnsTillExitingCave = 0;
  bool isAddingTimeToScore = false;

  Color normalBorderColor = COLOR_BLACK;
  Color flashBorderColor = COLOR_GRAY;
  Color borderColor = normalBorderColor;

  int cameraX = 0;
  int cameraY = 0;
  int cameraVelX = 0;
  int cameraVelY = 0;

  // These variables are initialized when game starts

  bool isCaveStart;
  int pauseTurnsLeft;
  uint8_t currentCaveNumber;
  int difficultyLevel;
  int livesLeft;
  int score;
  int scoreTillBonusLife;
  int turnsTillStopBonusLifeFlashing;

  // These variables are initialized when cave starts
  bool isExitingCave;
  int caveTimeLeft;
  int ticksTillNextCaveSecond;
  bool isOutOfTimeTextShown;
  int outOfTimeTurn = 0;
  int diamondsCollected;
  int currentDiamondValue;
  int rockfordTurnsTillBirth;
  int cellCoverTurnsLeft;
  int tileCoverTicksLeft;
  int rockfordCol;
  int rockfordRow;
  bool rockfordIsBlinking;
  bool rockfordIsTapping;
  bool rockfordIsMoving;
  bool rockfordIsFacingRight;

  //
  // Game loop
  //

  bool gameIsRunning = true;

  while (gameIsRunning) {
    perfcPrev = perfc;
    QueryPerformanceCounter(&perfc);
    dt = (float)(perfc.QuadPart - perfcPrev.QuadPart) / (float)perfcFreq.QuadPart;
    //debugPrint("dt: %f\n", dt);
    if (dt > maxDt) {
      dt = maxDt;
    }

    // Handle Windows messages
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      switch (msg.message) {
        case WM_QUIT:
          gameIsRunning = false;
          break;

        default:
          TranslateMessage(&msg);
          DispatchMessage(&msg);
          break;
      }
    }

    if (isKeyDown(KEY_QUIT)) {
      gameIsRunning = false;
    }

    if (isGameStart) {
      isGameStart = false;

      isCaveStart = true;
      pauseTurnsLeft = 0;
      currentCaveNumber = START_CAVE;
      difficultyLevel = 0;
      livesLeft = DEV_SINGLE_LIFE ? 1 : 3;
      score = 0;
      scoreTillBonusLife = BONUS_LIFE_COST;
      turnsTillStopBonusLifeFlashing = 0;
    }

    if (isCaveStart && pauseTurnsLeft == 0) {
      isCaveStart = false;

      decodeCave(currentCaveNumber);
      currentCaveColors = caveColors[currentCaveNumber];

      isExitingCave = false;
      turnsSinceRockfordSeenAlive = 0;
      diamondsCollected = 0;
      currentDiamondValue = caveInfo->initialDiamondValue;
      caveTimeLeft = DEV_QUICK_OUT_OF_TIME ? 5 : caveInfo->caveTime[difficultyLevel];
      ticksTillNextCaveSecond = TICKS_PER_CAVE_SECOND;
      isOutOfTime = false;
      isOutOfTimeTextShown = false;
      outOfTimeTurn = 0;
      rockfordTurnsTillBirth = DEV_IMMEDIATE_STARTUP ? 0 : ROCKFORD_TURNS_TILL_BIRTH;
      cellCoverTurnsLeft = DEV_IMMEDIATE_STARTUP ? 1 : CELL_COVER_TURNS;

      rockfordIsBlinking = false;
      rockfordIsTapping = false;
      tileCoverTicksLeft = 0;
      rockfordIsMoving = false;
      rockfordIsFacingRight = true;

      if (DEV_SINGLE_DIAMOND_NEEDED) {
        caveInfo->diamondsNeeded[difficultyLevel] = 1;
      }

      for (int row = 0; row < CAVE_HEIGHT; ++row) {
        for (int col = 0; col < CAVE_WIDTH; ++col) {
          cellCover[row][col] = true;
        }
      }

      for (int row = 0; row < PLAYFIELD_HEIGHT_IN_TILES; ++row) {
        for (int col = 0; col < PLAYFIELD_WIDTH_IN_TILES; ++col) {
          tileCover[row][col] = false;
        }
      }

      // Find initial rockford position
      for (int row = 0; row < CAVE_HEIGHT; ++row) {
        for (int col = 0; col < CAVE_WIDTH; ++col) {
          if (map[row][col] == OBJ_PRE_ROCKFORD_1) {
            rockfordRow = row;
            rockfordCol = col;
            if (DEV_NEAR_OUTBOX) {
              map[row-1][col] = OBJ_FLASHING_OUTBOX;
            }
          }
        }
      }
    }

    float tickDuration = DEV_SLOW_TICK_DURATION ? 0.15f : 0.03375f;
    tickTimer += dt;

    if (tickTimer >= tickDuration) {
      tickTimer -= tickDuration;
      tick++;

      int rockfordRectLeft = PLAYFIELD_LEFT + rockfordCol*CELL_SIZE - cameraX;
      int rockfordRectTop = PLAYFIELD_TOP + rockfordRow*CELL_SIZE - cameraY;
      int rockfordRectRight = rockfordRectLeft + CELL_SIZE;
      int rockfordRectBottom = rockfordRectTop + CELL_SIZE;

      if (isAddingTimeToScore) {
        if (caveTimeLeft > 0) {
          --caveTimeLeft;
          ++score;
        } else {
          isAddingTimeToScore = false;
          isExitingCave = true;
          ++currentCaveNumber;
          turnsTillExitingCave = TURNS_TILL_EXITING_CAVE;
        }
      } else {
        //
        // Update cave timer
        //

        if (turnsTillExitingCave == 0 && tileCoverTicksLeft == 0 && rockfordTurnsTillBirth == 0 && !isOutOfTime) {
          --ticksTillNextCaveSecond;
          if (ticksTillNextCaveSecond == 0) {
            ticksTillNextCaveSecond = TICKS_PER_CAVE_SECOND;
            if (caveTimeLeft > 0) {
              --caveTimeLeft;
            } else {
              isOutOfTime = true;
              isOutOfTimeTextShown = true;
            }
          }
        }

        //
        // Turn-based update logic
        //

        if (tick % TICKS_PER_TURN == 0) {
          if (pauseTurnsLeft > 0) {
            pauseTurnsLeft--;
          } else {
            turn++;

            borderColor = normalBorderColor;

            if (turnsTillStopBonusLifeFlashing > 0) {
              --turnsTillStopBonusLifeFlashing;
            }

            //
            // Move camera
            //

            if (rockfordRectRight > CAMERA_START_RIGHT) {
              cameraVelX = CAMERA_STEP;
            } else if (rockfordRectLeft < CAMERA_START_LEFT) {
              cameraVelX = -CAMERA_STEP;
            }
            if (rockfordRectBottom > CAMERA_START_BOTTOM) {
              cameraVelY = CAMERA_STEP;
            } else if (rockfordRectTop < CAMERA_START_TOP) {
              cameraVelY = -CAMERA_STEP;
            }

            if (rockfordRectLeft >= CAMERA_STOP_LEFT && rockfordRectRight <= CAMERA_STOP_RIGHT) {
              cameraVelX = 0;
            }
            if (rockfordRectTop >= CAMERA_STOP_TOP && rockfordRectBottom <= CAMERA_STOP_BOTTOM) {
              cameraVelY = 0;
            }

            cameraX += cameraVelX;
            cameraY += cameraVelY;

            if (cameraX < CAMERA_X_MIN) {
              cameraX = CAMERA_X_MIN;
            } else if (cameraX > CAMERA_X_MAX) {
              cameraX = CAMERA_X_MAX;
            }

            if (cameraY < CAMERA_Y_MIN) {
              cameraY = CAMERA_Y_MIN;
            } else if (cameraY > CAMERA_Y_MAX) {
              cameraY = CAMERA_Y_MAX;
            }

            //
            // Out of time
            //

            if (isOutOfTime) {
              ++outOfTimeTurn;
              if (isOutOfTimeTextShown) {
                if (outOfTimeTurn == OUT_OF_TIME_ON_TURNS) {
                  outOfTimeTurn = 0;
                  isOutOfTimeTextShown = false;
                }
              } else {
                if (outOfTimeTurn == OUT_OF_TIME_OFF_TURNS) {
                  outOfTimeTurn = 0;
                  isOutOfTimeTextShown = true;
                }
              }
            }

            //////////////////////

            if (turnsTillGameRestart > 0) {
              --turnsTillGameRestart;
              if (turnsTillGameRestart == 0) {
                isGameStart = true;
              }
            } else if (turnsTillExitingCave > 0) {
              --turnsTillExitingCave;
              if (turnsTillExitingCave == 0) {
                tileCoverTicksLeft = TILE_COVER_TICKS;
              }
            } else if (isExitingCave) {
              // Do nothing
            } else if (cellCoverTurnsLeft > 0) {
              //
              // Update cell cover
              //

              cellCoverTurnsLeft--;
              if (cellCoverTurnsLeft > 1) {
                for (int row = 0; row < CAVE_HEIGHT; ++row) {
                  for (int i = 0; i < 3; ++i) {
                    cellCover[row][rand()%CAVE_WIDTH] = false;
                  }
                }
              } else if (cellCoverTurnsLeft == 1) {
                pauseTurnsLeft = COVER_PAUSE;
              } else if (cellCoverTurnsLeft == 0) {
                for (int row = 0; row < CAVE_HEIGHT; ++row) {
                  for (int col = 0; col < CAVE_WIDTH; ++col) {
                    cellCover[row][col] = false;
                  }
                }
              }
            } else {
              //
              // Scan cave
              //

              ++turnsSinceRockfordSeenAlive;

              for (int row = 0; row < CAVE_HEIGHT; ++row) {
                for (int col = 0; col < CAVE_WIDTH; ++col) {
                  switch (map[row][col]) {
                    case OBJ_PRE_ROCKFORD_1:
                      turnsSinceRockfordSeenAlive = 0;
                      if (rockfordTurnsTillBirth == 0) {
                        map[row][col] = OBJ_PRE_ROCKFORD_2;
                      } else if (cellCoverTurnsLeft == 0) {
                        rockfordTurnsTillBirth--;
                      }
                      break;

                    case OBJ_PRE_ROCKFORD_2:
                      turnsSinceRockfordSeenAlive = 0;
                      map[row][col] = OBJ_PRE_ROCKFORD_3;
                      break;

                    case OBJ_PRE_ROCKFORD_3:
                      turnsSinceRockfordSeenAlive = 0;
                      map[row][col] = OBJ_PRE_ROCKFORD_4;
                      break;

                    case OBJ_PRE_ROCKFORD_4:
                      turnsSinceRockfordSeenAlive = 0;
                      map[row][col] = OBJ_ROCKFORD;
                      break;

                      //
                      // Update Rockford
                      //

                    case OBJ_ROCKFORD: {
                      turnsSinceRockfordSeenAlive = 0;

                      int newRow = row;
                      int newCol = col;

                      rockfordIsMoving = false;

                      if (!isOutOfTime && tileCoverTicksLeft == 0) {
                        if (isKeyDown(KEY_RIGHT)) {
                          rockfordIsMoving = true;
                          rockfordIsFacingRight = true;
                          ++newCol;
                        } else if (isKeyDown(KEY_LEFT)) {
                          rockfordIsMoving = true;
                          rockfordIsFacingRight = false;
                          --newCol;
                        } else if (isKeyDown(KEY_DOWN)) {
                          rockfordIsMoving = true;
                          ++newRow;
                        } else if (isKeyDown(KEY_UP)) {
                          rockfordIsMoving = true;
                          --newRow;
                        }
                      }

                      bool actuallyMoved = false;

                      switch (map[newRow][newCol]) {
                        case OBJ_SPACE:
                        case OBJ_DIRT:
                          actuallyMoved = true;
                          break;

                        case OBJ_DIAMOND_STATIONARY:
                        case OBJ_DIAMOND_STATIONARY_SCANNED:
                          //
                          // Pick up a diamond
                          //

                          actuallyMoved = true;
                          score += currentDiamondValue;

                          // Check for bonus life
                          scoreTillBonusLife -= currentDiamondValue;
                          if (scoreTillBonusLife <= 0) {
                            scoreTillBonusLife += BONUS_LIFE_COST;
                            turnsTillStopBonusLifeFlashing = BONUS_LIFE_FLASHING_TURNS;
                            ++livesLeft;
                            if (livesLeft > MAX_LIVES) {
                              livesLeft = MAX_LIVES;
                            }
                          }

                          // Check if all the needed diamonds for this cave were collected
                          ++diamondsCollected;
                          if (diamondsCollected == caveInfo->diamondsNeeded[difficultyLevel]) {
                            currentDiamondValue = caveInfo->extraDiamondValue;
                            borderColor = flashBorderColor;
                          }
                          break;

                        case OBJ_BOULDER_STATIONARY:
                        case OBJ_BOULDER_STATIONARY_SCANNED:
                          // Pushing boulders
                          if (rand() % 4 == 0) {
                            if (isKeyDown(KEY_RIGHT) && map[newRow][newCol+1] == OBJ_SPACE) {
                              map[newRow][newCol+1] = OBJ_BOULDER_STATIONARY_SCANNED;
                              actuallyMoved = true;
                            } else if (isKeyDown(KEY_LEFT) && map[newRow][newCol-1] == OBJ_SPACE) {
                              map[newRow][newCol-1] = OBJ_BOULDER_STATIONARY_SCANNED;
                              actuallyMoved = true;
                            }
                          }
                          break;

                        case OBJ_FLASHING_OUTBOX:
                          actuallyMoved = true;
                          isAddingTimeToScore = true;
                          break;
                      }

                      if (actuallyMoved) {
                        if (isKeyDown(KEY_FIRE)) {
                          map[newRow][newCol] = OBJ_SPACE;
                        } else {
                          map[row][col] = OBJ_SPACE;
                          map[newRow][newCol] = OBJ_ROCKFORD_SCANNED;
                          rockfordRow = newRow;
                          rockfordCol = newCol;
                        }
                      }

                      //
                      // Update Rockford idle animation
                      //

                      if (rockfordIsMoving) {
                        rockfordIsBlinking = false;
                        rockfordIsTapping = false;
                      } else {
                        if (tick % 8 == 0) {
                          rockfordIsBlinking = rand() % 4 == 0;
                          if (rand() % 16 == 0) {
                            rockfordIsTapping = !rockfordIsTapping;
                          }
                        }
                      }
                      break;
                    }

                      //
                      // Update boulders and diamonds
                      //

                    case OBJ_BOULDER_STATIONARY:
                    case OBJ_BOULDER_FALLING:
                      updateBoulderAndDiamond(row, col, OBJ_BOULDER_FALLING_SCANNED, OBJ_BOULDER_STATIONARY_SCANNED, map[row][col] == OBJ_BOULDER_FALLING);
                      break;

                    case OBJ_DIAMOND_STATIONARY:
                    case OBJ_DIAMOND_FALLING:
                      updateBoulderAndDiamond(row, col, OBJ_DIAMOND_FALLING_SCANNED, OBJ_DIAMOND_STATIONARY_SCANNED, map[row][col] == OBJ_DIAMOND_FALLING);
                      break;

                      //
                      // Update explosion
                      //

                    case OBJ_EXPLODE_TO_SPACE_0: map[row][col] = OBJ_EXPLODE_TO_SPACE_1; break;
                    case OBJ_EXPLODE_TO_SPACE_1: map[row][col] = OBJ_EXPLODE_TO_SPACE_2; break;
                    case OBJ_EXPLODE_TO_SPACE_2: map[row][col] = OBJ_EXPLODE_TO_SPACE_3; break;
                    case OBJ_EXPLODE_TO_SPACE_3: map[row][col] = OBJ_EXPLODE_TO_SPACE_4; break;
                    case OBJ_EXPLODE_TO_SPACE_4: map[row][col] = OBJ_SPACE; break;

                    case OBJ_EXPLODE_TO_DIAMOND_0: map[row][col] = OBJ_EXPLODE_TO_DIAMOND_1; break;
                    case OBJ_EXPLODE_TO_DIAMOND_1: map[row][col] = OBJ_EXPLODE_TO_DIAMOND_2; break;
                    case OBJ_EXPLODE_TO_DIAMOND_2: map[row][col] = OBJ_EXPLODE_TO_DIAMOND_3; break;
                    case OBJ_EXPLODE_TO_DIAMOND_3: map[row][col] = OBJ_EXPLODE_TO_DIAMOND_4; break;
                    case OBJ_EXPLODE_TO_DIAMOND_4: map[row][col] = OBJ_DIAMOND_STATIONARY; break;

                      //
                      // Update out box
                      //

                    case OBJ_PRE_OUTBOX:
                      if (diamondsCollected >= caveInfo->diamondsNeeded[difficultyLevel]) {
                        map[row][col] = OBJ_FLASHING_OUTBOX;
                      }
                      break;

                      //
                      // Update firefly
                      //

                    case OBJ_FIREFLY_LEFT:
                    case OBJ_FIREFLY_UP:
                    case OBJ_FIREFLY_RIGHT:
                    case OBJ_FIREFLY_DOWN: {
                      if (checkFlyExplode(map[row-1][col]) || checkFlyExplode(map[row+1][col]) ||
                          checkFlyExplode(map[row][col-1]) || checkFlyExplode(map[row][col+1])) {
                        explodeCell(row-1, col-1, false, 1);
                        explodeCell(row-1, col+0, false, 1);
                        explodeCell(row-1, col+1, false, 1);

                        explodeCell(row+0, col-1, false, 1);
                        explodeCell(row+0, col+0, false, 1);
                        explodeCell(row+0, col+1, false, 0);

                        explodeCell(row+1, col-1, false, 0);
                        explodeCell(row+1, col+0, false, 0);
                        explodeCell(row+1, col+1, false, 0);
                      } else {
                        int newRow, newCol;
                        Object newDirection;
                        getNewFireflyPosition(row, col, map[row][col], TURN_LEFT, &newRow, &newCol, &newDirection);
                        if (map[newRow][newCol] == OBJ_SPACE) {
                          map[newRow][newCol] = newDirection;
                          map[row][col] = OBJ_SPACE;
                        } else {
                          getNewFireflyPosition(row, col, map[row][col], STRAIGHT_AHEAD, &newRow, &newCol, &newDirection);
                          if (map[newRow][newCol] == OBJ_SPACE) {
                            map[newRow][newCol] = newDirection;
                            map[row][col] = OBJ_SPACE;
                          } else {
                            getNewFireflyPosition(row, col, map[row][col], TURN_RIGHT, &newRow, &newCol, &newDirection);
                            map[row][col] = newDirection;
                          }
                        }
                      }
                      break;
                    }
                  }
                }
              }

              //
              // Remove scanned status for cells
              //

              for (int row = 0; row < CAVE_HEIGHT; ++row) {
                for (int col = 0; col < CAVE_WIDTH; ++col) {
                  switch (map[row][col]) {
                    case OBJ_FIREFLY_LEFT_SCANNED:       map[row][col] = OBJ_FIREFLY_LEFT;       break;
                    case OBJ_FIREFLY_UP_SCANNED:         map[row][col] = OBJ_FIREFLY_UP;         break;
                    case OBJ_FIREFLY_RIGHT_SCANNED:      map[row][col] = OBJ_FIREFLY_RIGHT;      break;
                    case OBJ_FIREFLY_DOWN_SCANNED:       map[row][col] = OBJ_FIREFLY_DOWN;       break;
                    case OBJ_BOULDER_STATIONARY_SCANNED: map[row][col] = OBJ_BOULDER_STATIONARY; break;
                    case OBJ_BOULDER_FALLING_SCANNED:    map[row][col] = OBJ_BOULDER_FALLING;    break;
                    case OBJ_DIAMOND_STATIONARY_SCANNED: map[row][col] = OBJ_DIAMOND_STATIONARY; break;
                    case OBJ_DIAMOND_FALLING_SCANNED:    map[row][col] = OBJ_DIAMOND_FALLING;    break;
                    case OBJ_BUTTERFLY_DOWN_SCANNED:     map[row][col] = OBJ_BUTTERFLY_DOWN;     break;
                    case OBJ_BUTTERFLY_LEFT_SCANNED:     map[row][col] = OBJ_BUTTERFLY_LEFT;     break;
                    case OBJ_BUTTERFLY_UP_SCANNED:       map[row][col] = OBJ_BUTTERFLY_UP;       break;
                    case OBJ_BUTTERFLY_RIGHT_SCANNED:    map[row][col] = OBJ_BUTTERFLY_RIGHT;    break;
                    case OBJ_ROCKFORD_SCANNED:           map[row][col] = OBJ_ROCKFORD;           break;
                    case OBJ_AMOEBA_SCANNED:             map[row][col] = OBJ_AMOEBA;             break;
                  }
                }
              }

              //
              // Handle failure
              //

              if (tileCoverTicksLeft == 0 && rockfordTurnsTillBirth == 0 &&
                  ((isFailed() && isKeyDown(KEY_FIRE)) || isKeyDown(KEY_FAIL))) {
                tileCoverTicksLeft = TILE_COVER_TICKS;
                --livesLeft;
              }
            }
          }
        }

        //
        // Update tile cover
        //

        if (tileCoverTicksLeft > 0) {
          --tileCoverTicksLeft;

          if (tileCoverTicksLeft == 0) {
            if (livesLeft == 0) {
              for (int row = 0; row < PLAYFIELD_HEIGHT_IN_TILES; ++row) {
                for (int col = 0; col < PLAYFIELD_WIDTH_IN_TILES; ++col) {
                  tileCover[row][col] = true;
                }
              }
              turnsTillGameRestart = TURNS_TILL_GAME_RESTART;
            } else {
              pauseTurnsLeft = COVER_PAUSE;
              isCaveStart = true;
            }
          } else {
            for (int i = 0; i < 7; ++i) {
              int row = rand() % PLAYFIELD_HEIGHT_IN_TILES;
              int col = rand() % PLAYFIELD_WIDTH_IN_TILES;
              tileCover[row][col] = true;
            }
          }
        }
      }

      //
      // Update status bar
      //

      if (livesLeft == 0) {
        sprintf_s(statusBarText, sizeof(statusBarText), "        G A M E  O V E R");
      } else if (isOutOfTimeTextShown && tileCoverTicksLeft == 0) {
        sprintf_s(statusBarText, sizeof(statusBarText), "     O U T   O F   T I M E");
      } else {
        if (rockfordTurnsTillBirth > 0 || tileCoverTicksLeft > 0 || isCaveStart) {
          sprintf_s(statusBarText, sizeof(statusBarText), "  PLAYER 1,  %d MEN,  ROOM %c/%d",
                    livesLeft, 'A' + currentCaveNumber, difficultyLevel+1);
        } else {
          if (diamondsCollected < caveInfo->diamondsNeeded[difficultyLevel]) {
            sprintf_s(statusBarText, sizeof(statusBarText), "   %02d*%02d   %02d   %03d   %06d",
                      caveInfo->diamondsNeeded[difficultyLevel],
                      currentDiamondValue, diamondsCollected, caveTimeLeft, score);
          } else {
            sprintf_s(statusBarText, sizeof(statusBarText), "   ***%02d   %02d   %03d   %06d",
                      currentDiamondValue, diamondsCollected, caveTimeLeft, score);
          }
        }
      }

      //
      // Render
      //

      // Draw border
      drawFilledRect(0, 0, BACKBUFFER_WIDTH - 1, BACKBUFFER_HEIGHT - 1, borderColor);

      // Draw cave
      for (int row = 0; row < CAVE_HEIGHT; ++row) {
        for (int col = 0; col < CAVE_WIDTH; ++col) {
          int x = PLAYFIELD_LEFT + col*CELL_SIZE - cameraX;
          int y = PLAYFIELD_TOP + row*CELL_SIZE - cameraY;

          if (cellCover[row][col]) {
            drawSprite(spriteSteelWall, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, turn);
          } else {
            switch (map[row][col]) {
              case OBJ_SPACE:
                if (turnsTillStopBonusLifeFlashing > 0) {
                  drawSprite(spriteSpaceFlash, turn, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                } else {
                  drawSprite(spriteSpace, 0, x, y, COLOR_BLACK, COLOR_BLACK, 0);
                }
                break;

              case OBJ_STEEL_WALL:
              case OBJ_PRE_OUTBOX:
                drawSprite(spriteSteelWall, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                break;

              case OBJ_FLASHING_OUTBOX:
                if (turn % 2 == 0) {
                  drawSprite(spriteOutbox, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                } else {
                  drawSprite(spriteSteelWall, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                }
                break;

              case OBJ_DIRT:
                drawSprite(spriteDirt, 0, x, y, currentCaveColors.dirtFg, COLOR_BLACK, 0);
                break;

              case OBJ_BRICK_WALL:
                drawSprite(spriteBrickWall, 0, x, y, currentCaveColors.brickWallFg, currentCaveColors.brickWallBg, 0);
                break;

              case OBJ_BOULDER_STATIONARY:
              case OBJ_BOULDER_FALLING:
                drawSprite(spriteBoulder, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                break;

              case OBJ_DIAMOND_STATIONARY:
              case OBJ_DIAMOND_FALLING:
                drawSprite(spriteDiamond, turn, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;

              case OBJ_FIREFLY_LEFT:
              case OBJ_FIREFLY_UP:
              case OBJ_FIREFLY_RIGHT:
              case OBJ_FIREFLY_DOWN:
                drawSprite(spriteFirefly, turn, x, y, currentCaveColors.flyFg, currentCaveColors.flyBg, 0);
                break;

                //
                // Draw Rockford birth
                //

              case OBJ_PRE_ROCKFORD_1:
                if (rockfordTurnsTillBirth > 0) {
                  if (rockfordTurnsTillBirth % 2) {
                    drawSprite(spriteSteelWall, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                  } else {
                    drawSprite(spriteOutbox, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, 0);
                  }
                } else {
                  drawSprite(spriteExplosion, 0, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                }
                break;
              case OBJ_PRE_ROCKFORD_2:
                drawSprite(spriteExplosion, 1, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
              case OBJ_PRE_ROCKFORD_3:
                drawSprite(spriteExplosion, 2, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
              case OBJ_PRE_ROCKFORD_4:
                drawSprite(spriteRockfordRight, turn, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                break;

                //
                // Draw rockford
                //

              case OBJ_ROCKFORD:
                if (rockfordIsMoving) {
                  if (rockfordIsFacingRight) {
                    drawSprite(spriteRockfordRight, tick, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                  } else {
                    drawSprite(spriteRockfordLeft, tick, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                  }
                } else if (rockfordIsBlinking && rockfordIsTapping) {
                  drawSprite(spriteRockfordBlinkTap, tick, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                } else if (rockfordIsBlinking) {
                  drawSprite(spriteRockfordBlink, tick, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                } else if (rockfordIsTapping) {
                  drawSprite(spriteRockfordTap, tick, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                } else {
                  drawSprite(spriteRockfordIdle, 0, x, y, COLOR_GRAY, COLOR_BLACK, 0);
                }
                break;

                //
                // Draw explosion
                //

              case OBJ_EXPLODE_TO_SPACE_1:
              case OBJ_EXPLODE_TO_DIAMOND_1:
                drawSprite(spriteExplosion, 1, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
              case OBJ_EXPLODE_TO_SPACE_2:
              case OBJ_EXPLODE_TO_DIAMOND_2:
                drawSprite(spriteExplosion, 2, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
              case OBJ_EXPLODE_TO_SPACE_3:
              case OBJ_EXPLODE_TO_DIAMOND_3:
                drawSprite(spriteExplosion, 1, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
              case OBJ_EXPLODE_TO_SPACE_4:
              case OBJ_EXPLODE_TO_DIAMOND_4:
                drawSprite(spriteExplosion, 0, x, y, COLOR_WHITE, COLOR_BLACK, 0);
                break;
            }
          }
        }
      }

      //
      // Draw tile cover
      //

      for (int row = 0; row < PLAYFIELD_HEIGHT_IN_TILES; ++row) {
        for (int col = 0; col < PLAYFIELD_WIDTH_IN_TILES; ++col) {
          if (tileCover[row][col]) {
            int x = PLAYFIELD_LEFT + col*TILE_SIZE;
            int y = PLAYFIELD_TOP + row*TILE_SIZE;
            drawSprite(spriteSteelWallTile, 0, x, y, currentCaveColors.boulderSteelWallOutboxFg, COLOR_BLACK, turn);
          }
        }
      }

      //
      // Draw status bar
      //

      {
        // Black background
        drawFilledRect(VIEWPORT_LEFT, VIEWPORT_TOP, VIEWPORT_RIGHT, VIEWPORT_TOP+STATUS_BAR_HEIGHT, COLOR_BLACK);

        int x = VIEWPORT_LEFT;
        int y = VIEWPORT_TOP + TILE_SIZE;

        for (int i = 0; statusBarText[i]; ++i) {
          drawSprite(spriteAscii, statusBarText[i]-' ', x + i*TILE_SIZE, y, COLOR_GRAY, COLOR_BLACK, 0);
        }
      }

      //
      // Camera debugging
      //

      if (DEV_CAMERA_DEBUGGING) {
        drawRect(CAMERA_START_LEFT, 0, CAMERA_START_LEFT, BACKBUFFER_HEIGHT-1, COLOR_WHITE);
        drawRect(CAMERA_STOP_LEFT, 0, CAMERA_STOP_LEFT, BACKBUFFER_HEIGHT-1, COLOR_WHITE);
        drawRect(CAMERA_START_RIGHT, 0, CAMERA_START_RIGHT, BACKBUFFER_HEIGHT-1, COLOR_WHITE);
        drawRect(CAMERA_STOP_RIGHT, 0, CAMERA_STOP_RIGHT, BACKBUFFER_HEIGHT-1, COLOR_WHITE);

        drawRect(0, CAMERA_START_TOP, BACKBUFFER_WIDTH-1, CAMERA_START_TOP, COLOR_WHITE);
        drawRect(0, CAMERA_STOP_TOP, BACKBUFFER_WIDTH-1, CAMERA_STOP_TOP, COLOR_WHITE);
        drawRect(0, CAMERA_START_BOTTOM, BACKBUFFER_WIDTH-1, CAMERA_START_BOTTOM, COLOR_WHITE);
        drawRect(0, CAMERA_STOP_BOTTOM, BACKBUFFER_WIDTH-1, CAMERA_STOP_BOTTOM, COLOR_WHITE);

        drawRect(rockfordRectLeft, rockfordRectTop, rockfordRectRight, rockfordRectBottom, COLOR_WHITE);
      }

      // Display backbuffer
      StretchDIBits(deviceContext,
                    0, 0, windowWidth, windowHeight,
                    0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT,
                    backbuffer, bitmapInfo,
                    DIB_RGB_COLORS, SRCCOPY);
    }
  }

  return 0;
}
