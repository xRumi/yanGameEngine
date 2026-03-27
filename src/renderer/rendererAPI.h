#pragma once
#include "asset_types.h"

void rendererSetFPS(double fps);
void rendererCameraReset();
void rendererCameraSetPosition(vec3 position);
void rendererAddEntity(Entity* entity);