#ifndef NDS_ENGINE_MESH_H
#define NDS_ENGINE_MESH_H

#include "engine/core/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nds_mesh_vertex {
    float x, y, z;
} nds_mesh_vertex;

typedef struct nds_mesh {
    nds_mesh_vertex* vertices;
    uint16_t* indices;
    size_t vertex_count;
    size_t index_count;
} nds_mesh;

/* Small, deterministic text format intended as an intermediate asset format.
 * It is deliberately limited to 16-bit indices for GLES2-era hardware. */
nds_result nds_mesh_load(const char* path, nds_mesh* out_mesh);
nds_result nds_mesh_load_text(const char* text, nds_mesh* out_mesh);
void nds_mesh_destroy(nds_mesh* mesh);

#ifdef __cplusplus
}
#endif

#endif
