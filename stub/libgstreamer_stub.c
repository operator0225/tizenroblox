/*
 * GStreamer stub/shim for Tizen TV
 *
 * Sober uses GStreamer for Roblox video playback (cutscenes, in-game video).
 * This shim:
 *   1. Tries to load the system GStreamer 1.x at runtime
 *   2. Falls back to no-op stubs if not found (video won't play but game runs)
 *
 * Covers: libgstreamer-1.0.so.0, libgstapp-1.0.so.0, libgstvideo-1.0.so.0
 * All three are provided in a single stub since sober links each separately.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

typedef void GstElement;
typedef void GstPipeline;
typedef void GstBus;
typedef void GstPad;
typedef void GstCaps;
typedef void GstStructure;
typedef void GstSample;
typedef void GstBuffer;
typedef void GstBufferList;
typedef void GstClock;
typedef void GstVideoFrame;
typedef void GstMiniObject;
typedef uint32_t GstState;
typedef uint32_t GstStateChangeReturn;
typedef uint32_t GstFlowReturn;
typedef uint32_t GstPadLinkReturn;
typedef uint64_t GstClockTime;
typedef uint64_t GstSeekFlags;
typedef uint64_t GstSeekType;
typedef int gboolean;
typedef void (*GstAppSinkCallbacks_t)(GstElement*, void*);
typedef void GstAppSinkCallbacksStruct;

static void *gst_core = NULL;
static void *gst_app  = NULL;
static void *gst_vid  = NULL;

/* Core */
static void     (*r_gst_init)(int*, char***) = NULL;
static gboolean (*r_gst_is_initialized)(void) = NULL;
static GstElement* (*r_gst_pipeline_new)(const char*) = NULL;
static GstElement* (*r_gst_element_factory_make)(const char*, const char*) = NULL;
static void*    (*r_gst_element_factory_find)(const char*) = NULL;
static gboolean (*r_gst_bin_add)(GstElement*, GstElement*) = NULL;
static void     (*r_gst_bin_add_many)(GstElement*, ...) = NULL;
static gboolean (*r_gst_element_link)(GstElement*, GstElement*) = NULL;
static gboolean (*r_gst_element_link_many)(GstElement*, ...) = NULL;
static GstStateChangeReturn (*r_gst_element_set_state)(GstElement*, GstState) = NULL;
static GstPad*  (*r_gst_element_get_static_pad)(GstElement*, const char*) = NULL;
static GstPad*  (*r_gst_pad_get_current_caps)(GstPad*) = NULL;
static GstPad*  (*r_gst_pad_query_caps)(GstPad*, GstCaps*) = NULL;
static gboolean (*r_gst_pad_is_linked)(GstPad*) = NULL;
static GstPadLinkReturn (*r_gst_pad_link)(GstPad*, GstPad*) = NULL;
static GstCaps* (*r_gst_caps_from_string)(const char*) = NULL;
static GstStructure* (*r_gst_caps_get_structure)(const GstCaps*, unsigned int) = NULL;
static gboolean (*r_gst_caps_is_any)(const GstCaps*) = NULL;
static gboolean (*r_gst_caps_is_empty)(const GstCaps*) = NULL;
static const char* (*r_gst_structure_get_name)(const GstStructure*) = NULL;
static gboolean (*r_gst_structure_get_int)(const GstStructure*, const char*, int*) = NULL;
static const char* (*r_gst_structure_get_string)(const GstStructure*, const char*) = NULL;
static GstMiniObject* (*r_gst_mini_object_ref)(GstMiniObject*) = NULL;
static void     (*r_gst_mini_object_unref)(GstMiniObject*) = NULL;
static void*    (*r_gst_object_ref)(void*) = NULL;
static void     (*r_gst_object_unref)(void*) = NULL;
static gboolean (*r_gst_element_seek_simple)(GstElement*, int, GstSeekFlags, GstClockTime) = NULL;

/* App plugin */
static void     (*r_gst_app_sink_set_callbacks)(GstElement*, GstAppSinkCallbacksStruct*, void*, void*) = NULL;
static GstSample* (*r_gst_app_sink_pull_sample)(GstElement*) = NULL;
static GstCaps* (*r_gst_app_src_get_caps)(GstElement*) = NULL;
static GstFlowReturn (*r_gst_app_src_push_buffer)(GstElement*, GstBuffer*) = NULL;
static GstFlowReturn (*r_gst_app_src_end_of_stream)(GstElement*) = NULL;

/* Sample / Buffer */
static GstBuffer* (*r_gst_sample_get_buffer)(GstSample*) = NULL;
static GstCaps*   (*r_gst_sample_get_caps)(GstSample*) = NULL;
static gboolean   (*r_gst_buffer_map)(GstBuffer*, void*, int) = NULL;
static void       (*r_gst_buffer_unmap)(GstBuffer*, void*) = NULL;
static GstBuffer* (*r_gst_buffer_new_allocate)(void*, size_t, void*) = NULL;

/* Video */
static void (*r_gst_video_frame_unmap)(GstVideoFrame*) = NULL;

