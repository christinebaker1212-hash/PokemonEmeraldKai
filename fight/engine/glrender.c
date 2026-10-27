/*
 * OpenGL renderer (original module 0x479CA0-0x47CC0F): textures (GlTex) and
 * the video memory they use, set-up and statistics, the projection, the
 * OpenGLScreen path (the RGB565 screen buffer drawn as one texture), sprite
 * drawing through GLSL programs, the clip rectangle as a scissor box, screen
 * capture, render targets ("vbufs": textures with a depth renderbuffer
 * attached to a framebuffer object), presenting, and the shaders (loaded
 * from data/<name>.vert/.frag with #defines inserted at a marker).
 *
 * GL 1.1 entry points come from opengl32; newer ones through mugen.exe's own
 * GLEW copy (the g_glew* globals), which glewInit fills.
 *
 * Floating point follows the x87 code of the original: values the original
 * keeps on the FPU stack are doubles here, values it stores are floats.
 *
 * Quirks kept from the original:
 * - Gl_DrawQuadIndexed leaves texture unit 1 active, binds the palette
 *   texture to GL_TEXTURE_1D although streamed palettes are 2D textures, and
 *   does not disable GL_TEXTURE_2D;
 * - Gl_DrawSprite leaves GL_BLEND enabled for masked, unblended sprites that
 *   are not 32-bit; it negates the add colour of the applied PalFX in place
 *   (g_palFxApplied) to emulate inversion with blending;
 * - GlTex_Delete clamps the video memory count at 0 instead of going
 *   negative.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

#include "mugen.h"

/* GL constants newer than GL 1.1. */
#define GLR_CLAMP_TO_EDGE               0x812F
#define GLR_TEXTURE0                    0x84C0
#define GLR_TEXTURE1                    0x84C1
#define GLR_CONSTANT_COLOR              0x8001
#define GLR_CONSTANT_ALPHA              0x8003
#define GLR_FUNC_ADD                    0x8006
#define GLR_FUNC_REVERSE_SUBTRACT       0x800B
#define GLR_SHADING_LANGUAGE_VERSION    0x8B8C
#define GLR_FRAGMENT_SHADER             0x8B30
#define GLR_VERTEX_SHADER               0x8B31
#define GLR_INFO_LOG_LENGTH             0x8B84
#define GLR_FRAMEBUFFER                 0x8D40
#define GLR_RENDERBUFFER                0x8D41
#define GLR_COLOR_ATTACHMENT0           0x8CE0
#define GLR_DEPTH_ATTACHMENT            0x8D00
#define GLR_FRAMEBUFFER_COMPLETE        0x8CD5

/* Marker in shader sources replaced by the #define lines. */
#define GLR_DEFINES_MARK "////*EB_DEFINES*////"

/* Size of the #define text of a program. */
#define GLR_DEFINES_SIZE 0x1FA0

/* Drains the GL error queue. */
static void Glr_ClearErrors(void)
{
    while (glGetError() != GL_NO_ERROR) {
    }
}

/* Drains the GL error queue, returning the last error (GL_NO_ERROR for none). */
static GLenum Glr_LastError(void)
{
    GLenum err = GL_NO_ERROR;
    GLenum e;

    while ((e = glGetError()) != GL_NO_ERROR) {
        err = e;
    }
    return err;
}

/* Counts an allocation of bytes of video memory and the peak. */
static void Glr_AddMemory(GlTex *tex, unsigned int bytes)
{
    tex->size = bytes;
    g_glTexMemory += bytes;
    if (g_glTexMemory > g_glTexMemoryPeak) {
        g_glTexMemoryPeak = g_glTexMemory;
    }
}

static Sprite *Glr_SpriteAt(const SpriteSet *set, int index)
{
    const KeyedArray *a = set->sprites;

    return (Sprite *)(a->data + a->elem_size * index);
}

/* ---- textures ---- */

/* 0x00479CA0: a new texture object with a GL texture name; NULL on failure. */
GlTex *GlTex_Create(void)
{
    GlTex *tex = calloc(1, sizeof(GlTex));

    if (tex == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    if (!GlTex_Init(tex)) {
        if (GlTex_Delete(tex)) {
            free(tex);
        }
        return NULL;
    }
    return tex;
}

/* 0x00479CF0: deletes and frees a texture; 0 (nothing freed) while it is referenced. */
int GlTex_Free(GlTex *tex)
{
    if (tex != NULL) {
        if (!GlTex_Delete(tex)) {
            return 0;
        }
        free(tex);
    }
    return 1;
}

/* 0x00479D20: clears tex and generates its texture name; 0 on a GL error. */
int GlTex_Init(GlTex *tex)
{
    Glr_ClearErrors();
    tex->id = 0;
    tex->w = 0;
    tex->h = 0;
    tex->unused_0c = 0;
    tex->depth = 0;
    tex->refs = 0;
    tex->size = 0;
    glGenTextures(1, &tex->id);
    return Glr_LastError() == GL_NO_ERROR;
}

/*
 * 0x00479D70: deletes the GL objects of tex (not the structure); 0 while it
 * is referenced.
 */
int GlTex_Delete(GlTex *tex)
{
    if (tex->refs > 0) {
        return 0;
    }
    if (tex->depth != 0) {
        g_glewDeleteRenderbuffersEXT(1, &tex->depth);
    }
    tex->depth = 0;
    if (tex->id != 0) {
        glDeleteTextures(1, &tex->id);
        if (g_glTexMemory < tex->size) {
            g_glTexMemory = 0;
        } else {
            g_glTexMemory -= tex->size;
        }
    }
    tex->id = 0;
    return 1;
}

/* 0x00479E00: allocates w x h RGBA texels for tex (no data). */
int GlTex_Alloc(GlTex *tex, int w, int h)
{
    GLenum err;

    Glr_ClearErrors();
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    err = Glr_LastError();
    if (err == GL_NO_ERROR) {
        Glr_AddMemory(tex, Gl_TexBytes((unsigned int)(w * h), GL_RGBA));
    }
    tex->h = h;
    tex->w = w;
    return err == GL_NO_ERROR;
}

/*
 * 0x00479EC0: uploads w x h pixels of bpp bits (8: luminance, 24: RGB, 32:
 * premultiplied RGBA) to tex. Other depths return 0.
 */
int GlTex_Upload2D(int h, int bpp, GlTex *tex, const void *pixels, int w)
{
    if (bpp <= 8) {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        return GlTex_Upload(tex, h, GL_LUMINANCE, w, GL_LUMINANCE, pixels);
    }
    if (bpp == 24) {
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        return GlTex_Upload(tex, h, GL_RGB, w, GL_RGB, pixels);
    }
    if (bpp == 32) {
        return GlTex_UploadRGBA(h, w, tex, pixels);
    }
    return 0;
}

/* 0x00479F60: uploads w x h RGBA pixels to tex, premultiplied by their alpha (NULL: no data). */
int GlTex_UploadRGBA(int h, int w, GlTex *tex, const void *pixels)
{
    unsigned char *premul = NULL;
    int ok;

    if (pixels != NULL) {
        premul = Gl_Premultiply(w * h * 4, pixels);
        if (premul == NULL) {
            return 0;
        }
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    ok = GlTex_Upload(tex, h, GL_RGBA, w, GL_RGBA, premul != NULL ? premul : pixels);
    free(premul);
    return ok;
}

/* 0x00479FE0: binds tex and uploads w x h pixels (nearest filtering, clamped). */
int GlTex_Upload(GlTex *tex, int h, int internal_format, int w, int format, const void *pixels)
{
    GLenum err;

    Glr_ClearErrors();
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, pixels);
    err = Glr_LastError();
    if (err == GL_NO_ERROR) {
        Glr_AddMemory(tex, Gl_TexBytes((unsigned int)(w * h), internal_format));
    }
    tex->w = w;
    tex->h = h;
    return err == GL_NO_ERROR;
}

/* 0x0047A0E0: uploads width RGBA pixels to tex as a 1D texture (palettes). */
int GlTex_Upload1D(GlTex *tex, int width, const void *pixels)
{
    GLenum err;

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    Glr_ClearErrors();
    glBindTexture(GL_TEXTURE_1D, tex->id);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GLR_CLAMP_TO_EDGE);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    err = Glr_LastError();
    if (err == GL_NO_ERROR) {
        Glr_AddMemory(tex, Gl_TexBytes((unsigned int)width, GL_RGBA));
    }
    tex->w = width;
    tex->h = 0;
    return err == GL_NO_ERROR;
}

