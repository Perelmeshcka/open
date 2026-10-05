#pragma once

#include "utils.h"

#include <raylib.h>

void DrawBigPixel(int x, int y, Color col);
void DrawBigLine(iv2 a, iv2 b, Color col);
void DrawBigPoly(iv2 *pol, u32 len, Color col);
