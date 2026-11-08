#include <stdio.h>
#include "amgf.h"

int main(void)
{
    /* --- 1. PRZYGOTOWANIE DANYCH --- */
    amgf_color colors[] = {
        { 0xFFFF0000u, { 'f','1',0,0 } }, /* Czerwony */
        { 0xFF00FF00u, { 'f','2',0,0 } }, /* Zielony */
    };
    amgf_seam seams[] = {
        { 0, 0, 65536, 0 }, /* Linia pozioma (0,0) -> (1.0, 0) */
    };
    amgf_section empty = { NULL, 0, 0 };

    /* --- 2. ZAPIS DO PLIKU --- */
    amgf_section cl    = amgf_cl_build(colors, 2);
    amgf_section szwy  = amgf_seams_build(seams, 1);

    int rc = amgf_create("test.amgf",
                         AMGF_FLAG_BG, 0xFF000000u,
                         &cl, &szwy, &empty, &empty, &empty, &empty);
    
    amgf_section_free(&cl);
    amgf_section_free(&szwy);

    if (rc != 0) {
        printf("Blad zapisu: %d\n", rc);
        return 1;
    }
    printf("Zapisano test.amgf pomyslnie.\n");

    /* --- 3. ODCZYT Z PLIKU --- */
    amgf_file loaded;
    rc = amgf_load("test.amgf", &loaded);
    
    if (rc != 0) {
        printf("Blad odczytu: %d\n", rc);
        return 1;
    }
    
    /* --- 4. WERYFIKACJA I WYPISANIE --- */
    amgf_print_info(&loaded);
    
    /* Sprzatanie pamieci */
    amgf_free(&loaded);

    return 0;
}
