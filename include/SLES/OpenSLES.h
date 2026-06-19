#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t   SLuint8;
typedef uint16_t  SLuint16;
typedef uint32_t  SLuint32;
typedef int32_t   SLint32;
typedef int16_t   SLint16;
typedef uint8_t   SLboolean;
typedef SLuint32  SLmillibel;
typedef SLuint32  SLmillisecond;
typedef SLuint32  SLmicrosecond;
typedef SLuint32  SLresult;
typedef void*     SLObjectItf;
typedef void**    SLObjectItf_; /* pointer to interface */

#define SL_RESULT_SUCCESS           ((SLresult)0x00000000)
#define SL_RESULT_PRECONDITIONS_VIOLATED ((SLresult)0x00000001)
#define SL_RESULT_PARAMETER_INVALID ((SLresult)0x00000002)
#define SL_RESULT_MEMORY_FAILURE    ((SLresult)0x00000003)
#define SL_RESULT_RESOURCE_ERROR    ((SLresult)0x00000004)
#define SL_RESULT_RESOURCE_LOST     ((SLresult)0x00000005)
#define SL_RESULT_IO_ERROR          ((SLresult)0x00000006)
#define SL_RESULT_BUFFER_INSUFFICIENT ((SLresult)0x00000007)
#define SL_RESULT_CONTENT_CORRUPTED ((SLresult)0x00000008)
#define SL_RESULT_CONTENT_UNSUPPORTED ((SLresult)0x00000009)
#define SL_RESULT_CONTENT_NOT_FOUND ((SLresult)0x0000000A)
#define SL_RESULT_PERMISSION_DENIED ((SLresult)0x0000000B)
#define SL_RESULT_FEATURE_UNSUPPORTED ((SLresult)0x0000000C)
#define SL_RESULT_INTERNAL_ERROR    ((SLresult)0x0000000D)
#define SL_RESULT_UNKNOWN_ERROR     ((SLresult)0x0000000E)
#define SL_RESULT_OPERATION_ABORTED ((SLresult)0x0000000F)
#define SL_RESULT_CONTROL_LOST      ((SLresult)0x00000010)

#define SL_BOOLEAN_FALSE ((SLboolean)0x00)
#define SL_BOOLEAN_TRUE  ((SLboolean)0x01)

/* Object interface IDs */
typedef struct SLInterfaceID_ { uint32_t a,b,c,d; } *SLInterfaceID;

extern SLInterfaceID SL_IID_ENGINE;
extern SLInterfaceID SL_IID_PLAY;
extern SLInterfaceID SL_IID_VOLUME;
extern SLInterfaceID SL_IID_BUFFERQUEUE;
extern SLInterfaceID SL_IID_OUTPUTMIX;
extern SLInterfaceID SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
extern SLInterfaceID SL_IID_ANDROIDCONFIGURATION;

/* Play states */
#define SL_PLAYSTATE_STOPPED ((SLuint32)0x00000001)
#define SL_PLAYSTATE_PAUSED  ((SLuint32)0x00000002)
#define SL_PLAYSTATE_PLAYING ((SLuint32)0x00000003)

/* Data locator PCM */
#define SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE ((SLuint32)0x800007BD)
#define SL_DATALOCATOR_OUTPUTMIX               ((SLuint32)0x00000004)
#define SL_DATAFORMAT_PCM                      ((SLuint32)0x00000001)

#define SL_PCMSAMPLEFORMAT_FIXED_8  ((SLuint16)0x0008)
#define SL_PCMSAMPLEFORMAT_FIXED_16 ((SLuint16)0x0010)

#define SL_SPEAKER_FRONT_LEFT   ((SLuint32)0x00000001)
#define SL_SPEAKER_FRONT_RIGHT  ((SLuint32)0x00000002)
#define SL_SPEAKER_FRONT_CENTER ((SLuint32)0x00000004)

