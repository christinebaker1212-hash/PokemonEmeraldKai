/*
 * Collision boxes (original module 0x4795C0-0x479C9F): the "Clsn1"/"Clsn2"
 * box lists of AIR elements and the overlap tests between two lists.
 *
 * In the overlap tests a "flipped" list or box is mirrored around x = 0
 * (the player faces left): x1..x2 becomes -x2..-x1.
 */
#include "mugen.h"

/* 0x004795C0: an empty list for count boxes (NULL if count <= 0). */
Clsn *Clsn_Create(int count)
{
    Clsn *clsn;

    if (count <= 0) {
        return NULL;
    }
    clsn = Mem_Calloc(1, sizeof(Clsn));
    if (clsn == NULL) {
        return NULL;
    }
    clsn->valid = 1;
    clsn->capacity = count;
    clsn->refs = 1;
    clsn->boxes = Mem_Calloc(count, sizeof(ClsnBox));
    if (clsn->boxes == NULL) {
        Clsn_Release(clsn);
        return NULL;
    }
    return clsn;
}

/* 0x00479640: drops a reference; frees the list with the last one. */
void Clsn_Release(Clsn *clsn)
{
    if (clsn == NULL) {
        return;
    }
    clsn->refs--;
    if (clsn->refs <= 0) {
        if (clsn->boxes != NULL) {
            free(clsn->boxes);
        }
        free(clsn);
    }
}

/* 0x00479670: 0 when the list is full. */
int Clsn_AddBox(Clsn *clsn, int x1, int y1, int x2, int y2)
{
    ClsnBox box;

    if (clsn->count >= clsn->capacity) {
        return 0;
    }
    box.hidden = 0;
    box.x1 = x1;
    box.y1 = y1;
    box.x2 = x2;
    box.y2 = y2;
    clsn->boxes[clsn->count] = box;
    clsn->count++;
    return 1;
}

static int Clsn_IsNumberStart(char c)
{
    return (c >= '0' && c <= '9') || c == '-';
}

/*
 * Moves *pos from the number just read to the start of the next one: to the
 * next ',' (or the end), then to the next digit or '-'. 0 if the line ends.
 */
static int Clsn_NextNumber(const char *s, int *pos)
{
    int i = *pos;

    while (s[i] != ',' && s[i] != '\0') {
        i++;
    }
    while (!Clsn_IsNumberStart(s[i])) {
        if (s[i] == '\0') {
            return 0;
        }
        i++;
    }
    *pos = i;
    return 1;
}

/*
 * 0x004796E0: parses a "Clsn1: n" / "Clsn2Default: n" line and the n
 * "Clsn1[i] = x1, y1, x2, y2" lines after it (read from ini, whose cursor is
 * on the line after the header). The index in brackets is ignored. Corners
 * are sorted, then scaled by g_clsnScaleX/Y (truncated) unless both are 1.
 *
 * Quirks kept from the original: the header is searched for ':' from its 6th
 * char on (it is assumed to start with "Clsn1"/"Clsn2"); a box line that does
 * not start with "clsn" makes it return NULL without freeing the list.
 */
Clsn *Clsn_Parse(const char *line, Ini *ini)
{
    Clsn *clsn;
    int i;
    int n;

    i = 5;
    while (line[i] != '\0' && line[i] != ':') {
        i++;
    }
    if (line[i] == '\0') {
        return NULL;
    }
    do {
        i++;
    } while (Char_IsSpace(line[i]));
    if (!Char_IsDigit(line[i])) {
        return NULL;
    }
    clsn = Clsn_Create(atoi(line + i));
    if (clsn == NULL) {
        return NULL;
    }
    for (n = 0; n < clsn->capacity; n++) {
        const char *s = Ini_NextLine(ini);
        int x1, y1, x2, y2, t;
        int j;

        if (s == NULL) {
            Clsn_Release(clsn);
            return NULL;
        }
        if (!Str_PrefixMatchI(s, "clsn", 4)) {
            return NULL;            /* leaks clsn */
        }
        j = 5;
        while (s[j] != '=') {
            if (s[j] == '\0') {
                goto fail;
            }
            j++;
        }
        do {
            j++;
            if (Clsn_IsNumberStart(s[j])) {
                break;
            }
            if (s[j] == '\0') {
                goto fail;
            }
        } while (1);
        x1 = atoi(s + j);
        if (!Clsn_NextNumber(s, &j)) {
            goto fail;
        }
        y1 = atoi(s + j);
        if (!Clsn_NextNumber(s, &j)) {
            goto fail;
        }
        x2 = atoi(s + j);
        if (!Clsn_NextNumber(s, &j)) {
            goto fail;
        }
        y2 = atoi(s + j);
        if (x1 > x2) {
            t = x1;
            x1 = x2;
            x2 = t;
        }
        if (y1 > y2) {
            t = y1;
            y1 = y2;
            y2 = t;
        }
        if (g_clsnScaleX == 1.0f && g_clsnScaleY == 1.0f) {
            Clsn_AddBox(clsn, x1, y1, x2, y2);
        } else {
            /* The original multiplies the exact integer on the x87 stack (fild + fmul);
             * converting to double first keeps it from being rounded to float. */
            Clsn_AddBox(clsn, (int)((double)x1 * g_clsnScaleX), (int)((double)y1 * g_clsnScaleY),
                        (int)((double)x2 * g_clsnScaleX), (int)((double)y2 * g_clsnScaleY));
        }
    }
    return clsn;

fail:
    Clsn_Release(clsn);
    return NULL;
}

