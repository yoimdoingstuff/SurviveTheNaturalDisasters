#include "engine/render/texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* N DSTEX 1
 * width <n>
 * height <n>
 * rgba8
 * <width*height lines of four decimal bytes>
 */
static int read_token(FILE* f, char* out, size_t capacity)
{
    int c; size_t n = 0;
    if (!f || !out || capacity < 2) return 0;
    do { c = fgetc(f); } while (c != EOF && (c == ' ' || c == '\t' || c == '\r' || c == '\n'));
    if (c == EOF) return 0;
    while (c != EOF && c != ' ' && c != '\t' && c != '\r' && c != '\n') {
        if (n + 1 >= capacity) return 0;
        out[n++] = (char)c;
        c = fgetc(f);
    }
    out[n] = '\0';
    return 1;
}

nds_result nds_texture_load(const char* path, nds_texture* out_texture)
{
    FILE* f = NULL; char token[32]; uint32_t width, height; size_t pixels, bytes, i;
    nds_texture texture = {0};
    if (!path || !out_texture) return NDS_ERR_INVALID_ARG;
    *out_texture = (nds_texture){0};
    f = fopen(path, "rb");
    if (!f) return NDS_ERR_NOT_FOUND;
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "NDSTEX") != 0) goto invalid;
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "1") != 0) goto invalid;
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "width") != 0) goto invalid;
    if (!read_token(f, token, sizeof(token))) goto invalid;
    width = (uint32_t)strtoul(token, NULL, 10);
    if (!width || width > 4096) goto invalid;
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "height") != 0) goto invalid;
    if (!read_token(f, token, sizeof(token))) goto invalid;
    height = (uint32_t)strtoul(token, NULL, 10);
    if (!height || height > 4096) goto invalid;
    if (!read_token(f, token, sizeof(token)) || strcmp(token, "rgba8") != 0) goto invalid;
    if (width > SIZE_MAX / height) goto invalid;
    pixels = (size_t)width * height;
    if (pixels > SIZE_MAX / 4) goto invalid;
    bytes = pixels * 4;
    texture.rgba8 = (uint8_t*)malloc(bytes);
    if (!texture.rgba8) { fclose(f); return NDS_ERR_UNKNOWN; }
    texture.width = width; texture.height = height;
    for (i = 0; i < bytes; ++i) {
        if (!read_token(f, token, sizeof(token))) { nds_texture_destroy(&texture); goto invalid_closed; }
        {
            char* end = NULL; unsigned long value = strtoul(token, &end, 10);
            if (!end || *end || value > 255) { nds_texture_destroy(&texture); goto invalid_closed; }
            texture.rgba8[i] = (uint8_t)value;
        }
    }
    fclose(f); *out_texture = texture; return NDS_OK;
invalid:
    fclose(f);
invalid_closed:
    return NDS_ERR_INVALID_ARG;
}

void nds_texture_destroy(nds_texture* texture)
{
    if (!texture) return;
    free(texture->rgba8);
    *texture = (nds_texture){0};
}
