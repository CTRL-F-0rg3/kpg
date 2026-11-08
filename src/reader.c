#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "amgf.h"

/* --- Czytanie big-endian --- */
static uint16_t read_u16(const uint8_t **p) {
    uint16_t v = ((uint16_t)(*p)[0] << 8) | (*p)[1];
    *p += 2;
    return v;
}

static uint32_t read_u32(const uint8_t **p) {
    uint32_t v = ((uint32_t)(*p)[0] << 24) | ((uint32_t)(*p)[1] << 16) |
                 ((uint32_t)(*p)[2] << 8)  | (*p)[3];
    *p += 4;
    return v;
}

int amgf_load(const char *path, amgf_file *out)
{
    if (!path || !out) return -1;
    memset(out, 0, sizeof(amgf_file));

    FILE *f = fopen(path, "rb");
    if (!f) return -2;

    /* Sprawdzamy rozmiar pliku, zeby od razu wiedziec czy jest uciety */
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < AMGF_HDR_SIZE) {
        fclose(f);
        return -3; /* Za maly na naglowek */
    }

    /* Wczytujemy caly naglowek na raz do bufora */
    uint8_t hdr[AMGF_HDR_SIZE];
    if (fread(hdr, 1, AMGF_HDR_SIZE, f) != AMGF_HDR_SIZE) {
        fclose(f);
        return -4;
    }

    const uint8_t *p = hdr;

    /* 1. Tozsamosc */
    if (memcmp(p, AMGF_MAGIC, 4) != 0) { fclose(f); return -5; }
    p += 4;
    uint16_t major = read_u16(&p);
    /* uint16_t minor = read_u16(&p); */ /* pomijamy, na razie wersja 1.0 */
    (void)major;
    p += 2;

    /* 2. Wlasciwosci globalne */
    out->flags = read_u32(&p);
    out->bg_argb = read_u32(&p);

    /* 3. Spis sekcji (zapisujemy offsety i rozmiary) */
    uint32_t off[6], sz[6], cnt[6];
    for (int i = 0; i < 6; i++) {
        cnt[i] = read_u32(&p);
        off[i] = read_u32(&p);
        sz[i]  = read_u32(&p);
    }

    uint32_t total = read_u32(&p);
    if ((long)total != file_size) {
        fclose(f);
        return -6; /* Plik uciety lub rozjechany */
    }

    /* 4. Wczytywanie sekcji (na razie tylko cl i szwy) */
    
    /* --- COLORS (cl) --- */
    if (cnt[0] > 0 && sz[0] > 0) {
        out->colors = malloc(cnt[0] * sizeof(amgf_color));
        if (!out->colors) { fclose(f); return -7; }
        
        fseek(f, off[0], SEEK_SET);
        uint8_t *buf = malloc(sz[0]);
        fread(buf, 1, sz[0], f);
        
        const uint8_t *pb = buf;
        for (uint32_t i = 0; i < cnt[0]; i++) {
            out->colors[i].argb = read_u32(&pb);
            memcpy(out->colors[i].name, pb, 4);
            pb += 4;
        }
        free(buf);
        out->color_count = cnt[0];
    }

    /* --- SEAMS (szwy) --- */
    if (cnt[1] > 0 && sz[1] > 0) {
        out->seams = malloc(cnt[1] * sizeof(amgf_seam));
        if (!out->seams) { fclose(f); return -7; }
        
        fseek(f, off[1], SEEK_SET);
        uint8_t *buf = malloc(sz[1]);
        fread(buf, 1, sz[1], f);
        
        const uint8_t *pb = buf;
        for (uint32_t i = 0; i < cnt[1]; i++) {
            out->seams[i].x1 = (int32_t)read_u32(&pb);
            out->seams[i].y1 = (int32_t)read_u32(&pb);
            out->seams[i].x2 = (int32_t)read_u32(&pb);
            out->seams[i].y2 = (int32_t)read_u32(&pb);
        }
        free(buf);
        out->seam_count = cnt[1];
    }

    fclose(f);
    return 0;
}

void amgf_free(amgf_file *f)
{
    if (!f) return;
    free(f->colors);
    free(f->seams);
    /* free(f->tables); ... na przyszlosc */
    memset(f, 0, sizeof(amgf_file));
}

void amgf_print_info(const amgf_file *f)
{
    if (!f) return;
    printf("=== Zawartosc pliku AMGF ===\n");
    printf("Flagi: 0x%08X, Tlo: 0x%08X\n", f->flags, f->bg_argb);
    
    printf("Kolory (%u):\n", f->color_count);
    for (uint32_t i = 0; i < f->color_count; i++) {
        printf("  [%u] 0x%08X (%.4s)\n", i, f->colors[i].argb, f->colors[i].name);
    }

    printf("Szwy (%u):\n", f->seam_count);
    for (uint32_t i = 0; i < f->seam_count; i++) {
        printf("  [%u] Od (%d, %d) do (%d, %d)\n", i, 
            f->seams[i].x1, f->seams[i].y1, f->seams[i].x2, f->seams[i].y2);
    }
}
