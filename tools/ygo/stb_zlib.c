/* The zlib decoder the game takes from stb_image (src/mp/mp_avatar.c builds it there), for the native tests */
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_STDIO_WRAPPERS
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "../../src/mp/third_party/stb_image.h"
