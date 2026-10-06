#include "config.h"
#include "utils.h"
#include "draw.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <raylib.h>

void Move(int rot, int vel)
{
  if (IsKeyDown(KEY_LEFT))
    P.a = angle(P.a - rot);
  if (IsKeyDown(KEY_RIGHT))
    P.a = angle(P.a + rot);
  if (IsKeyDown(KEY_UP))
    P.l = angle(P.l + rot);
  if (IsKeyDown(KEY_DOWN))
    P.l = angle(P.l - rot);

  if (IsKeyDown(KEY_W)) {
    P.x += vel * T.sin[P.a];
    P.y += vel * T.cos[P.a];
  }
  if (IsKeyDown(KEY_S)) {
    P.x -= vel * T.sin[P.a];
    P.y -= vel * T.cos[P.a];
  }
  if (IsKeyDown(KEY_A)) {
    P.x -= vel * T.cos[P.a];
    P.y += vel * T.sin[P.a];
  }
  if (IsKeyDown(KEY_D)) {
    P.x += vel * T.cos[P.a];
    P.y -= vel * T.sin[P.a];
  }

  if (IsKeyDown(KEY_SPACE))
    P.z += vel;
  if (IsKeyDown(KEY_LEFT_SHIFT))
    P.z -= vel;
}

void DrawSegm(iv3 a, iv3 b, Color col)
{
  iv3 ac = camcrd(a, P);
  iv3 bc = camcrd(b, P);

  iv3 *clip = clipseg(ac, bc).v;

  if (clip == NULL)
    return;

  iv2 as = calcproj(clip[0], FOCUS);
  iv2 bs = calcproj(clip[1], FOCUS);
  free(clip);
  
  DrawBigLine(as, bs, col);
}

void DrawWall(wall w)
{
  iv3 wc[4] = {w.a, w.b, w.b, w.a};
  wc[2].z += w.h;
  wc[3].z += w.h;

  for (u32 i = 0; i < 4; ++i)
    wc[i] = camcrd(wc[i], P);

  poly clip = clippol(wc, 4);

  iv2 *proj = (iv2 *)calloc(clip.len, sizeof(iv2));
  for (u32 i = 0; i < clip.len; ++i)
    proj[i] = calcproj(clip.v[i], FOCUS);

  DrawBigPoly(proj, clip.len, w.c);
  free(clip.v);
  free(proj);
}

void DrawSec(sector sec)
{
  if (P.z >= sec.b[0].z && P.z <= sec.b[0].z + sec.w[0].h)
    return;
  
  iv3 *side = (iv3 *)calloc(sec.len, sizeof(iv3));
  for (u32 i = 0; i < sec.len; ++i)
    side[i] = sec.b[i];
  Color c = sec.bc;

  if (P.z > side[0].z + sec.w[0].h) {
    for (u32 i = 0; i < sec.len; ++i)
      side[i].z += sec.w[0].h;
    c = sec.tc;
  }

  for (u32 i = 0; i < sec.len; ++i)
    side[i] = camcrd(side[i], P);

  poly clip = clippol(side, sec.len);
  free(side);

  if (clip.v == NULL)
    return;

  iv2 *pol = (iv2 *)calloc(clip.len, sizeof(iv2));
  for (u32 i = 0; i < clip.len; ++i)
    pol[i] = calcproj(clip.v[i], FOCUS);

  DrawBigPoly(pol, clip.len, c);
  free(pol);
  free(clip.v);
}

typedef struct bspnode
{
  wall w;
  void *front;
  void *back;
} bspnode;

bspnode *root;

bspnode *bsp(u32 *idx, u32 len, wall *walls, u32 *wlen)
{
  if (len == 0)
    return NULL;
  
  bspnode *res = malloc(sizeof(bspnode));
  res->front = NULL;
  res->back  = NULL;

  u32 del = idx[rand() % len];
  res->w = walls[del];

  if (len == 1)
    return res;
  
  iv2 la = {(walls[del]).a.x, (walls[del]).a.y};
  iv2 lb = {(walls[del]).b.x, (walls[del]).b.y};
  iv3 l = through(la, lb);

  u32 *front = (u32 *)calloc(len, sizeof(u32));
  u32 *back  = (u32 *)calloc(len, sizeof(u32));
  u32 flen = 0, blen = 0;

  u32 wi;
  wall cur;
  iv2 a, b, m;
  bool ain, bin;
  dbl aval, bval, dx, dy, dz, ndx, ndy, ndz, mz;

  for (u32 i = 0; i < len; ++i) {
    wi = idx[i];
    if (wi == del)
      continue;
    
    cur = walls[wi];
    a = (iv2){ cur.a.x, cur.a.y };
    b = (iv2){ cur.b.x, cur.b.y };
    aval = lval(a, l);
    bval = lval(b, l);

    if (fabs(aval) < EPS && fabs(bval) < EPS) {
      ain = true;
      bin = true;
    } else if (fabs(aval) < EPS) {
      bin = bval > EPS;
      ain = bin;
    } else if (fabs(bval) < EPS) {
      ain = aval > EPS;
      bin = ain;
    } else {
      ain = aval > EPS;
      bin = bval > EPS;
    }

    if (ain && bin)
      front[flen++] = wi;
    if (!ain && !bin)
      back[blen++] = wi;
    if (ain != bin) {
      m = cross(l, through(a, b));
      walls[*wlen] = cur;
      ++(*wlen);

      dx = b.x - a.x;
      ndx = b.x - m.x;
      dy = b.y - a.y;
      ndy = b.y - m.y;

      dz = cur.b.z - cur.a.z;
      if (dx != 0)
        ndz = dz * ndx / dx;
      else
        ndz = dz * ndy / dy;
      mz = cur.b.z - ndz;

      walls[wi].b = (iv3){m.x, m.y, mz};
      walls[*wlen - 1].a = (iv3){m.x, m.y, mz};

      if (ain) {
        front[flen++] = wi;
        back[blen++]  = *wlen - 1;
      } else {
        front[flen++] = *wlen - 1;
        back[blen++]  = wi;
      }
    }
  }

  res->back  = bsp(back,  blen, walls, wlen);
  res->front = bsp(front, flen, walls, wlen);

  free(front);
  free(back);
  return res;
}