/* 0x0047A1F0: gives a texture a depth renderbuffer of its size (render targets). */
int GlTex_AllocDepth(GlTex *tex)
{
    Glr_ClearErrors();
    if (tex->id == 0) {
        return 0;
    }
    if (tex->depth == 0) {
        g_glewGenRenderbuffersEXT(1, &tex->depth);
        g_glewBindRenderbufferEXT(GLR_RENDERBUFFER, tex->depth);
        g_glewRenderbufferStorageEXT(GLR_RENDERBUFFER, GL_DEPTH_COMPONENT, tex->w, tex->h);
        g_glewBindRenderbufferEXT(GLR_RENDERBUFFER, 0);
    }
    return Glr_LastError() == GL_NO_ERROR;
}

/* ---- OpenGLScreen ---- */

/* 0x0047A260: uploads the w x h RGBA screen to the top left of g_glScreenTexCur. */
int GlScreen_Upload(int h, int w, const void *pixels)
{
    GlTex *tex = g_glScreenTexCur;
    GLenum err;

    Glr_ClearErrors();
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    err = Glr_LastError();
    g_glScreenUploadW = w;
    g_glScreenUploadH = h;
    return err == GL_NO_ERROR;
}

/* 0x0047A320: draws the uploaded screen over w x h pixels of the window. */
void GlScreen_Draw(int w, int h)
{
    GlTex *tex = g_glScreenTexCur;
    float u = (float)((double)g_glScreenUploadW / tex->w);
    float v = (float)((double)g_glScreenUploadH / tex->h);
    float fw, fh;

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (float)GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glTexCoord2f(u, 0.0f);
    fw = (float)w;
    glVertex3f(fw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, v);
    fh = (float)h;
    glVertex3f(0.0f, fh, 0.0f);
    glTexCoord2f(u, v);
    glVertex3f(fw, fh, 0.0f);
    glEnd();
    glFlush();
    glDisable(GL_TEXTURE_2D);
}

/*
 * 0x0047A460: video memory of pixels texels of a GL internal format, as the
 * original estimates it (its own table, with unsigned wrapping).
 */
unsigned int Gl_TexBytes(unsigned int pixels, int format)
{
    if (format > 0x803C) {
        if (format == 0x81A5) {
            return pixels * 2;
        }
        if (format < 0x81A5) {
            /* 0x803D..0x805B */
            static const unsigned char sizes[] = {
                2, 2, 1, 1, 2, 2, 2, 2, 2, 4, 4, 4, 1, 1, 1, 2, 2, 1, 2, 2, 4, 4, 8, 8, 1, 2, 4, 4, 4, 8, 8,
            };
            unsigned int i = (unsigned int)(format - 0x803D);
            return i < sizeof(sizes) ? pixels * sizes[i] : pixels;
        }
        if (format > 0x8815) {
            if (format == 0x8C40 || format == 0x8C41) {
                return pixels * 4;
            }
            if (format > 0x8C41) {
                /* 0x8C42..0x8C47 */
                static const unsigned char sizes[] = { 8, 8, 2, 2, 1, 1 };
                unsigned int i = (unsigned int)(format - 0x8C42);
                return i < sizeof(sizes) ? pixels * sizes[i] : pixels;
            } else {
                /* 0x8816..0x881E */
                static const unsigned char sizes[] = { 4, 4, 4, 1, 8, 8, 4, 2, 2 };
                unsigned int i = (unsigned int)(format - 0x8816);
                return i < sizeof(sizes) ? pixels * sizes[i] : pixels;
            }
        }
        if (format >= 0x8814) {
            return pixels << 4;
        }
        if (format > 0x84EA) {
            switch (format) {
            case 0x84EC:
                return pixels >> 1;
            case 0x84ED:
                return (pixels * 4) >> 3;
            case 0x84EE:
                return (pixels * 4) >> 2;
            default:
                return pixels;
            }
        }
        if (format >= 0x84E9) {
            return pixels >> 1;
        }
        if (format > 0x83F1) {
            return format <= 0x83F3 ? (pixels * 4) >> 2 : pixels;
        }
        if (format >= 0x83F0) {
            return (pixels * 4) >> 3;
        }
        if (format >= 0x81A6 && format <= 0x81A7) {
            return pixels * 4;
        }
        return pixels;
    }
    if (format >= 0x803B) {
        return pixels;
    }
    if (format > 0x1906) {
        if (format > 0x190A) {
            return pixels;
        }
        if (format == 0x190A) {
            return pixels * 2;
        }
        return format <= 0x1908 ? pixels * 4 : pixels;
    }
    if (format == 0x1906 || format > 4) {
        return pixels;
    }
    if (format >= 3) {
        return pixels * 4;
    }
    if (format == 2) {
        return pixels * 2;
    }
    return pixels;
}