typedef struct SLDataLocator_AndroidSimpleBufferQueue {
    SLuint32 locatorType;
    SLuint32 numBuffers;
} SLDataLocator_AndroidSimpleBufferQueue;

typedef struct SLDataLocator_OutputMix {
    SLuint32   locatorType;
    SLObjectItf outputMix;
} SLDataLocator_OutputMix;

typedef struct SLDataFormat_PCM {
    SLuint32 formatType;
    SLuint32 numChannels;
    SLuint32 samplesPerSec;
    SLuint32 bitsPerSample;
    SLuint32 containerSize;
    SLuint32 channelMask;
    SLuint32 endianness;
} SLDataFormat_PCM;

typedef struct SLDataSource {
    void* pLocator;
    void* pFormat;
} SLDataSource;

typedef struct SLDataSink {
    void* pLocator;
    void* pFormat;
} SLDataSink;

/* Interface structures */
struct SLObjectItf_;
struct SLEngineItf_;
struct SLPlayItf_;
struct SLAndroidSimpleBufferQueueItf_;
struct SLVolumeItf_;
struct SLOutputMixItf_;

typedef const struct SLObjectItf_* const* SLObjectItf_t;
typedef const struct SLEngineItf_* const* SLEngineItf;
typedef const struct SLPlayItf_* const* SLPlayItf;
typedef const struct SLAndroidSimpleBufferQueueItf_* const* SLAndroidSimpleBufferQueueItf;
typedef const struct SLVolumeItf_* const* SLVolumeItf;
typedef const struct SLOutputMixItf_* const* SLOutputMixItf;

typedef void (*slAndroidSimpleBufferQueueCallback)(SLAndroidSimpleBufferQueueItf caller, void* pContext);

struct SLAndroidSimpleBufferQueueItf_ {
    SLresult (*Enqueue)(SLAndroidSimpleBufferQueueItf self, const void* pBuffer, SLuint32 size);
    SLresult (*Clear)(SLAndroidSimpleBufferQueueItf self);
    SLresult (*GetState)(SLAndroidSimpleBufferQueueItf self, void* pState);
    SLresult (*RegisterCallback)(SLAndroidSimpleBufferQueueItf self,
                                  slAndroidSimpleBufferQueueCallback callback, void* pContext);
};

struct SLPlayItf_ {
    SLresult (*SetPlayState)(SLPlayItf self, SLuint32 state);
    SLresult (*GetPlayState)(SLPlayItf self, SLuint32* state);
    SLresult (*GetDuration)(SLPlayItf self, SLmillisecond* msec);
    SLresult (*GetPosition)(SLPlayItf self, SLmillisecond* msec);
    SLresult (*RegisterCallback)(SLPlayItf self, void* callback, void* pContext);
    SLresult (*SetCallbackEventsMask)(SLPlayItf self, SLuint32 eventFlags);
    SLresult (*SetMarkerPosition)(SLPlayItf self, SLmillisecond mSec);
    SLresult (*ClearMarkerPosition)(SLPlayItf self);
    SLresult (*GetMarkerPosition)(SLPlayItf self, SLmillisecond* mSec);
    SLresult (*SetPositionUpdatePeriod)(SLPlayItf self, SLmillisecond mSec);
    SLresult (*GetPositionUpdatePeriod)(SLPlayItf self, SLmillisecond* mSec);
};

struct SLVolumeItf_ {
    SLresult (*SetVolumeLevel)(SLVolumeItf self, SLmillibel level);
    SLresult (*GetVolumeLevel)(SLVolumeItf self, SLmillibel* level);
    SLresult (*GetMaxVolumeLevel)(SLVolumeItf self, SLmillibel* maxLevel);
    SLresult (*SetMute)(SLVolumeItf self, SLboolean mute);
    SLresult (*GetMute)(SLVolumeItf self, SLboolean* mute);
    SLresult (*EnableStereoPosition)(SLVolumeItf self, SLboolean enable);
    SLresult (*IsEnabledStereoPosition)(SLVolumeItf self, SLboolean* pEnable);
    SLresult (*SetStereoPosition)(SLVolumeItf self, SLint16 stereoPosition);
    SLresult (*GetStereoPosition)(SLVolumeItf self, SLint16* pStereoPosition);
};

