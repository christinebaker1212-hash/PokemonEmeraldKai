// Moemon Kai: prints each card of the pack with what keeps it from being playable ("" when it is), for
// tools/mtg/scope.py (docs/MTG_SCOPE.md).
#include <stdio.h>
#include <stdlib.h>
#include "mtg_internal.h"

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "graphics/mtg/forge.pak", "rb");
    long n;
    void *p;
    int i;
    if (!f)
        return 1;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    p = malloc((size_t)n);
    if (fread(p, 1, (size_t)n, f) != (size_t)n || !Mtg_DbLoad(p, (uint32_t)n))
        return 1;
    fclose(f);
    free(p);
    for (i = 0; i < Mtg_DbCount(); i++) {
        const MtgCardDef *c = Mtg_Card(i);
        printf("%s\t%s\n", c->name, c->supported ? "" : c->unsupported);
    }
    return 0;
}