/* 0x0047A640: a malloc'ed copy of bytes / 4 RGBA pixels with the colours multiplied by alpha. */
unsigned char *Gl_Premultiply(int bytes, const unsigned char *src)
{
    unsigned char *out = malloc(bytes);
    unsigned char *dst;
    int n;

    if (out == NULL) {
        return NULL;
    }
    dst = out;
    for (n = bytes / 4; n > 0; n--) {
        unsigned int a = src[3];
        dst[0] = (unsigned char)(src[0] * a / 255);
        dst[1] = (unsigned char)(src[1] * a / 255);
        dst[2] = (unsigned char)(src[2] * a / 255);
        dst[3] = (unsigned char)a;
        src += 4;
        dst += 4;
    }
    return out;
}

/* ---- set-up ---- */

/*
 * 0x0047A6D0: starts the renderer once (glewInit, driver strings in the
 * match log, the system textures). framebuffers: the OpenGL renderer, which
 * needs GL 2.0, NPOT textures, framebuffer objects and fragment programs,
 * detects the sprite offsets and loads the shaders. Returns 0 on failure.
 */
int Gl_Init(int framebuffers)
{
    int h = g_videoHeight;
    int w = g_videoWidth;
    const GLubyte *s;

    if (g_glInitialized) {
        return 1;
    }
    glewInit();
    if ((s = glGetString(GL_VERSION)) != NULL) {
        MatchLog_Printf(7, "Version: %s\n", s);
    }
    if ((s = glGetString(GL_RENDERER)) != NULL) {
        MatchLog_Printf(7, "Renderer: %s\n", s);
    }
    if ((s = glGetString(GL_VENDOR)) != NULL) {
        MatchLog_Printf(7, "Vendor: %s\n", s);
    }
    if ((s = glGetString(GLR_SHADING_LANGUAGE_VERSION)) != NULL) {
        MatchLog_Printf(7, "GLSL version: %s\n", s);
    }
    if ((s = glGetString(GL_EXTENSIONS)) != NULL) {
        MatchLog_Printf(7, "Extensions: %s\n", s);
    }
    if (framebuffers == 1) {
        if (!g_glewVersion20) {
            MatchLog_Printf(3, "GL: OpenGL version is below 2.0 (2.0 or higher required).\n");
            return 0;
        }
        if (!g_glewArbTextureNpot) {
            MatchLog_Printf(3, "GL: Video driver does not support ARB_texture_non_power_of_two.\n");
            return 0;
        }
        if (!g_glewExtFramebufferObject) {
            MatchLog_Printf(3, "GL: Video driver does not support EXT_framebuffer_object.\n");
            return 0;
        }
        if (!g_glewArbFragmentProgram) {
            MatchLog_Printf(3, "GL: Video driver does not support ARB_fragment_program.\n");
            return 0;
        }
    }
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &g_glMaxTextureSize);
    MatchLog_Printf(7, "GL: Max texture size %i.\n", g_glMaxTextureSize);
    g_glSysTex[0] = GlTex_Create();
    if (g_glSysTex[0] == NULL) {
        MatchLog_Printf(3, "GL: Could not create system texture.\n");
        return 0;
    }
    g_glSysTex[1] = GlTex_Create();
    if (g_glSysTex[1] == NULL) {
        MatchLog_Printf(3, "GL: Could not create system texture.\n");
        return 0;
    }
    glViewport(0, 0, w, h);
    if (framebuffers == 1) {
        Gl_DetectOffsets();
        if (!Gl_LoadShaders()) {
            MatchLog_Printf(3, "GL: Could not load shaders.\n");
            return 0;
        }
    }
    g_glProjection = 0;
    Gl_SetupView();
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    g_gfxFlags |= 1;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    g_glInitialized = 1;
    return 1;
}

/* Deletes and frees a system texture (unless it is referenced). */
static void Glr_FreeSysTex(GlTex *tex)
{
    if (tex != NULL && GlTex_Delete(tex)) {
        free(tex);
    }
}

/* 0x0047A930: stops the renderer and logs the draw statistics. */
int Gl_Deinit(void)
{
    if (g_glInitialized) {
        Gl_FreeShaders();
        Glr_FreeSysTex(g_glSysTex[1]);
        Glr_FreeSysTex(g_glSysTex[0]);
        g_glInitialized = 0;
    }
    MatchLog_Printf(7,
                    "GL draw statistics:\nSprite Texture: %llu\nSprite Stream : %llu\nPal Texture   : %llu\nPal "
                    "Stream    : %llu\nTex compress  : %llu\nMax used tex mem: %lluk\n",
                    g_glStatSpriteTexture, g_glStatSpriteStream, g_glStatPalTexture, g_glStatPalStream,
                    g_glStatTexCompress, g_glTexMemoryPeak >> 10);
    return 1;
}

/*
 * 0x0047AA00: the projection for the render target: orthographic (y down on
 * the screen, up in a framebuffer) or perspective (g_glProjection 1), then the
 * z scale.
 */
int Gl_SetupView(void)
{
    int h = g_targetHeight;
    int w = g_targetWidth;
    int bottom, top;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (g_glProjection == 0) {
        bottom = 0;
        top = h;
        if (g_glBoundFramebuffer == 0) {
            bottom = h;
            top = 0;
        }
        glOrtho(0.0, (double)w, (double)bottom, (double)top, g_glNear, g_glFar);
        glTranslated(0.0, 0.0, g_glNear * -2.0);
    } else if (g_glProjection == 1) {
        double nh = (double)-h;
        double nw = (double)-w;
        double a = nh * 0.25;
        double b = (double)h * 0.25;
        if (g_glBoundFramebuffer == 0) {
            double t = a;
            a = b;
            b = t;
        }
        glFrustum(nw * 0.25, (double)w * 0.25, a, b, g_glNear, g_glFar);
        glTranslated(0.5 * nw, nh * 0.5, g_glNear * -2.0);
    }
    glScaled(1.0, 1.0, g_glZScale);
    return 1;
}

/* 0x0047AB80: identity modelview moved by the detected sprite offset; no program in use. */
void Gl_ResetModelview(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(g_glOffsetX, g_glOffsetY, 0.0f);
    if (g_glCurrentProgram != 0) {
        g_glewUseProgram(0);
        g_glCurrentProgram = 0;
    }
}

