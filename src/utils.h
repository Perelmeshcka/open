#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <raylib.h>

#define EPS 1e-9

typedef double   dbl;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct iv2 {
  dbl x;
  dbl y;
} iv2;

typedef struct iv3 {
  dbl x;
  dbl y;
  dbl z;
} iv3;

typedef struct trigon
{
  float sin[360];
  float cos[360];
} trigon;
trigon T;

typedef struct wall
{
  iv3   a;
  iv3   b;
  dbl   h;
  Color c;
  u32   sec;
} wall;

typedef struct sector
{
  iv3   *b;
  wall  *w;
  u32   len;
  u32   n;
  iv2   cen;
  Color tc;
  Color bc;
} sector;

typedef struct pos
{
  dbl x, y, z;
  int a;
  int l;
} pos;
pos P;

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

bool swap(void *a, void *b, u64 size);
dbl gety(iv2 a, iv2 b, dbl x);
bool between(dbl v, dbl e1, dbl e2);
void change(int *lo, int *hi, bool *lf, bool *hf, int val);
dbl cut(dbl val, dbl lo, dbl hi);
iv3 camcrd(iv3 t, pos cam);
iv2 calcproj(iv3 t, dbl f);
int angle(int deg);
void init(void);

typedef struct poly {
  iv3 *v;
  u32 len;
} poly;

poly clipseg(iv3 ac, iv3 bc);
poly clippol(iv3 *pol, u32 len);
bool equal(iv3 a, iv3 b);
dbl walldist(wall w);
iv3 through(iv2 a, iv2 b);
dbl lval(iv2 t, iv3 l);
iv2 cross(iv3 l, iv3 k);
