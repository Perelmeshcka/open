#include "draw.h"
#include "config.h"
#include "utils.h"

#include <stdlib.h>
#include <raylib.h>

void DrawBigPixel(int x, int y, Color col) {
  DrawRectangle(PIX * x, PIX * y, PIX, PIX, col);
}

void DrawBigLine(iv2 a, iv2 b, Color col)
{
  int x1 = a.x, y1 = a.y, x2 = b.x, y2 = b.y;
  int x, y;
  
  if (x1 == x2) {
    for (y = min(y1, y2); y <= max(y1, y2); ++y)
      DrawBigPixel(x1, y, col);
    return;
  }

  if (abs(y2 - y1) < abs(x2 - x1)) {
    if (x1 > x2) {
      swap(&x1, &x2, sizeof(int));
      swap(&y1, &y2, sizeof(int));
    }
    
    for (x = x1; x <= x2; ++x) {
      y = x * (y2 - y1) / (x2 - x1) + y1 - x1 * (y2 - y1) / (x2 - x1);
      DrawBigPixel(x, y, col);
    }
  }

  else {
    if (y1 > y2) {
      swap(&x1, &x2, sizeof(int));
      swap(&y1, &y2, sizeof(int));
    }
    
    for (y = y1; y <= y2; ++y) {
      x = y * (x2 - x1) / (y2 - y1) + x1 - y1 * (x2 - x1) / (y2 - y1);
      DrawBigPixel(x, y, col);
    }
  }
}

void DrawBigPoly(iv2 *pol, u32 len, Color col)
{
  int lx = pol[0].x, hx = pol[0].x;

  int x, y, ly, hy;
  bool lf, hf;
  u32 i, j;

  for (i = 1; i < len; ++i) {
    if (pol[i].x < lx)
      lx = pol[i].x;
    if (pol[i].x > hx)
      hx = pol[i].x;
  }

  lx = cut(lx, 0, WIDTH - 1);
  hx = cut(hx, 0, HEIGHT - 1);
  
  // int ly = min(min(b1.y, b2.y), min(t1.y, t2.y));
  // int hy = max(max(b1.y, b2.y), max(t1.y, t2.y));

  for (x = lx; x <= hx; ++x) {
    lf = false;
    hf = false;

    for (i = 0; i < len; ++i) {
      j = (i + 1) % len;
      if (!between(x, pol[i].x, pol[j].x))
        continue;

      if (pol[i].x == pol[j].x) {
        change(&ly, &hy, &lf, &hf, pol[i].y);
        change(&ly, &hy, &lf, &hf, pol[j].y);
      } else {
        change(&ly, &hy, &lf, &hf, gety(pol[i], pol[j], x));
      }
    }

    if (!lf || !hf)
      continue;
    
    ly = cut(ly, -1, HEIGHT);
    hy = cut(hy, -1, HEIGHT);
    for (y = ly; y <= hy; ++y)
      DrawBigPixel(x, y, col);
  }
}