/* ---- sprites ---- */

/* PalFX colour terms of the programs: balance, add (/255), mul (/256). */
typedef struct GlrFx {
    float colorbal;
    float add[3];
    float mul[3];
} GlrFx;

static void Glr_FxTerms(const PalFx *fx, GlrFx *t)
{
    t->colorbal = (float)(fx->color * (1.0 / 256));
    t->add[0] = (float)(fx->add[0] / 255.0);
    t->add[1] = (float)(fx->add[1] / 255.0);
    t->add[2] = (float)(fx->add[2] / 255.0);
    t->mul[0] = (float)(fx->mul[0] * (1.0 / 256));
    t->mul[1] = (float)(fx->mul[1] * (1.0 / 256));
    t->mul[2] = (float)((1.0 / 256) * fx->mul[2]);
}

/*
 * Program variant of a quad: 0 plain, 1 add/mul, 2 colour balance or
 * inversion; + 6 for stretched / tinted quads.
 */
static int Glr_Variant(const PalFx *fx, const GlrFx *t, int special)
{
    int mode = 0;

    if (0.0f != t->add[0] || 0.0f != t->add[1] || 0.0f != t->add[2] || 1.0f != t->mul[0] || 1.0f != t->mul[1] ||
        1.0f != t->mul[2]) {
        mode = 1;
    }
    if (fx->invertall != 0 || 1.0f != t->colorbal) {
        mode = 2;
    }
    return mode + (special ? 6 : 0);
}

/*
 * Settings of a quad from its draw parameters: special (stretched with
 * top_stretch / bottom_stretch, offset with top_offset / bottom_offset or
 * squeezed with width_ratio), and the stretch factors (not above 0 means 1).
 */
static int Glr_QuadSpecial(const DrawParams *dp, float *c0, float *c1)
{
    int special = !(1.0f == dp->top_stretch && 1.0f == dp->bottom_stretch && 0.0f == dp->top_offset && 0.0f == dp->bottom_offset &&
                    1.0f == dp->width_ratio);

    *c0 = dp->top_stretch;
    *c1 = dp->bottom_stretch;
    if (!(0.0f < *c0)) {
        *c0 = 1.0f;
    }
    if (!(0.0f < *c1)) {
        *c1 = 1.0f;
    }
    return special;
}

/* Sets the uniforms of a program. */
static void Glr_SetUniforms(const GlProgram *prog, const PalFx *fx, const GlrFx *t, const GlTex *tex,
                            const DrawParams *dp, int mask)
{
    g_glewUniform1i(prog->texunit0, 0);
    g_glewUniform1i(prog->texunit1, 1);
    g_glewUniform1i(prog->mask, mask != 0);
    g_glewUniform3f(prog->add, t->add[0], t->add[1], t->add[2]);
    g_glewUniform3f(prog->mul, t->mul[0], t->mul[1], t->mul[2]);
    g_glewUniform1i(prog->invertall, fx->invertall);
    g_glewUniform1f(prog->colorbal, t->colorbal);
    g_glewUniform2f(prog->texsize, (float)tex->w, (float)tex->h);
    g_glewUniform1f(prog->src_alphamul, dp->alpha_src);
}

/*
 * The four corners of a quad with texture coordinates; stretched quads use
 * projective coordinates for the bottom edge.
 */
static void Glr_QuadVertices(const float *v, const DrawParams *dp, float c0, float c1, int special)
{
    if (!special) {
        g_glewMultiTexCoord2f(GLR_TEXTURE0, 0.0f, 0.0f);
        glVertex2f(v[0], v[1]);
        g_glewMultiTexCoord2f(GLR_TEXTURE0, 1.0f, 0.0f);
        glVertex2f(v[2], v[3]);
        g_glewMultiTexCoord2f(GLR_TEXTURE0, 0.0f, 1.0f);
        glVertex2f(v[4], v[5]);
        g_glewMultiTexCoord2f(GLR_TEXTURE0, 1.0f, 1.0f);
    } else {
        float q = (float)((double)c0 / c1);
        double c2x2 = (double)dp->top_offset + dp->top_offset;
        double c3x2;
        float s;
        if (0.0f != dp->width_ratio) {
            q = (float)((double)q / dp->width_ratio);
        }
        s = (float)(((1.0 - c0) - c2x2) * 0.5);
        g_glewMultiTexCoord2f(GLR_TEXTURE0, s, 0.0f);
        glVertex2f(v[0], v[1]);
        s = (float)((c0 + 1.0 - c2x2) * 0.5);
        g_glewMultiTexCoord2f(GLR_TEXTURE0, s, 0.0f);
        glVertex2f(v[2], v[3]);
        c3x2 = (double)dp->bottom_offset + dp->bottom_offset;
        s = (float)(q * (((1.0 - c1) - c3x2) * 0.5));
        g_glewMultiTexCoord4f(GLR_TEXTURE0, s, 1.0f, 0.0f, q);
        glVertex2f(v[4], v[5]);
        s = (float)(q * ((c1 + 1.0 - c3x2) * 0.5));
        g_glewMultiTexCoord4f(GLR_TEXTURE0, s, 1.0f, 0.0f, q);
    }
    glVertex2f(v[6], v[7]);
}

/* Uses a program unless it is current. */
static void Glr_UseProgram(unsigned int program)
{
    if (g_glCurrentProgram != program) {
        g_glewUseProgram(program);
        g_glCurrentProgram = program;
    }
}

/*
 * 0x0047ABD0: draws an RGBA sprite texture as the quad with corners
 * (x0, y0)..(x3, y3) (triangle strip order) through an "rgba" program for
 * the PalFX fx.
 */
