#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ANativeWindow;
typedef struct ANativeWindow ANativeWindow;

enum {
    WINDOW_FORMAT_RGBA_8888   = 1,
    WINDOW_FORMAT_RGBX_8888   = 2,
    WINDOW_FORMAT_RGB_565     = 4,
};

typedef struct ARect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
} ARect;

void    ANativeWindow_acquire(ANativeWindow* window);
void    ANativeWindow_release(ANativeWindow* window);
int32_t ANativeWindow_getWidth(ANativeWindow* window);
int32_t ANativeWindow_getHeight(ANativeWindow* window);
int32_t ANativeWindow_getFormat(ANativeWindow* window);
int32_t ANativeWindow_setBuffersGeometry(ANativeWindow* window, int32_t width, int32_t height, int32_t format);
int32_t ANativeWindow_lock(ANativeWindow* window, void* outBuffer, ARect* inOutDirtyBounds);
int32_t ANativeWindow_unlockAndPost(ANativeWindow* window);
int32_t ANativeWindow_setBuffersTransform(ANativeWindow* window, int32_t transform);

#ifdef __cplusplus
}
#endif
