#ifndef DD_FOV_H
#define DD_FOV_H

#ifdef __cplusplus
extern "C" {
#endif

void dd_fovy_set(float val);
void dd_fovaspect_set(float val);

float dd_fovy_get();
float dd_fovaspect_get();

#ifdef __cplusplus
}
#endif

#endif