void Gl_DrawQuadRgba(const PalFx *fx, GlTex *tex, float x0, float y0, float x1, float y1, float x2, float y2,
                     float x3, float y3, const DrawParams *dp)
{
    float v[8] = { x0, y0, x1, y1, x2, y2, x3, y3 };
    float c0, c1;
    int mask = dp->masked;
    int special = Glr_QuadSpecial(dp, &c0, &c1);
    GlrFx t;
    const GlProgram *prog;

    Glr_FxTerms(fx, &t);
    prog = &g_glProgramsRgba[Glr_Variant(fx, &t, special)];
    Glr_UseProgram(prog->program);
    g_glewActiveTexture(GLR_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, special ? GL_REPEAT : GLR_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLR_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (prog->program != 0) {
        Glr_SetUniforms(prog, fx, &t, tex, dp, mask);
    }
    glBegin(GL_TRIANGLE_STRIP);
    Glr_QuadVertices(v, dp, c0, c1, special);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

/*
 * 0x0047B0D0: Gl_DrawQuadRgba for paletted sprites: an "indexed" program,
 * the sprite texture on unit 0 and the palette on unit 1.
 */
void Gl_DrawQuadIndexed(const PalFx *fx, GlTex *tex, GlTex *pal, float x0, float y0, float x1, float y1, float x2,
                        float y2, float x3, float y3, const DrawParams *dp)
{
    float v[8] = { x0, y0, x1, y1, x2, y2, x3, y3 };
    float c0, c1;
    int mask = dp->masked;
    int special = Glr_QuadSpecial(dp, &c0, &c1);
    GlrFx t;
    const GlProgram *prog;

    Glr_FxTerms(fx, &t);
    prog = &g_glProgramsIndexed[Glr_Variant(fx, &t, special)];
    Glr_UseProgram(prog->program);
    g_glewActiveTexture(GLR_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, special ? GL_REPEAT : GL_CLAMP);
    g_glewActiveTexture(GLR_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, pal->id);
    if (prog->program != 0) {
        Glr_SetUniforms(prog, fx, &t, tex, dp, mask);
    }
    glBegin(GL_TRIANGLE_STRIP);
    Glr_QuadVertices(v, dp, c0, c1, special);
    glEnd();
}

/* -1 for a set flip bit, 1 otherwise. */
static int Glr_FlipSign(unsigned int flip, unsigned int bit)
{
    return (flip & bit) ? -1 : 1;
}

/*
 * 0x0047B830: draws a sprite with the draw parameters: the sprite (uploaded
 * to a system texture unless it is a texture) and its palette (likewise) go
 * through the programs, transformed by the modelview built from dp, with
 * blending set from dp.
 */
void Gl_DrawSprite(int sprite, const DrawParams *dp, SpriteSet *set)
{
    Sprite *spr = Glr_SpriteAt(set, sprite);
    GlTex *tex = g_glSysTex[0];
    GlTex *pal_tex = g_glSysTex[1];
    float m[16] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
    int sx1 = Glr_FlipSign(dp->flip, DRAW_FLIP_H);
    int sy1 = Glr_FlipSign(dp->flip, DRAW_FLIP_V);
    int sx2 = Glr_FlipSign(dp->flip2, DRAW_FLIP_H);
    int sy2 = Glr_FlipSign(dp->flip2, DRAW_FLIP_V);
    int blend = dp->blend;
    int blended = blend != 0;
    int paletted = spr->bpp <= 8;
    int rgba32 = spr->bpp == 32;
    int w, h;
    float fw, fh, left, right;
    PalFx *fx;

    if (0.0f == dp->scale2_x || 0.0f == dp->scale2_y || 0.0f == dp->scale_x || 0.0f == dp->scale_y) {
        return;
    }
    w = spr->w;
    h = spr->h;
    if (spr->format == SPRITE_FORMAT_TEXTURE) {
        g_glStatSpriteTexture++;
        tex = spr->pixels;
    } else {
        void *pixels = Sprite_GetPixels(set, sprite);
        if (pixels == NULL || !GlTex_Upload2D(h, spr->bpp, tex, pixels, w)) {
            return;
        }
        g_glStatSpriteStream++;
    }
    if (paletted) {
        Palette *pal = SpriteSet_GetPaletteConverted(set, spr->pal_index);
        if (pal->is_texture == 1) {
            g_glStatPalTexture++;
            pal_tex = pal->data;
        } else {
            int n = pal->count;
            const void *colors = pal->is_texture == 0 ? pal->data : NULL;
            if (n == 0x100) {
                GlTex_UploadRGBA(1, 0x100, pal_tex, colors);
            } else if (n < 0x100) {
                unsigned char padded[0x100 * 4];
                memcpy(padded, colors, n * 4);
                memset(padded + n * 4, 0, (0x100 - n) * 4);
                GlTex_UploadRGBA(1, 0x100, pal_tex, padded);
            }
            g_glStatPalStream++;
        }
    }
    fx = PalFx_GetApplied();
    if (fx->invertall != 0) {
        /* Inversion through blending: add <-> subtract with negated offsets. */
        int swap = 0;
        if (blend == BLEND_ADD) {
            if (1.0f == dp->alpha_dst) {
                blend = BLEND_SUB;
                swap = 1;
            }
        } else if (blend == BLEND_SUB) {
            blend = BLEND_ADD;
            swap = 1;
        }
        if (swap) {
            fx->add[0] = -fx->add[0];
            fx->invertall = 0;
            fx->add[1] = -fx->add[1];
            fx->add[2] = -fx->add[2];
        }
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (paletted) {
        glTranslatef(g_glPalOffset, g_glPalOffset, 0.0f);
    } else {
        glTranslatef(g_glRgbaOffsetX, g_glRgbaOffsetY, 0.0f);
    }
    if ((double)(sx2 * sx1) * dp->scale2_x * dp->scale_x < 0.0) {
        glTranslatef(0.99999899f, 0.0f, 0.0f);
    }
    if ((double)(sy2 * sy1) * dp->scale2_y * dp->scale_y < 0.0) {
        glTranslatef(0.0f, 0.99999899f, 0.0f);
    }
    glTranslatef(dp->x, dp->y, 0.0f);
    glScalef((float)((double)sx2 * dp->scale2_x), (float)((double)sy2 * dp->scale2_y), 1.0f);
    glRotatef(dp->angle_y, 1.0f, 0.0f, 0.0f);
    glRotatef(dp->angle_z, 0.0f, 1.0f, 0.0f);
    glRotatef(-dp->angle, 0.0f, 0.0f, 1.0f);
    m[4] = dp->shear_x;
    m[1] = dp->shear_y;
    glMultMatrixf(m);
    glScalef((float)((double)sx1 * dp->scale_x), (float)((double)sy1 * dp->scale_y), 1.0f);
    glTranslatef((float)-spr->x, (float)-spr->y, 0.0f);

    if (blended) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GLR_CONSTANT_ALPHA);
        g_glewBlendColor(dp->alpha_src, dp->alpha_src, dp->alpha_src, dp->alpha_dst);
        if (blend == BLEND_ADD) {
            double sum;
            g_glewBlendEquation(GLR_FUNC_ADD);
            sum = (double)dp->alpha_src + dp->alpha_dst;
            if ((double)0.999f <= sum && sum <= (double)1.001f) {
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            }
        } else if (blend == BLEND_SUB) {
            g_glewBlendEquation(GLR_FUNC_REVERSE_SUBTRACT);
        }
    } else if (dp->masked != 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        g_glewBlendEquation(GLR_FUNC_ADD);
    }

    /* The quad: the top edge squeezed by width_ratio around width_anchor. */
    fw = (float)w;
    fh = (float)h;
    left = 0.0f;
    right = fw;
    if (!(1.0f == dp->width_ratio)) {
        float d = (float)((double)fw * dp->width_ratio - fw);
        left = (float)(0.0 - (double)d * dp->width_anchor);
        right = (float)((1.0 - dp->width_anchor) * d + fw);
    }
    if (paletted) {
        Gl_DrawQuadIndexed(fx, tex, pal_tex, left, 0.0f, right, 0.0f, 0.0f, fh, fw, fh, dp);
    } else {
        Gl_DrawQuadRgba(fx, tex, left, 0.0f, right, 0.0f, 0.0f, fh, fw, fh, dp);
    }
    if (blended || rgba32) {
        glDisable(GL_BLEND);
    }
}

/*
 * 0x0047B5C0: draws a sprite with the OpenGL renderer. With g_glSpriteBufferFlags & 3
 * == 3, scaled or rotated (other than by multiples of 90 degrees) paletted
 * sprites are first drawn at their size into a buffer, which is then drawn
 * with the draw parameters.
 */
void Sprite_DrawGL(SpriteSet *set, int sprite, const DrawParams *dp)
{
    Sprite *spr = Glr_SpriteAt(set, sprite);
    SpriteSet *target, *buf;
    int target_sprite, cx1, cy1, cx2, cy2;
    float saved_sx, saved_sy, scale;
    DrawParams first;
    Sprite *bspr;

    if ((g_glSpriteBufferFlags & 3) == 3 && spr->bpp <= 8) {
        double a;
        scale = (float)fabs((double)dp->scale_x * spr->scale_x * dp->scale2_x);
        if (!(scale != 1.0f)) {
            scale = (float)fabs((double)spr->scale_y * dp->scale_y * dp->scale2_y);
            if (!(scale != 1.0f)) {
                a = dp->angle / 90.0;
                if ((double)Ftol(a) == a) {
                    Gl_DrawSprite(sprite, dp, set);
                    return;
                }
            }
        }
        target = g_renderTarget;
        target_sprite = g_renderTargetSprite;
        cx1 = g_clipX1;
        cy1 = g_clipY1;
        cx2 = g_clipX2;
        cy2 = g_clipY2;
        memset(&first, 0, sizeof(first));
        first.scale2_y = 1.0f;
        first.scale2_x = 1.0f;
        first.scale_y = 1.0f;
        first.scale_x = 1.0f;
        first.alpha_src = 1.0f;
        first.bottom_stretch = 1.0f;
        first.top_stretch = 1.0f;
        first.flags = 0xFFFF;
        first.width_ratio = 1.0f;
        first.masked = 1;
        first.width_anchor = 0.5f;
        buf = Screen_Create(g_videoDepth, spr->w, spr->h);
        if (buf == NULL) {
            MatchLog_Printf(5, "DrawSprite: could not allocate pass 1 buffer\n");
            return;
        }
        Gfx_SetRenderTarget(buf, 0);
        g_gfxClear(0);
        saved_sx = spr->scale_x;
        saved_sy = spr->scale_y;
        bspr = Glr_SpriteAt(buf, 0);
        spr->scale_y = 1.0f;
        spr->scale_x = 1.0f;
        first.x = (float)spr->x;
        first.y = (float)spr->y;
        Gl_DrawSprite(sprite, &first, set);
        Gfx_SetRenderTarget(target, target_sprite);
        Gfx_SetClipRect(cx1, cy1, cx2, cy2);
        spr->scale_x = saved_sx;
        spr->scale_y = saved_sy;
        bspr->x = spr->x;
        bspr->y = spr->y;
        bspr->scale_x = spr->scale_x;
        bspr->scale_y = spr->scale_y;
        Gl_DrawSprite(0, dp, buf);
        if (g_renderTarget == buf) {
            Gfx_SetRenderTarget(NULL, 0);
            g_renderTarget = NULL;
            g_renderTargetSprite = 0;
        }
        SpriteSet_Free(buf);
        return;
    }
    Gl_DrawSprite(sprite, dp, set);
}

/* ---- clip rectangle, capture, render targets ---- */

/* 0x0047BEE0: the clip rectangle as the scissor box (disabled for the whole target). */
void Gl_ApplyClipRect(void)
{
    if (g_clipX1 == 0 && g_clipY1 == 0 && g_clipX2 == g_targetWidth && g_clipY2 == g_targetHeight) {
        glDisable(GL_SCISSOR_TEST);
        return;
    }
    glEnable(GL_SCISSOR_TEST);
    if (g_glBoundFramebuffer == 0) {
        glScissor(g_clipX1, g_targetHeight - g_clipY2, g_clipX2 - g_clipX1, g_clipY2 - g_clipY1);
    } else {
        glScissor(g_clipX1, g_clipY1, g_clipX2 - g_clipX1, g_clipY2 - g_clipY1);
    }
}

/* 0x0047BF90: g_gfxCapture of the OpenGL renderer: the whole render target. */
void *Gl_Capture(void)
{
    return Gl_ReadPixels(g_targetWidth, g_targetHeight);
}

/*
 * 0x0047BFB0: malloc'ed RGBA pixels of the bottom w x h of the render target
 * (top-down; alpha forced to 255). NULL when memory runs out.
 */
unsigned char *Gl_ReadPixels(int w, int h)
{
    int row = w * 4;
    int size = w * h * 4;
    int y0;
    unsigned char *buf, *tmp;

    Glr_ClearErrors();
    y0 = g_targetHeight - h;
    buf = malloc(size);
    if (buf == NULL) {
        return NULL;
    }
    tmp = malloc(row);
    if (tmp == NULL) {
        free(buf);
        return NULL;
    }
    if (g_glBoundFramebuffer == 0) {
        glReadBuffer(GL_BACK);
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, y0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);
    Glr_ClearErrors();
    if (g_glBoundFramebuffer == 0) {
        unsigned char *top = buf;
        unsigned char *bottom = buf + (h - 1) * row;
        int n;
        for (n = h / 2; n > 0; n--) {
            memcpy(tmp, bottom, row);
            memcpy(bottom, top, row);
            memcpy(top, tmp, row);
            top += row;
            bottom -= row;
        }
    }
    {
        int i;
        for (i = 3; i < size; i += 4) {
            buf[i] = 0xFF;
        }
    }
    free(tmp);
    return buf;
}

/*
 * 0x0047C100: a w x h render target texture of depth bits (24 or 32) with a
 * depth renderbuffer; NULL for lower depths and on failure.
 */
GlTex *GlVbuf_Alloc(int depth, int w, int h)
{
    GlTex *tex;

    if (depth <= 16) {
        MatchLog_Printf(3, "GL_VbufAlloc does not support coldepth %i\n", depth);
        return NULL;
    }
    tex = calloc(1, sizeof(GlTex));
    if (tex == NULL) {
        g_outOfMemory = 1;
        return NULL;
    }
    if (!GlTex_Init(tex) || !GlTex_Upload2D(h, depth, tex, NULL, w)) {
        if (GlTex_Delete(tex)) {
            free(tex);
        }
        return NULL;
    }
    if (!GlTex_AllocDepth(tex)) {
        GlTex_Free(tex);
        return NULL;
    }
    return tex;
}

/* 0x0047C1A0: renders to vbuf (NULL: the window), with the viewport and projection for it. */
int GlVbuf_Bind(GlTex *vbuf)
{
    int w, h;

    if (vbuf == NULL) {
        g_glewBindFramebufferEXT(GLR_FRAMEBUFFER, 0);
        h = g_videoHeight;
        w = g_videoWidth;
        g_glBoundFramebuffer = 0;
    } else {
        Gl_AttachFramebuffer(vbuf->id, vbuf->depth);
        h = vbuf->h;
        w = vbuf->w;
    }
    glViewport(0, 0, w, h);
    Gl_SetupView();
    return 1;
}

/*
 * 0x0047C1F0: g_gfxPresent of the OpenGL renderer: draws the screen buffer
 * (a texture sprite) to the window at (x, y), w x h (0 x 0: its size), lets
 * g_gfxPrePresent draw over the result, then swaps / flips and clears.
 */
void Gl_Present(SpriteSet *set, int sprite, float x, float y, float w, float h)
{
    if (set != NULL) {
        Sprite *spr = Glr_SpriteAt(set, sprite);
        SpriteSet *target;
        int target_sprite;
        DrawParams dp;
        if (spr == NULL || spr->format != SPRITE_FORMAT_TEXTURE) {
            return;
        }
        if (0.0f == w && 0.0f == h) {
            w = (float)spr->w;
            h = (float)spr->h;
        }
        target = g_renderTarget;
        target_sprite = g_renderTargetSprite;
        Gfx_SetRenderTarget(NULL, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawParams_Init(&dp);
        dp.x = x;
        dp.y = y;
        dp.flags |= 9;
        dp.masked = 0;
        dp.scale_x = (float)((double)w / spr->w);
        dp.scale_y = (float)((double)h / spr->h);
        Sprite_DrawGL(set, sprite, &dp);
        Gfx_SetRenderTarget(target, target_sprite);
    }
    if (g_gfxPrePresent != NULL) {
        SpriteSet *target = g_renderTarget;
        int target_sprite = g_renderTargetSprite;
        g_gfxPrePresent(NULL, 0);
        Gfx_SetRenderTarget(target, target_sprite);
    }
    if (g_videoFlags & VIDEO_HWBLIT) {
        if (g_videoSdlFlags & SDL_DOUBLEBUF) {
            if (g_renderMode == RENDER_OPENGL || g_renderMode == RENDER_OPENGL_SCREEN) {
                SDL_GL_SwapBuffers();
            } else {
                SDL_Flip(g_videoSurface);
            }
        }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth(1.0);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

/*
 * 0x0047C3B0: attaches texture (and depth renderbuffer) to the renderer's
 * framebuffer object and binds it; texture 0 binds the window. Logs an
 * incomplete framebuffer.
 */
void Gl_AttachFramebuffer(unsigned int texture, unsigned int depth)
{
    GLenum status;

    if (g_glFramebuffer == 0) {
        g_glewGenFramebuffersEXT(1, &g_glFramebuffer);
    }
    if (texture != 0) {
        unsigned int fb = g_glFramebuffer;
        g_glewBindFramebufferEXT(GLR_FRAMEBUFFER, fb);
        g_glBoundFramebuffer = (int)fb;
        g_glewFramebufferTexture2DEXT(GLR_FRAMEBUFFER, GLR_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        g_glewFramebufferRenderbufferEXT(GLR_FRAMEBUFFER, GLR_DEPTH_ATTACHMENT, GLR_RENDERBUFFER, depth);
    } else {
        g_glewBindFramebufferEXT(GLR_FRAMEBUFFER, 0);
        g_glBoundFramebuffer = 0;
    }
    status = g_glewCheckFramebufferStatusEXT(GLR_FRAMEBUFFER);
    if (status != GLR_FRAMEBUFFER_COMPLETE) {
        MatchLog_Printf(7, "Framebuffer status %i\n", status);
    }
}

/* ---- shaders ---- */

/*
 * 0x0047C460: a malloc'ed copy of the shader source src with defines
 * replacing its GLR_DEFINES_MARK (src as it is without the mark).
 */
char *Gl_InsertDefines(const char *src, const char *defines)
{
    size_t len_src = strlen(src);
    size_t len_def = strlen(defines);
    size_t len_mark = strlen(GLR_DEFINES_MARK);
    const char *mark = strstr(src, GLR_DEFINES_MARK);
    size_t pos, total;
    char *out;

    if (mark == NULL) {
        out = malloc(len_src + 1);
        if (out == NULL) {
            return NULL;
        }
        strncpy(out, src, len_src);
        out[len_src] = '\0';
        return out;
    }
    pos = (size_t)(mark - src);
    total = len_def - len_mark + len_src;
    out = malloc(total + 1);
    if (out == NULL) {
        return NULL;
    }
    strncpy(out, src, pos);
    strncpy(out + pos, defines, len_def);
    strncpy(out + pos + len_def, src + pos + len_mark, total - pos - len_def);
    out[total] = '\0';
    return out;
}

/* path = "data/" name ext, as the original builds it with strncat. */
static void Glr_ShaderPath(char *path, const char *name, const char *ext)
{
    size_t len;

    path[0] = '\0';
    strncat(path, "data/", MAX_PATH - 1);
    len = strlen(path);
    if (MAX_PATH - len > 1) {
        strncat(path, name, MAX_PATH - 1 - len);
    }
    len = strlen(path);
    if (MAX_PATH - len > 1) {
        strncat(path, ext, MAX_PATH - 1 - len);
    }
}

/* Creates and compiles a shader from src (freed). */
static unsigned int Glr_CompileShader(unsigned int type, char *src)
{
    const char *p = src;
    unsigned int shader = g_glewCreateShader(type);

    g_glewShaderSource(shader, 1, &p, NULL);
    g_glewCompileShader(shader);
    free(src);
    return shader;
}

/*
 * 0x0047C580: loads program prog from data/<name>.vert and .frag, with
 * "#define <d>" for each of the NULL-terminated defines and PREMUL. Returns
 * 0 when a file cannot be read.
 */
int Gl_LoadShader(const char **defines, GlProgram *prog, const char *name)
{
    char defs[GLR_DEFINES_SIZE + 0x110];
    char path[MAX_PATH];
    size_t n = 0;
    FILE *f;
    char *text, *src;

    for (; *defines != NULL; defines++) {
        _snprintf(defs + n, GLR_DEFINES_SIZE - n, "#define %s\n", *defines);
        n += strlen(defs + n);
    }
    _snprintf(defs + n, GLR_DEFINES_SIZE - n, "#define PREMUL\n");

    Glr_ShaderPath(path, name, ".vert");
    f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    {
        long pos = ftell(f);
        long size;
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, pos, SEEK_SET);
        text = calloc(1, size + 1);
        if (text == NULL) {
            fclose(f);
            return 0;
        }
        fread(text, size, 1, f);
        fclose(f);
        text[size] = '\0';
    }
    src = Gl_InsertDefines(text, defs);
    free(text);
    if (src == NULL) {
        return 0;
    }
    prog->vert = Glr_CompileShader(GLR_VERTEX_SHADER, src);
    Gl_LogInfo(prog->vert);

    Glr_ShaderPath(path, name, ".frag");
    f = fopen(path, "rb");
    if (f == NULL) {
        return 0;
    }
    text = Gl_ReadFile(f);
    if (text == NULL) {
        return 0;
    }
    src = Gl_InsertDefines(text, defs);
    free(text);
    if (src == NULL) {
        return 0;
    }
    prog->frag = Glr_CompileShader(GLR_FRAGMENT_SHADER, src);
    Gl_LogInfo(prog->frag);

    prog->program = g_glewCreateProgram();
    g_glewAttachShader(prog->program, prog->vert);
    g_glewAttachShader(prog->program, prog->frag);
    g_glewLinkProgram(prog->program);
    Gl_LogInfo(prog->program);
    prog->texunit0 = g_glewGetUniformLocation(prog->program, "texunit0");
    prog->texunit1 = g_glewGetUniformLocation(prog->program, "texunit1");
    prog->mask = g_glewGetUniformLocation(prog->program, "mask");
    prog->add = g_glewGetUniformLocation(prog->program, "add");
    prog->mul = g_glewGetUniformLocation(prog->program, "mul");
    prog->invertall = g_glewGetUniformLocation(prog->program, "invertall");
    prog->colorbal = g_glewGetUniformLocation(prog->program, "colorbal");
    prog->texsize = g_glewGetUniformLocation(prog->program, "texsize");
    prog->src_alphamul = g_glewGetUniformLocation(prog->program, "src_alphamul");
    return 1;
}

/*
 * 0x0047C990: loads the 9 variants of the "indexed" and "rgba" programs,
 * with the palette lookup mode of g_glIndexMode. Failures are only logged.
 */
int Gl_LoadShaders(void)
{
    const char *mode;
    int i;

    if (g_glIndexMode == 0) {
        mode = "INDEX_STANDARD";
    } else if (g_glIndexMode == 1) {
        mode = "INDEX_GEFORCE7";
    } else {
        mode = "INDEX_SAFE";
    }
    for (i = 0; i < GL_PROGRAM_COUNT; i++) {
        const char **defines = g_glShaderDefines[i];
        defines[0] = mode;
        if (!Gl_LoadShader(defines, &g_glProgramsIndexed[i], "indexed")) {
            MatchLog_Printf(3, "Unabled to load indexed shader\n");
        }
        if (!Gl_LoadShader(defines, &g_glProgramsRgba[i], "rgba")) {
            MatchLog_Printf(3, "Unabled to load RGBA shader\n");
        }
    }
    return 1;
}

/* Deletes the GL objects of a program and clears it. */
static void Glr_FreeProgram(GlProgram *prog)
{
    g_glewDeleteProgram(prog->program);
    g_glewDeleteShader(prog->frag);
    g_glewDeleteShader(prog->vert);
    memset(prog, 0, sizeof(*prog));
}

/* 0x0047CA50: deletes every program. */
int Gl_FreeShaders(void)
{
    int i;

    for (i = 0; i < GL_PROGRAM_COUNT; i++) {
        Glr_FreeProgram(&g_glProgramsIndexed[i]);
    }
    for (i = 0; i < GL_PROGRAM_COUNT; i++) {
        Glr_FreeProgram(&g_glProgramsRgba[i]);
    }
    return 1;
}

/*
 * 0x0047CAF0: writes the info log of a shader or program to the match log.
 * Quirk: the original leaves the log size uninitialised (stack garbage) when
 * obj is neither a shader nor a program; here it is 0 then.
 */
int Gl_LogInfo(unsigned int obj)
{
    int size = 0;
    int len = 0;
    char *log;

    if (g_glewIsShader(obj)) {
        g_glewGetShaderiv(obj, GLR_INFO_LOG_LENGTH, &size);
    } else {
        g_glewGetProgramiv(obj, GLR_INFO_LOG_LENGTH, &size);
    }
    log = malloc(size);
    if (log == NULL) {
        return 0;
    }
    if (g_glewIsShader(obj)) {
        g_glewGetShaderInfoLog(obj, size, &len, log);
    } else {
        g_glewGetProgramInfoLog(obj, size, &len, log);
    }
    if (len > 0) {
        MatchLog_Printf(7, "%s\n", log);
    }
    free(log);
    return 1;
}

/* 0x0047CBA0: the rest of f as a malloc'ed string; closes f. */
char *Gl_ReadFile(FILE *f)
{
    long pos = ftell(f);
    long size;
    char *text;

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, pos, SEEK_SET);
    text = calloc(1, size + 1);
    if (text == NULL) {
        fclose(f);
        return NULL;
    }
    fread(text, size, 1, f);
    fclose(f);
    text[size] = '\0';
    return text;
}
