#include <stdlib.h>
#include <string.h>
#include "amgf.h"

/*
 * cl:    8 B / rekord, big-endian
 *   [0] u32 argb (0xAARRGGBB)
 *   [4] char name[4]
 *
 * szwy: 16 B / rekord, big-endian, fixed 16.16
 *   [0] i32 x1
 *   [4] i32 y1
 *   [8] i32 x2
 *  [12] i32 y2
 */

static void put_u32(uint8_t **p, uint32_t v)
{
    (*p)[0] = (uint8_t)(v >> 24);
    (*p)[1] = (uint8_t)(v >> 16);
    (*p)[2] = (uint8_t)(v >> 8);
    (*p)[3] = (uint8_t)v;
    *p += 4;
}

amgf_section amgf_cl_build(const amgf_color *colors, uint32_t count)
{
    amgf_section s = { NULL, 0, 0 };
    if (count == 0) return s;

    uint32_t size = count * AMGF_CL_REC_SIZE;
    uint8_t *buf = malloc(size);
    if (!buf) return s;

    uint8_t *p = buf;
    for (uint32_t i = 0; i < count; i++) {
        put_u32(&p, colors[i].argb);
        memcpy(p, colors[i].name, 4);
        p += 4;
    }

    s.data  = buf;
    s.count = count;
    s.size  = size;
    return s;
}

amgf_section amgf_seams_build(const amgf_seam *seams, uint32_t count)
{
    amgf_section s = { NULL, 0, 0 };
    if (count == 0) return s;

    uint32_t size = count * AMGF_SEAM_REC_SIZE;
    uint8_t *buf = malloc(size);
    if (!buf) return s;

    uint8_t *p = buf;
    for (uint32_t i = 0; i < count; i++) {
        put_u32(&p, (uint32_t)seams[i].x1);
        put_u32(&p, (uint32_t)seams[i].y1);
        put_u32(&p, (uint32_t)seams[i].x2);
        put_u32(&p, (uint32_t)seams[i].y2);
    }

    s.data  = buf;
    s.count = count;
    s.size  = size;
    return s;
}

void amgf_section_free(amgf_section *s)
{
    if (!s) return;
    free((void *)s->data);
    s->data  = NULL;
    s->count = 0;
    s->size  = 0;
}
