#ifndef RENDERER_H
#define RENDERER_H

#define MAX_PORTAL_DEPTH 100

void r_init();
void r_shutdown();

void r_drawframe(const CCamera * pcamera);

#endif
