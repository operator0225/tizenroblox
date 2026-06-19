#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct AInputEvent;
typedef struct AInputEvent AInputEvent;

struct AInputQueue;
typedef struct AInputQueue AInputQueue;

/* Event types */
#define AINPUT_EVENT_TYPE_KEY    1
#define AINPUT_EVENT_TYPE_MOTION 2

/* Key actions */
#define AKEY_EVENT_ACTION_DOWN    0
#define AKEY_EVENT_ACTION_UP      1
#define AKEY_EVENT_ACTION_MULTIPLE 2

/* Motion actions */
#define AMOTION_EVENT_ACTION_DOWN         0
#define AMOTION_EVENT_ACTION_UP           1
#define AMOTION_EVENT_ACTION_MOVE         2
#define AMOTION_EVENT_ACTION_CANCEL       3
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP   6

/* Axis */
#define AMOTION_EVENT_AXIS_X               0
#define AMOTION_EVENT_AXIS_Y               1
#define AMOTION_EVENT_AXIS_LTRIGGER       17
#define AMOTION_EVENT_AXIS_RTRIGGER       18
#define AMOTION_EVENT_AXIS_HAT_X          15
#define AMOTION_EVENT_AXIS_HAT_Y          16

/* Sources */
#define AINPUT_SOURCE_GAMEPAD   0x00000401
#define AINPUT_SOURCE_JOYSTICK  0x01000010
#define AINPUT_SOURCE_KEYBOARD  0x00000101
#define AINPUT_SOURCE_TOUCHSCREEN 0x00001002

/* Keycodes */
#define AKEYCODE_BUTTON_A      96
#define AKEYCODE_BUTTON_B      97
#define AKEYCODE_BUTTON_X      99
#define AKEYCODE_BUTTON_Y      100
#define AKEYCODE_BUTTON_L1     102
#define AKEYCODE_BUTTON_R1     103
#define AKEYCODE_BUTTON_L2     104
#define AKEYCODE_BUTTON_R2     105
#define AKEYCODE_BUTTON_START  108
#define AKEYCODE_BUTTON_SELECT 109
#define AKEYCODE_DPAD_UP       19
#define AKEYCODE_DPAD_DOWN     20
#define AKEYCODE_DPAD_LEFT     21
#define AKEYCODE_DPAD_RIGHT    22
#define AKEYCODE_BACK          4
#define AKEYCODE_HOME          3

int32_t AInputEvent_getType(const AInputEvent* event);
int32_t AInputEvent_getSource(const AInputEvent* event);
int32_t AKeyEvent_getAction(const AInputEvent* event);
int32_t AKeyEvent_getKeyCode(const AInputEvent* event);
int32_t AKeyEvent_getMetaState(const AInputEvent* event);
int32_t AMotionEvent_getAction(const AInputEvent* event);
float   AMotionEvent_getX(const AInputEvent* event, size_t pointer_index);
float   AMotionEvent_getY(const AInputEvent* event, size_t pointer_index);
float   AMotionEvent_getAxisValue(const AInputEvent* event, int32_t axis, size_t pointer_index);
size_t  AMotionEvent_getPointerCount(const AInputEvent* event);
int32_t AMotionEvent_getPointerId(const AInputEvent* event, size_t pointer_index);

void    AInputQueue_attachLooper(AInputQueue* queue, ALooper* looper, int ident,
                                 ALooper_callbackFunc callback, void* data);
void    AInputQueue_detachLooper(AInputQueue* queue);
int32_t AInputQueue_hasEvents(AInputQueue* queue);
int32_t AInputQueue_getEvent(AInputQueue* queue, AInputEvent** outEvent);
int32_t AInputQueue_preDispatchEvent(AInputQueue* queue, AInputEvent* event);
void    AInputQueue_finishEvent(AInputQueue* queue, AInputEvent* event, int handled);

typedef int (*ALooper_callbackFunc)(int fd, int events, void* data);

#ifdef __cplusplus
}
#endif
