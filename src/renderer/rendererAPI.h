#pragma once
#include "asset_types.h"

#define UP_DIRECTION_VEC3 (vec3){{0, 1, 0}}

void rendererSetFPS(double fps);
void rendererSceneSet(Scene* scene);

void rendererEnableWireframe();
void rendererDisableWireframe();
void rendererWireframeToggle();