void freebsp(bspnode *node)
{
  if (node->back != NULL)
    freebsp(node->back);
  if (node->front != NULL)
    freebsp(node->front);
  free(node);
}

sector *secs;
u32 slen;

sector initsec(iv3 *b, u32 len, dbl h,
               Color *c, Color tc, Color bc, u32 n)
{
  wall *w = (wall *)calloc(len, sizeof(wall));
  for (u32 i = 0; i < len; ++i) {
    w[i] = (wall){
      .a   = b[i],
      .b   = b[(i + 1) % len],
      .h   = h,
      .c   = c[i],
      .sec = n,
    };
  }
  
  sector res = {
    .b   = b,
    .w   = w,
    .len = len,
    .bc  = bc,
    .tc  = tc,
  };

  return res;
}

void initwalls(void)
{
  slen = 2;
  secs = (sector *)calloc(slen, sizeof(sector));
  
  iv3 *b0 = (iv3 *)calloc(3, sizeof(iv3));
  b0[0] = (iv3){10, 10, 0};
  b0[1] = (iv3){100, 10, 0};
  b0[2] = (iv3){50, 100, 0};
  Color *c0 = (Color *)calloc(3, sizeof(Color));
  c0[0] = YELLOW;
  c0[1] = BLUE;
  c0[2] = ORANGE;
  secs[0] = initsec(b0, 3, 20, c0, PURPLE, GREEN, 0);
  free(c0);

  iv3 *b1 = (iv3 *)calloc(4, sizeof(iv3));
  b1[0] = (iv3){200, 10, 0};
  b1[1] = (iv3){260, 10, 0};
  b1[2] = (iv3){250, 90, 0};
  b1[3] = (iv3){200, 100, 0};
  Color *c1 = (Color *)calloc(4, sizeof(Color));
  c1[0] = YELLOW;
  c1[1] = BLUE;
  c1[2] = ORANGE;
  c1[3] = WHITE;
  secs[1] = initsec(b1, 4, 40, c1, DARKPURPLE, DARKBROWN, 1);
  free(c1);

  u32 wl = 0;
  wall *w = (wall *)calloc(100, sizeof(wall));

  u32 i, j;
  for (i = 0; i < slen; ++i)
    for (j = 0; j < secs[i].len; ++j)
      w[wl++] = secs[i].w[j];

  u32 *range = (u32 *)calloc(wl, sizeof(u32));
  for (i = 0; i < wl; ++i)
    range[i] = i;
  root = bsp(range, wl, w, &wl);
  free(range);
  free(w);
}

void DrawBSP(bspnode *node, u32 *u)
{
  if (node == NULL)
    return;

  wall w = node->w;
  iv3 wac = camcrd(w.a, P);
  iv3 wbc = camcrd(w.b, P);
  bool vis = atan2(wac.y, wac.x) > atan2(wbc.y, wbc.x);

  if (node->back == NULL && node->front == NULL) {
    if (vis)
      DrawWall(w);
    ++u[w.sec];
    // if (u[w.sec] == secs[w.sec].len)
      // DrawSec(secs[w.sec]);
    
    return;
  }

  iv2 p = {P.x, P.y};
  iv2 wa = {w.a.x, w.a.y};
  iv2 wb = {w.b.x, w.b.y};
  iv3 l = through(wa, wb);
  
  if (node->back == NULL) {
    if (lval(p, l) > EPS) {
      if (vis)
        DrawWall(w);
      ++u[w.sec];
      // if (u[w.sec] == secs[w.sec].len)
        // DrawSec(secs[w.sec]);
      DrawBSP(node->front, u);
    } else {
      DrawBSP(node->front, u);
      if (vis)
        DrawWall(w);
      ++u[w.sec];
      // if (u[w.sec] == secs[w.sec].len)
        // DrawSec(secs[w.sec]);
    }

    return;
  }
  
  if (lval(p, l) > EPS) {
    DrawBSP(node->back, u);
    if (vis)
      DrawWall(w);
    ++u[w.sec];
    // if (u[w.sec] == secs[w.sec].len)
      // DrawSec(secs[w.sec]);
    DrawBSP(node->front, u);
  } else {
    DrawBSP(node->front, u);
    if (vis)
      DrawWall(w);
    ++u[w.sec];
    // if (u[w.sec] == secs[w.sec].len)
      // DrawSec(secs[w.sec]);
    DrawBSP(node->back, u);
  }
}

void DrawScene(void)
{
  u32 *used = (u32 *)calloc(slen, sizeof(u32));
  for (u32 i = 0; i < slen; ++i)
    used[i] = 0;
  DrawBSP(root, used);
  free(used);
}

int main(void)
{
  InitWindow(PIX * WIDTH, PIX * HEIGHT, "open");
  SetTargetFPS(60);

  init();
  initwalls();
  
  while (!WindowShouldClose()) {
    Move(2, 10);
    
    BeginDrawing();
      ClearBackground(DARKBLUE);
      DrawScene();
    EndDrawing();
  }

  free(secs);
  freebsp(root);
  CloseWindow();
  return 0;
}