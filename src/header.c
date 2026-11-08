#include <stdio.h>
#include "amgf.h"

/*
 * Wyjscie (big-endian, offsety od poczatku pliku):
 *
 *  0x00  4  magic          "AMGF"
 *  0x04  2  ver_major      1
 *  0x06  2  ver_minor      0
 *  0x08  4  flags          bit0: 0=ARGB 1=RGBA; bit1: jest tlo
 *  0x0C  4  bg_argb
 *  0x10 12  sec_colors     {count, offset, size}
 *  0x1C 12  sec_seams      {count, offset, size}
 *  0x28 12  sec_tables     {count, offset, size}
 *  0x34 12  sec_shapes     {count, offset, size}
 *  0x40 12  sec_frames     {count, offset, size}
 *  0x4C 12  sec_trees      {count, offset, size}
 *  0x58  4  total_size
 *
 * Payload w kolejnosci: cl | szwy | tab | shape | fr | tree
 * Sekcja pusta: count=0, offset=0, size=0
 */

static int be_u16(FILE *f, uint16_t v)
{
    uint8_t b[2] = { (uint8_t)(v >> 8), (uint8_t)v };
    return fwrite(b, 1, 2, f) == 2 ? 0 : -1;
}

static int be_u32(FILE *f, uint32_t v)
{
    uint8_t b[4] = { (uint8_t)(v >> 24), (uint8_t)(v >> 16),
                     (uint8_t)(v >> 8),  (uint8_t)v };
    return fwrite(b, 1, 4, f) == 4 ? 0 : -1;
}

static int write_sec_hdr(FILE *f, const amgf_section *s, uint32_t offset)
{
    uint32_t count = s->size ? s->count : 0;
    uint32_t off   = s->size ? offset  : 0;
    int e = 0;
    e |= be_u32(f, count);
    e |= be_u32(f, off);
    e |= be_u32(f, s->size);
    return e;
}

static int write_payload(FILE *f, const amgf_section *s)
{
    if (s->size == 0) return 0;
    if (!s->data)     return -1;
    return fwrite(s->data, 1, s->size, f) == s->size ? 0 : -1;
}

int amgf_create(const char *path,
                uint32_t flags, uint32_t bg_argb,
                const amgf_section *colors,
                const amgf_section *seams,
                const amgf_section *tables,
                const amgf_section *shapes,
                const amgf_section *frames,
                const amgf_section *trees)
{
    if (!path || !colors || !seams || !tables
        || !shapes || !frames || !trees)
        return -1;

    uint32_t off_colors = AMGF_HDR_SIZE;
    uint32_t off_seams  = off_colors + colors->size;
    uint32_t off_tables = off_seams  + seams->size;
    uint32_t off_shapes = off_tables + tables->size;
    uint32_t off_frames = off_shapes + shapes->size;
    uint32_t off_trees  = off_frames + frames->size;
    uint32_t total      = off_trees  + trees->size;

    FILE *f = fopen(path, "wb");
    if (!f) return -2;

    int e = 0;
    e |= fwrite(AMGF_MAGIC, 1, 4, f) == 4 ? 0 : -1;
    e |= be_u16(f, AMGF_VER_MAJOR);
    e |= be_u16(f, AMGF_VER_MINOR);
    e |= be_u32(f, flags);
    e |= be_u32(f, bg_argb);
    e |= write_sec_hdr(f, colors, off_colors);
    e |= write_sec_hdr(f, seams,  off_seams);
    e |= write_sec_hdr(f, tables, off_tables);
    e |= write_sec_hdr(f, shapes, off_shapes);
    e |= write_sec_hdr(f, frames, off_frames);
    e |= write_sec_hdr(f, trees,  off_trees);
    e |= be_u32(f, total);

    if (e) { fclose(f); return -3; }

    e |= write_payload(f, colors);
    e |= write_payload(f, seams);
    e |= write_payload(f, tables);
    e |= write_payload(f, shapes);
    e |= write_payload(f, frames);
    e |= write_payload(f, trees);

    fclose(f);
    return e ? -4 : 0;
}