__attribute__((constructor))
static void gstreamer_stub_init(void) {
    /* Try system GStreamer */
    gst_core = dlopen("/usr/lib/aarch64-linux-gnu/libgstreamer-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_core) gst_core = dlopen("/usr/lib64/libgstreamer-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_core) gst_core = dlopen("/usr/lib/libgstreamer-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    gst_app  = dlopen("/usr/lib/aarch64-linux-gnu/libgstapp-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_app) gst_app = dlopen("/usr/lib64/libgstapp-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_app) gst_app = dlopen("/usr/lib/libgstapp-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    gst_vid  = dlopen("/usr/lib/aarch64-linux-gnu/libgstvideo-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_vid) gst_vid = dlopen("/usr/lib64/libgstvideo-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (!gst_vid) gst_vid = dlopen("/usr/lib/libgstvideo-1.0.so.0", RTLD_NOW | RTLD_GLOBAL);

    if (gst_core) {
        fprintf(stderr, "[gst-stub] System GStreamer loaded\n");
#define LOAD_CORE(sym) r_##sym = dlsym(gst_core, #sym)
        LOAD_CORE(gst_init); LOAD_CORE(gst_is_initialized);
        LOAD_CORE(gst_pipeline_new); LOAD_CORE(gst_element_factory_make);
        LOAD_CORE(gst_element_factory_find);
        LOAD_CORE(gst_bin_add); LOAD_CORE(gst_bin_add_many);
        LOAD_CORE(gst_element_link); LOAD_CORE(gst_element_link_many);
        LOAD_CORE(gst_element_set_state); LOAD_CORE(gst_element_get_static_pad);
        LOAD_CORE(gst_pad_get_current_caps); LOAD_CORE(gst_pad_query_caps);
        LOAD_CORE(gst_pad_is_linked); LOAD_CORE(gst_pad_link);
        LOAD_CORE(gst_caps_from_string); LOAD_CORE(gst_caps_get_structure);
        LOAD_CORE(gst_caps_is_any); LOAD_CORE(gst_caps_is_empty);
        LOAD_CORE(gst_structure_get_name); LOAD_CORE(gst_structure_get_int);
        LOAD_CORE(gst_structure_get_string);
        LOAD_CORE(gst_mini_object_ref); LOAD_CORE(gst_mini_object_unref);
        LOAD_CORE(gst_object_ref); LOAD_CORE(gst_object_unref);
        LOAD_CORE(gst_element_seek_simple);
        LOAD_CORE(gst_sample_get_buffer); LOAD_CORE(gst_sample_get_caps);
        LOAD_CORE(gst_buffer_map); LOAD_CORE(gst_buffer_unmap);
        LOAD_CORE(gst_buffer_new_allocate);
#undef LOAD_CORE
    } else {
        fprintf(stderr, "[gst-stub] System GStreamer NOT found — video playback disabled\n");
    }

    if (gst_app) {
#define LOAD_APP(sym) r_##sym = dlsym(gst_app, #sym)
        LOAD_APP(gst_app_sink_set_callbacks); LOAD_APP(gst_app_sink_pull_sample);
        LOAD_APP(gst_app_src_get_caps); LOAD_APP(gst_app_src_push_buffer);
        LOAD_APP(gst_app_src_end_of_stream);
#undef LOAD_APP
    }

    if (gst_vid) {
#define LOAD_VID(sym) r_##sym = dlsym(gst_vid, #sym)
        LOAD_VID(gst_video_frame_unmap);
#undef LOAD_VID
    }
}

/* ── Core exports ─────────────────────────────────────────────────────────── */
void gst_init(int *argc, char ***argv)
    { if (r_gst_init) r_gst_init(argc, argv); }
gboolean gst_is_initialized(void)
    { return r_gst_is_initialized ? r_gst_is_initialized() : 0; }
GstElement* gst_pipeline_new(const char *name)
    { return r_gst_pipeline_new ? r_gst_pipeline_new(name) : NULL; }
GstElement* gst_element_factory_make(const char *factoryname, const char *name)
    { return r_gst_element_factory_make ? r_gst_element_factory_make(factoryname, name) : NULL; }
void* gst_element_factory_find(const char *name)
    { return r_gst_element_factory_find ? r_gst_element_factory_find(name) : NULL; }
gboolean gst_bin_add(GstElement *bin, GstElement *element)
    { return r_gst_bin_add ? r_gst_bin_add(bin, element) : 0; }
void gst_bin_add_many(GstElement *bin, ...)
    { /* variadic forwarding not practical without system lib */ (void)bin; }
gboolean gst_element_link(GstElement *src, GstElement *dest)
    { return r_gst_element_link ? r_gst_element_link(src, dest) : 0; }
void gst_element_link_many(GstElement *element_1, ...)
    { (void)element_1; }
GstStateChangeReturn gst_element_set_state(GstElement *element, GstState state)
    { return r_gst_element_set_state ? r_gst_element_set_state(element, state) : 1; }
GstPad* gst_element_get_static_pad(GstElement *element, const char *name)
    { return r_gst_element_get_static_pad ? r_gst_element_get_static_pad(element, name) : NULL; }
GstPad* gst_pad_get_current_caps(GstPad *pad)
    { return r_gst_pad_get_current_caps ? r_gst_pad_get_current_caps(pad) : NULL; }
GstPad* gst_pad_query_caps(GstPad *pad, GstCaps *filter)
    { return r_gst_pad_query_caps ? r_gst_pad_query_caps(pad, filter) : NULL; }
gboolean gst_pad_is_linked(GstPad *pad)
    { return r_gst_pad_is_linked ? r_gst_pad_is_linked(pad) : 0; }
GstPadLinkReturn gst_pad_link(GstPad *srcpad, GstPad *sinkpad)
    { return r_gst_pad_link ? r_gst_pad_link(srcpad, sinkpad) : 0; }
GstCaps* gst_caps_from_string(const char *string)
    { return r_gst_caps_from_string ? r_gst_caps_from_string(string) : NULL; }
GstStructure* gst_caps_get_structure(const GstCaps *caps, unsigned int index)
    { return r_gst_caps_get_structure ? r_gst_caps_get_structure(caps, index) : NULL; }
gboolean gst_caps_is_any(const GstCaps *caps)
    { return r_gst_caps_is_any ? r_gst_caps_is_any(caps) : 1; }
gboolean gst_caps_is_empty(const GstCaps *caps)
    { return r_gst_caps_is_empty ? r_gst_caps_is_empty(caps) : 1; }
const char* gst_structure_get_name(const GstStructure *structure)
    { return r_gst_structure_get_name ? r_gst_structure_get_name(structure) : ""; }
gboolean gst_structure_get_int(const GstStructure *structure, const char *fieldname, int *value)
    { return r_gst_structure_get_int ? r_gst_structure_get_int(structure, fieldname, value) : 0; }
const char* gst_structure_get_string(const GstStructure *structure, const char *fieldname)
    { return r_gst_structure_get_string ? r_gst_structure_get_string(structure, fieldname) : NULL; }
GstMiniObject* gst_mini_object_ref(GstMiniObject *mini_object)
    { return r_gst_mini_object_ref ? r_gst_mini_object_ref(mini_object) : mini_object; }
void gst_mini_object_unref(GstMiniObject *mini_object)
    { if (r_gst_mini_object_unref) r_gst_mini_object_unref(mini_object); }
void* gst_object_ref(void *object)
    { return r_gst_object_ref ? r_gst_object_ref(object) : object; }
void gst_object_unref(void *object)
    { if (r_gst_object_unref) r_gst_object_unref(object); }
gboolean gst_element_seek_simple(GstElement *element, int format,
    GstSeekFlags seek_flags, GstClockTime seek_pos) {
    return r_gst_element_seek_simple ?
        r_gst_element_seek_simple(element, format, seek_flags, seek_pos) : 0;
}

/* ── App plugin exports ───────────────────────────────────────────────────── */
void gst_app_sink_set_callbacks(GstElement *appsink, GstAppSinkCallbacksStruct *callbacks,
    void *user_data, void *notify) {
    if (r_gst_app_sink_set_callbacks) r_gst_app_sink_set_callbacks(appsink, callbacks, user_data, notify);
}
GstSample* gst_app_sink_pull_sample(GstElement *appsink)
    { return r_gst_app_sink_pull_sample ? r_gst_app_sink_pull_sample(appsink) : NULL; }
GstCaps* gst_app_src_get_caps(GstElement *appsrc)
    { return r_gst_app_src_get_caps ? r_gst_app_src_get_caps(appsrc) : NULL; }
GstFlowReturn gst_app_src_push_buffer(GstElement *appsrc, GstBuffer *buffer)
    { return r_gst_app_src_push_buffer ? r_gst_app_src_push_buffer(appsrc, buffer) : 0; }
GstFlowReturn gst_app_src_end_of_stream(GstElement *appsrc)
    { return r_gst_app_src_end_of_stream ? r_gst_app_src_end_of_stream(appsrc) : 0; }

/* ── Sample / Buffer exports ──────────────────────────────────────────────── */
GstBuffer* gst_sample_get_buffer(GstSample *sample)
    { return r_gst_sample_get_buffer ? r_gst_sample_get_buffer(sample) : NULL; }
GstCaps* gst_sample_get_caps(GstSample *sample)
    { return r_gst_sample_get_caps ? r_gst_sample_get_caps(sample) : NULL; }
gboolean gst_buffer_map(GstBuffer *buffer, void *info, int flags)
    { return r_gst_buffer_map ? r_gst_buffer_map(buffer, info, flags) : 0; }
void gst_buffer_unmap(GstBuffer *buffer, void *info)
    { if (r_gst_buffer_unmap) r_gst_buffer_unmap(buffer, info); }
GstBuffer* gst_buffer_new_allocate(void *allocator, size_t size, void *params)
    { return r_gst_buffer_new_allocate ? r_gst_buffer_new_allocate(allocator, size, params) : NULL; }

/* ── Video exports ────────────────────────────────────────────────────────── */
void gst_video_frame_unmap(GstVideoFrame *frame)
    { if (r_gst_video_frame_unmap) r_gst_video_frame_unmap(frame); }
