#pragma once
#include <stdint.h>
#include <stddef.h>
#include "native_window.h"
#include "input.h"
#include "looper.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AAssetManager;
typedef struct AAssetManager AAssetManager;

typedef struct ANativeActivityCallbacks {
    void (*onStart)(struct ANativeActivity* activity);
    void (*onResume)(struct ANativeActivity* activity);
    void* (*onSaveInstanceState)(struct ANativeActivity* activity, size_t* outLen);
    void (*onPause)(struct ANativeActivity* activity);
    void (*onStop)(struct ANativeActivity* activity);
    void (*onDestroy)(struct ANativeActivity* activity);
    void (*onWindowFocusChanged)(struct ANativeActivity* activity, int hasFocus);
    void (*onNativeWindowCreated)(struct ANativeActivity* activity, ANativeWindow* window);
    void (*onNativeWindowResized)(struct ANativeActivity* activity, ANativeWindow* window);
    void (*onNativeWindowRedrawNeeded)(struct ANativeActivity* activity, ANativeWindow* window);
    void (*onNativeWindowDestroyed)(struct ANativeActivity* activity, ANativeWindow* window);
    void (*onInputQueueCreated)(struct ANativeActivity* activity, AInputQueue* queue);
    void (*onInputQueueDestroyed)(struct ANativeActivity* activity, AInputQueue* queue);
    void (*onContentRectChanged)(struct ANativeActivity* activity, const ARect* rect);
    void (*onConfigurationChanged)(struct ANativeActivity* activity);
    void (*onLowMemory)(struct ANativeActivity* activity);
} ANativeActivityCallbacks;

typedef struct ANativeActivity {
    ANativeActivityCallbacks* callbacks;
    void* vm;            /* JavaVM* */
    void* env;           /* JNIEnv* */
    void* clazz;         /* jobject activity */
    const char* internalDataPath;
    const char* externalDataPath;
    int32_t sdkVersion;
    void* instance;
    AAssetManager* assetManager;
    const char* obbPath;
} ANativeActivity;

typedef void ANativeActivity_createFunc(ANativeActivity* activity, void* savedState, size_t savedStateSize);
extern ANativeActivity_createFunc ANativeActivity_onCreate;

void ANativeActivity_finish(ANativeActivity* activity);
void ANativeActivity_setWindowFlags(ANativeActivity* activity, uint32_t addFlags, uint32_t removeFlags);
void ANativeActivity_setWindowFormat(ANativeActivity* activity, int32_t format);
void ANativeActivity_showSoftInput(ANativeActivity* activity, uint32_t flags);
void ANativeActivity_hideSoftInput(ANativeActivity* activity, uint32_t flags);

#ifdef __cplusplus
}
#endif
