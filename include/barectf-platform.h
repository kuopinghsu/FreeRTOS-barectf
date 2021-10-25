#ifndef _MY_PLATFORM_H
#define _MY_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Platform context */
struct platform_ctx;

/* barectf context */
struct barectf_freertos_ctx;

/* Platform initialization function */
struct platform_ctx *trace_freertos_init(unsigned int buf_size,
                                         const char *data_stream_file_path);

/* Platform finalization function */
void trace_freertos_fini(struct platform_ctx *platform_ctx);

/* barectf context pointer access function */
struct barectf_freertos_ctx *trace_freertos_get_barectf_ctx(
    struct platform_ctx *platform_ctx);

#ifdef __cplusplus
}
#endif

#endif /* _MY_PLATFORM_H */
