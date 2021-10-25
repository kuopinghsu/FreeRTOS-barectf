#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>

#include "barectf.h"
#include "barectf-port.h"

/* Platform context */
struct platform_ctx {
    struct barectf_freertos_ctx ctx;
#ifndef HAVE_SYS_DUMP
    FILE *fh;
#endif
};

struct barectf_freertos_ctx *trace_freertos_ctx;

/* Clock source platform function */
static uint64_t clock_get_value(void * const data)
{
    return xGetTime();
}

static void write_packet(const struct platform_ctx * const platform_ctx)
{
#ifdef HAVE_SYS_DUMP
    sys_dump((int)barectf_packet_buf_addr(&platform_ctx->ctx),
             (int)barectf_packet_buf_size(&platform_ctx->ctx));
#else
    /* Append current packet to data stream file */
    const size_t nmemb = fwrite(barectf_packet_buf_addr(&platform_ctx->ctx),
                                barectf_packet_buf_size(&platform_ctx->ctx),
                                1, platform_ctx->fh);

    assert(nmemb == 1);
#endif
}

/* Full back end check platform function */
static int is_backend_full(void * const data)
{
    return 0;
}

/* Packet opening platform function */
static void open_packet(void * const data)
{
    struct platform_ctx * const platform_ctx = data;

    barectf_freertos_open_packet(&platform_ctx->ctx);
}

/* Packet closing platform function */
static void close_packet(void * const data)
{
    struct platform_ctx * const platform_ctx = data;

    /* Close packet now */
    barectf_freertos_close_packet(&platform_ctx->ctx);

    /* Write packet to file */
    write_packet(platform_ctx);
}

/* Platform initialization function */
struct platform_ctx *trace_freertos_init(const unsigned int buf_size,
    const char * const data_stream_file_path)
{
    uint8_t *buf = NULL;
    struct platform_ctx *platform_ctx;
    struct barectf_platform_callbacks cbs;

    /* Set platform callback functions */
    cbs.hrclock_clock_get_value = clock_get_value;
    cbs.is_backend_full = is_backend_full;
    cbs.open_packet = open_packet;
    cbs.close_packet = close_packet;

    /* Allocate platform context (which contains a barectf context) */
    platform_ctx = malloc(sizeof(*platform_ctx));

    if (!platform_ctx) {
        goto error;
    }

    /* Allocate packet buffer */
    buf = malloc(buf_size);

    if (!buf) {
        goto error;
    }

#ifndef HAVE_SYS_DUMP
    /* Open data stream file */
    platform_ctx->fh = fopen(data_stream_file_path, "wb");

    if (!platform_ctx->fh) {
        goto error;
    }
#endif

    /* Initialize barectf context */
    barectf_init(&platform_ctx->ctx, buf, buf_size, cbs, platform_ctx);

    /* Open the first packet */
    open_packet(platform_ctx);

    goto end;

error:
    if (platform_ctx)
        free(platform_ctx);

    if (buf)
        free(buf);

    platform_ctx = NULL;

end:
    /* Return platform context to user */
    return platform_ctx;
}

/* Platform finalization function */
void trace_freertos_fini(struct platform_ctx * const platform_ctx)
{
    /* Close current packet if needed */
    if (barectf_packet_is_open(&platform_ctx->ctx) &&
            !barectf_packet_is_empty(&platform_ctx->ctx)) {
        close_packet(platform_ctx);
    }

#ifndef HAVE_SYS_DUMP
    /* Close data stream file */
    fclose(platform_ctx->fh);
#endif

    /* Deallocate packet buffer */
    free(barectf_packet_buf(&platform_ctx->ctx));

    /* Deallocate platform context */
    free(platform_ctx);
}

/* barectf context pointer access function */
struct barectf_freertos_ctx *trace_freertos_get_barectf_ctx(
    struct platform_ctx * const platform_ctx)
{
    return &platform_ctx->ctx;
}

