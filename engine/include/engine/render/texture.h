#ifndef NDS_ENGINE_TEXTURE_H
#define NDS_ENGINE_TEXTURE_H

#include "engine/core/types.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_texture {
    uint32_t width;
    uint32_t height;
    uint8_t* rgba8;
} nds_texture;

nds_result nds_texture_load(const char* path, nds_texture* out_texture);
void nds_texture_destroy(nds_texture* texture);

#ifdef __cplusplus
}
#endif

#endif
