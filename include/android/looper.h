#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ALooper;
typedef struct ALooper ALooper;

enum {
    ALOOPER_POLL_WAKE     = -1,
    ALOOPER_POLL_CALLBACK = -2,
    ALOOPER_POLL_TIMEOUT  = -3,
    ALOOPER_POLL_ERROR    = -4,
};

enum {
    ALOOPER_EVENT_INPUT   = 1 << 0,
    ALOOPER_EVENT_OUTPUT  = 1 << 1,
    ALOOPER_EVENT_ERROR   = 1 << 2,
    ALOOPER_EVENT_HANGUP  = 1 << 3,
    ALOOPER_EVENT_INVALID = 1 << 4,
};

typedef int (*ALooper_callbackFunc)(int fd, int events, void* data);

ALooper* ALooper_prepare(int opts);
ALooper* ALooper_forThread(void);
void     ALooper_acquire(ALooper* looper);
void     ALooper_release(ALooper* looper);
int      ALooper_pollOnce(int timeoutMillis, int* outFd, int* outEvents, void** outData);
int      ALooper_pollAll(int timeoutMillis, int* outFd, int* outEvents, void** outData);
void     ALooper_wake(ALooper* looper);
int      ALooper_addFd(ALooper* looper, int fd, int ident, int events, ALooper_callbackFunc callback, void* data);
int      ALooper_removeFd(ALooper* looper, int fd);

#ifdef __cplusplus
}
#endif
