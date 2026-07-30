#pragma once
#include "asset_types.h"
#include "renderer.h"

#define UP_DIRECTION_VEC3 (vec3){{0, 1, 0}}

void rendererSetFPS(double fps);
void rendererSetScene(Scene* scene);

void rendererEnableWireframe();
void rendererDisableWireframe();
void rendererWireframeToggle();

UIText* rendererUICreateUIText(vec3 position, vec4 color, float scale);
void rendererUIDestroyUIText(UIText* uIText);
void rendererUIPrint(UIText* uIText, const char* message, ...);
void rendererUIRepositionText(UIText* uiText);
void rendererUIFixScale();
