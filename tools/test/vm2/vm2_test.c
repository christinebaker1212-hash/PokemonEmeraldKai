// Moemon Kai: src/platform/vm2.c against the VMU manager's pretend VM2 (vmu_pet/manager/test/fake_vm2.py), run by
// tools/test/vm2/vm2_test.sh: the port found (MOEMON_VM2_PORT), opened, the internal memory read, one block written
// and read back, the software disconnect.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm2.h"

static uint8_t sImage[VM2_IMAGE_SIZE], sBack[VM2_IMAGE_SIZE];

int main(int argc, char **argv)
{
    char port[256];
    uint8_t block[VM2_BLOCK_SIZE];
    FILE *f;
    int i;
    if (argc < 2)
        return 2;
    if (!Vm2_FindPort(port, sizeof port)) { printf("FAIL find\n"); return 1; }
    if (!Vm2_Open(port)) { printf("FAIL open %s\n", port); return 1; }
    printf("info: %s\n", Vm2_Info());
    if (!Vm2_ReadInternal(sImage)) { printf("FAIL read\n"); return 1; }
    f = fopen(argv[1], "rb");
    if (!f || fread(sBack, 1, VM2_IMAGE_SIZE, f) != VM2_IMAGE_SIZE) { printf("FAIL the internal file\n"); return 1; }
    fclose(f);
    if (memcmp(sImage, sBack, VM2_IMAGE_SIZE) != 0) { printf("FAIL read differs\n"); return 1; }
    for (i = 0; i < VM2_BLOCK_SIZE; i++)
        block[i] = (uint8_t)(i * 7 + 3);
    if (!Vm2_WriteBlock(200, block)) { printf("FAIL write\n"); return 1; }
    if (!Vm2_ReadInternal(sImage) || memcmp(sImage + 200 * VM2_BLOCK_SIZE, block, VM2_BLOCK_SIZE) != 0) { printf("FAIL write differs\n"); return 1; }
    Vm2_Restart();
    if (Vm2_IsOpen()) { printf("FAIL still open\n"); return 1; }
    printf("PASS vm2: found, opened, read, wrote a block, restarted\n");
    return 0;
}
