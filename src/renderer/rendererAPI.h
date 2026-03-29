#pragma once
#include "asset_types.h"

#define UP_DIRECTION_VEC3 (vec3){{0, 1, 0}}
#define FRONT_DIRECTION_EULER_ANGLE_YXZ_VEC3 (vec3){{0, 0, -1}}

void rendererSetFPS(double fps);
void rendererCameraReset();
void rendererCameraSetPosition(vec3 position);

void rendererAddEntity(Entity* entity);

void rendererEnableWireframe();
void rendererDisableWireframe();
void rendererWireframeToggle();