/* 0x004799E0: 1 if a box of clsn, moved by (dx, dy), overlaps box. Edges touching count. */
int Clsn_HitBox(const Clsn *clsn, const ClsnBox *box, int dx, int dy)
{
    int i;

    for (i = 0; i < clsn->capacity; i++) {
        const ClsnBox *b = &clsn->boxes[i];
        if (b->x2 + dx >= box->x1 && b->x1 + dx <= box->x2 && b->y2 + dy >= box->y1 && b->y1 + dy <= box->y2) {
            return 1;
        }
    }
    return 0;
}

/* 0x00479A40: Clsn_HitBox with box flipped. */
int Clsn_HitBoxFlip(const Clsn *clsn, const ClsnBox *box, int dx, int dy)
{
    int i;

    for (i = 0; i < clsn->capacity; i++) {
        const ClsnBox *b = &clsn->boxes[i];
        if (b->x2 + dx >= -box->x2 && b->x1 + dx <= -box->x1 && b->y2 + dy >= box->y1 && b->y1 + dy <= box->y2) {
            return 1;
        }
    }
    return 0;
}

/* 0x00479AB0: Clsn_HitBox with both clsn (before the move) and box flipped. */
int Clsn_HitBoxFlipBoth(const Clsn *clsn, const ClsnBox *box, int dx, int dy)
{
    int i;

    for (i = 0; i < clsn->capacity; i++) {
        const ClsnBox *b = &clsn->boxes[i];
        if (dx - b->x1 >= -box->x2 && dx - b->x2 <= -box->x1 && b->y2 + dy >= box->y1 && b->y1 + dy <= box->y2) {
            return 1;
        }
    }
    return 0;
}

/* 0x00479B20: 1 if a box of a overlaps a box of b moved by (dx, dy). */
int Clsn_HitClsn(const Clsn *a, const Clsn *b, int dx, int dy)
{
    int i;

    if (a == NULL || b == NULL || a->valid == 0 || b->valid == 0) {
        return 0;
    }
    for (i = 0; i < a->capacity; i++) {
        if (Clsn_HitBox(b, &a->boxes[i], dx, dy)) {
            return 1;
        }
    }
    return 0;
}

/*
 * 0x00479B80: 1 if a box of a overlaps a box of b, b being at (dx, dy) from a.
 * flip_a/flip_b (0 or 1; anything else gives 0) mirror each list.
 */
int Clsn_Collide(int flip_a, int flip_b, const Clsn *a, const Clsn *b, int dx, int dy)
{
    int i;

    if (a == NULL || b == NULL || a->valid == 0 || b->valid == 0) {
        return 0;
    }
    if (flip_a == 0) {
        if (flip_b == 0) {
            return Clsn_HitClsn(a, b, dx, dy);
        }
        if (flip_b == 1) {
            for (i = 0; i < b->capacity; i++) {
                if (Clsn_HitBoxFlip(a, &b->boxes[i], -dx, -dy)) {
                    return 1;
                }
            }
        }
    } else if (flip_a == 1) {
        if (flip_b == 0) {
            for (i = 0; i < a->capacity; i++) {
                if (Clsn_HitBoxFlip(b, &a->boxes[i], dx, dy)) {
                    return 1;
                }
            }
        } else if (flip_b == 1) {
            for (i = 0; i < a->capacity; i++) {
                if (Clsn_HitBoxFlipBoth(b, &a->boxes[i], dx, dy)) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