struct SLObjectItf_ {
    SLresult (*Realize)(SLObjectItf_t self, SLboolean async);
    SLresult (*Resume)(SLObjectItf_t self, SLboolean async);
    SLresult (*GetState)(SLObjectItf_t self, SLuint32* pState);
    SLresult (*GetInterface)(SLObjectItf_t self, SLInterfaceID iid, void* pInterface);
    SLresult (*RegisterCallback)(SLObjectItf_t self, void* callback, void* pContext);
    void     (*AbortAsyncOperation)(SLObjectItf_t self);
    void     (*Destroy)(SLObjectItf_t self);
    SLresult (*SetPriority)(SLObjectItf_t self, SLint32 priority, SLboolean preemptable);
    SLresult (*GetPriority)(SLObjectItf_t self, SLint32* pPriority, SLboolean* pPreemptable);
    SLresult (*SetLossOfControlInterfaces)(SLObjectItf_t self, SLint16 numInterfaces,
                                           SLInterfaceID* pInterfaceIDs, SLboolean enabled);
};

struct SLEngineItf_ {
    SLresult (*CreateAudioPlayer)(SLEngineItf self, SLObjectItf_t* pPlayer,
                                   SLDataSource* pAudioSrc, SLDataSink* pAudioSnk,
                                   SLuint32 numInterfaces, SLInterfaceID* pInterfaceIds,
                                   SLboolean* pInterfaceRequired);
    SLresult (*CreateAudioRecorder)(SLEngineItf self, SLObjectItf_t* pRecorder,
                                     SLDataSource* pAudioSrc, SLDataSink* pAudioSnk,
                                     SLuint32 numInterfaces, SLInterfaceID* pInterfaceIds,
                                     SLboolean* pInterfaceRequired);
    SLresult (*CreateOutputMix)(SLEngineItf self, SLObjectItf_t* pMix,
                                 SLuint32 numInterfaces, SLInterfaceID* pInterfaceIds,
                                 SLboolean* pInterfaceRequired);
    SLresult (*CreateMetadataExtractor)(SLEngineItf self, SLObjectItf_t* pMetadataExtractor,
                                        SLDataSource* pDataSource, SLuint32 numInterfaces,
                                        SLInterfaceID* pInterfaceIds, SLboolean* pInterfaceRequired);
    SLresult (*CreateExtensionObject)(SLEngineItf self, SLObjectItf_t* pObject,
                                       void* pParameters, SLuint32 objectID, SLuint32 numInterfaces,
                                       SLInterfaceID* pInterfaceIds, SLboolean* pInterfaceRequired);
    SLresult (*QueryNumSupportedInterfaces)(SLEngineItf self, SLuint32 objectID,
                                            SLuint32* pNumSupportedInterfaces);
    SLresult (*QuerySupportedInterfaces)(SLEngineItf self, SLuint32 objectID,
                                          SLuint32 index, SLInterfaceID* pInterfaceId);
    SLresult (*QueryNumSupportedExtensions)(SLEngineItf self, SLuint32* pNumExtensions);
    SLresult (*QuerySupportedExtension)(SLEngineItf self, SLuint32 index,
                                         SLchar* pExtensionName, SLint16* pNameLength);
    SLresult (*IsExtensionSupported)(SLEngineItf self, const SLchar* pExtensionName,
                                      SLboolean* pSupported);
};

typedef char SLchar;

SLresult slCreateEngine(SLObjectItf_t* pEngine, SLuint32 numOptions, void* pEngineOptions,
                        SLuint32 numInterfaces, SLInterfaceID* pInterfaceIds,
                        SLboolean* pInterfaceRequired);

#ifdef __cplusplus
}
#endif
