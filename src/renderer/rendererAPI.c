#include "rendererAPI.h"
#include "renderer.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>

extern RendererState internalStateRenderer;

void rendererSetFPS(double fps) {
    internalStateRenderer.targetFrameTime = 1 / (double)fps;
}
void rendererSetScene(Scene* scene) {
    internalStateRenderer.scene = scene;
}

void rendererEnableWireframe() {
    internalStateRenderer.useWireframe = true;
}
void rendererDisableWireframe() {
    internalStateRenderer.useWireframe = false;
}
void rendererWireframeToggle() {
    internalStateRenderer.useWireframe ^= true;
}

UIText* rendererUICreateUIText(vec3 position, vec4 color, float scale) {
    UIText* uiText = memalloc(sizeof(UIText), MEMORY_TAG_RENDERER_UI);
    uiText->position = position;
    uiText->color = vec4ColorToUInt32_t(color);
    uiText->scale = scale;
    uiText->characters = NULL;
    hashmap_put(internalStateRenderer.uiState.texts, (uint64_t)uiText, (uint64_t)uiText);
    return uiText;
}
void rendererUIDestroyUIText(UIText* uiText) {
    if (uiText->characters)
        darray_destroy(uiText->characters);
    hashmap_remove(internalStateRenderer.uiState.texts, (uint64_t)uiText);
    memfree(uiText, sizeof(UIText), MEMORY_TAG_RENDERER_UI);
}
void rendererUIPrint(UIText* uiText, const char* message, ...) {
    char output[MAX_UI_CHARACTERS];
    __builtin_va_list args;
    va_start(args, message);
    vsnprintf(output, MAX_UI_CHARACTERS, message, args);
    va_end(args);

    uint32_t textLength = strlen(output);

    if (textLength > uiText->allocatedCharCount) {
        if (uiText->text) memfree(uiText->text, uiText->allocatedCharCount + 1, MEMORY_TAG_RENDERER_UI);
        uiText->text = memalloc(textLength + 1, MEMORY_TAG_RENDERER_UI);
        uiText->textLength = textLength;
        uiText->allocatedCharCount = textLength;

        memcpy(uiText->text, output, textLength + 1);

        if (uiText->characters) {
            darray_get_state(uiText->characters)->capacity = uiText->allocatedCharCount;
            darray_get_state(uiText->characters)->length = uiText->allocatedCharCount;
            darray_destroy(uiText->characters);
        }
        uiText->characters = darray_create_resized_memoryTag(UICharacterInstance, textLength, MEMORY_TAG_RENDERER_UI);
    } else {
        uiText->textLength = textLength;
        memcpy(uiText->text, output, textLength + 1);
        darray_get_state(uiText->characters)->capacity = textLength;
        darray_get_state(uiText->characters)->length = textLength;
    }

    rendererUIRepositionText(uiText);
}

void rendererUIRepositionText(UIText* uiText) {
    vec2 size = {
        .x = 0.05 * 600.0 / platformGetPlatformState()->width * uiText->scale,
        .y = 0.09 * 600.0 / platformGetPlatformState()->height * uiText->scale
    };

    vec3 position = uiText->position;
    for (int i = 0; i < uiText->textLength; i++) {
        UICharacterInstance uiCharacterInstance = {
            .position = position,
            .color = uiText->color,
            .size = size,
            .character = uiText->text[i]
        };
        if (uiText->text[i] == '\n') {
            position.x = uiText->position.x;
            position.y += size.y;
            continue;
        }
        position.x += size.x;
        uiText->characters[i] = uiCharacterInstance;
    }
}
void rendererUIFixScale() {
    if (internalStateRenderer.uiState.prevWidth == platformGetPlatformState()->width &&
        internalStateRenderer.uiState.prevHeight == platformGetPlatformState()->height) return;
    internalStateRenderer.uiState.prevWidth = platformGetPlatformState()->width;
    internalStateRenderer.uiState.prevHeight = platformGetPlatformState()->height;
    UIText* uiText;
    hashmap_foreach(internalStateRenderer.uiState.texts, uiText) {
        rendererUIRepositionText(uiText);
    }
}