#ifndef RENDERER_H
#define RENDERER_H

#define MAX_PORTAL_DEPTH 100

void r_drawframe(const CCamera * pcamera);

void r_drawcons();
void r_shutdown();
void r_init();

#endif
