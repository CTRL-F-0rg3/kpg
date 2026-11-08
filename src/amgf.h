#ifndef AMGF_H
#define AMGF_H

#include <stdint.h>

#define AMGF_MAGIC     "AMGF"
#define AMGF_VER_MAJOR 1u
#define AMGF_VER_MINOR 0u
#define AMGF_HDR_SIZE  92u

#define AMGF_FLAG_RGBA (1u << 0)
#define AMGF_FLAG_BG   (1u << 1)

#define AMGF_CL_REC_SIZE   8u
#define AMGF_SEAM_REC_SIZE 16u

typedef struct {
    const void *data;
    uint32_t    count;
    uint32_t    size;
} amgf_section;

typedef struct {
    uint32_t argb;
    char     name[4];
} amgf_color;

typedef struct {
    int32_t x1, y1;
    int32_t x2, y2;
} amgf_seam;

/* Struktura trzymajaca CALY wczytany plik w pamieci */
typedef struct {
    uint32_t flags;
    uint32_t bg_argb;
    
    amgf_color *colors;
    uint32_t    color_count;

    amgf_seam  *seams;
    uint32_t    seam_count;

    /* W przyszlosci: tab, shape, fr, tree */
} amgf_file;

/* Zapis */
int amgf_create(const char *path,
                uint32_t flags, uint32_t bg_argb,
                const amgf_section *colors,
                const amgf_section *seams,
                const amgf_section *tables,
                const amgf_section *shapes,
                const amgf_section *frames,
                const amgf_section *trees);

amgf_section amgf_cl_build   (const amgf_color *colors, uint32_t count);
amgf_section amgf_seams_build(const amgf_seam  *seams,  uint32_t count);
void amgf_section_free(amgf_section *s);

/* Odczyt i zarzadzanie */
int  amgf_load(const char *path, amgf_file *out);
void amgf_free(amgf_file *f);
void amgf_print_info(const amgf_file *f);

#endif
