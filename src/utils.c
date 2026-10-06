#include "config.h"
#include "utils.h"

#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool swap(void *a, void *b, u64 size)
{
  void *t = malloc(size);
  if (!t)
    return false;
  
  memcpy(a, t, size);
  memcpy(b, a, size);
  memcpy(t, b, size);
  free(t);
  
  return true;
}

dbl gety(iv2 a, iv2 b, dbl x) {
  return x * (b.y - a.y) / (b.x - a.x) + a.y - a.x * (b.y - a.y) / (b.x - a.x);
}

bool between(dbl v, dbl e1, dbl e2) {
  return v >= min(e1, e2) && v <= max(e1, e2);
}

void change(int *lo, int *hi, bool *lf, bool*hf, int val)
{
  if (!*lf || val < *lo) {
    *lf = true;
    *lo = val;
  }

  if (!*hf || val > *hi) {
    *hf = true;
    *hi = val;
  }
}

dbl cut(dbl val, dbl lo, dbl hi)
{
  if (val < lo)
    val = lo;
  if (val > hi)
    val = hi;
  return val;
}

iv3 camcrd(iv3 t, pos cam)
{
  t.x -= cam.x;
  t.y -= cam.y;
  t.z -= cam.z;

  dbl x = t.x, y = t.y, z = t.z;

  x = t.x * T.cos[cam.a] - t.y * T.sin[cam.a];
  y = t.x * T.sin[cam.a] + t.y * T.cos[cam.a];

  t.x = x;
  t.y = y;
  
  y = t.y * T.cos[cam.l] + t.z * T.sin[cam.l];
  z = -t.y * T.sin[cam.l] + t.z * T.cos[cam.l];

  t.y = y;
  t.z = z;
  
  return t;
}

iv2 calcproj(iv3 t, dbl f)
{
  iv2 res = {
    .x = (dbl)WIDTH / 2 + t.x * f / t.y,
    .y = (dbl)HEIGHT / 2 - t.z * f / t.y,
  };

  return res;
}

int angle(int deg)
{
  if (deg < 0)
    deg += 360;
  if (deg >= 360)
    deg -= 360;
  return deg;
}

void init(void)
{
  for (u32 d = 0; d < 360; ++d) {
    T.sin[d] = sin(d * M_PI / 180);
    T.cos[d] = cos(d * M_PI / 180);
  }

  P.x = 70;
  P.y = -110;
  P.z = 20;
  P.a = 0;
  P.l = 0;
}

poly clipseg(iv3 ac, iv3 bc)
{
  poly res = {
    .v   = NULL,
    .len = 0,
  };
  
  if (ac.y < NEAR && bc.y < NEAR)
    return res;

  if (ac.y < NEAR) {
    dbl dy = bc.y - ac.y, dx = bc.x - ac.x, dz = bc.z - ac.z;
    dbl ndy = bc.y - NEAR;
    dbl ndx = dx * ndy / dy;
    dbl ndz = dz * ndy / dy;

    ac.x = bc.x - ndx;
    ac.y = bc.y - ndy;
    ac.z = bc.z - ndz;
  }
  
  if (bc.y < NEAR) {
    dbl dy = ac.y - bc.y, dx = ac.x - bc.x, dz = ac.z - bc.z;
    dbl ndy = ac.y - NEAR;
    dbl ndx = dx * ndy / dy;
    dbl ndz = dz * ndy / dy;

    bc.x = ac.x - ndx;
    bc.y = ac.y - ndy;
    bc.z = ac.z - ndz;
  }

  res.v = (iv3 *)calloc(2, sizeof(iv3));
  res.v[0] = ac;
  res.v[1] = bc;
  res.len = 2;

  return res;
}

poly clippol(iv3 *pol, u32 len)
{
  iv3 a, b;
  bool ain, bin;
  iv3 *seg;

  poly res = {
    .v   = (iv3 *)calloc(len * 2, sizeof(iv3)),
    .len = 0,
  };
  
  for (u32 i = 0; i < len; ++i) {
    a = pol[i];
    b = pol[(i + 1) % len];

    ain = a.y >= NEAR;
    bin = b.y >= NEAR;

    if (!ain && !bin)
      continue;
    if (ain != bin) {
      seg = clipseg(a, b).v;
      if (seg == NULL)
        continue;
      a = seg[0];
      b = seg[1];
      free(seg);
    }
    
    if (res.len == 0 || !equal(a, res.v[res.len - 1]))
      res.v[res.len++] = a;
    if (res.len == 0 || !equal(b, res.v[0]))
      res.v[res.len++] = b;
  }

  return res;
}

bool equal(iv3 a, iv3 b) {
  return fabs(a.x - b.x) < EPS && fabs(a.y - b.y) < EPS && fabs(a.z - b.z) < EPS;
}

dbl walldist(wall w) {
  iv3 t1 = w.a, t2 = w.b;
  t1.z += w.h;
  t2.z += w.h;
  
  iv3 b1c = camcrd(w.a, P);
  iv3 b2c = camcrd(w.b, P);
  iv3 t1c = camcrd(t1, P);
  iv3 t2c = camcrd(t2, P);

  iv2 mid = {
    .x = (b1c.x + b2c.x + t1c.x + t2c.x) / 4,
    .y = (b1c.y + b2c.y + t1c.y + t2c.y) / 4,
  };
  
  return sqrt(mid.x * mid.x + mid.y * mid.y);
}

iv3 through(iv2 v, iv2 u)
{
  return (iv3){
    .x = u.y - v.y,
    .y = v.x - u.x,
    .z = v.y * u.x - v.x * u.y,
  };
}

dbl lval(iv2 t, iv3 l) {
  return l.x * t.x + l.y * t.y + l.z;
}

iv2 cross(iv3 l, iv3 k)
{
  dbl det = l.x * k.y - l.y * k.x;
  iv2 res;

  if (fabs(det) < EPS) {
    fputs("0 DIV", stderr);
    return (iv2){0, 0};
  }

  res = (iv2){
    .x = (l.y * k.z - l.z * k.y) / det,
    .y = (l.z * k.x - l.y * k.z) / det,
  };

  return res;
}