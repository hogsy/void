#ifndef RENDERER_H
#define RENDERER_H

#include "World.h"

#define MAX_PORTAL_DEPTH 100

// renderer.cpp
void r_drawframe(vector_t *origin, vector_t *angles);
void r_drawcons();
void r_shutdown();
void r_init();

#endif
