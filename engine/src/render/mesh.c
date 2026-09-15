#include "engine/render/mesh.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NDS_MESH_MAX_VERTICES 65535u
#define NDS_MESH_MAX_INDICES 196605u

static void reset(nds_mesh* m)
{
    memset(m, 0, sizeof(*m));
}

void nds_mesh_destroy(nds_mesh* m)
{
    if (!m) return;
    free(m->vertices);
    free(m->indices);
    reset(m);
}

static int next_line(const char** cursor, char* line, size_t capacity)
{
    const char* p = *cursor;
    size_t n = 0;
    if (!p || !*p) return 0;
    while (*p && *p != '\n' && n + 1 < capacity) line[n++] = *p++;
    line[n] = '\0';
    if (*p == '\n') ++p;
    *cursor = p;
    return 1;
}

nds_result nds_mesh_load_text(const char* text, nds_mesh* out_mesh)
{
    const char* cursor;
    char line[256];
    size_t vertices = 0, indices = 0, vi = 0, ii = 0;
    int section = 0;
    nds_mesh result;

    if (!text || !out_mesh) return NDS_ERR_INVALID_ARG;
    reset(out_mesh);
    reset(&result);
    cursor = text;

    while (next_line(&cursor, line, sizeof(line))) {
        char* p = line;
        while (*p == ' ' || *p == '\t' || *p == '\r') ++p;
        if (*p == '\0' || *p == '#') continue;
        if (strcmp(p, "NDSMESH 1") == 0) continue;
        if (strcmp(p, "vertices") == 0) { section = 1; continue; }
        if (strcmp(p, "indices") == 0) { section = 2; continue; }
        if (section == 0) {
            unsigned long count;
            if (sscanf(p, "vertex_count %lu", &count) == 1) {
                if (count == 0 || count > NDS_MESH_MAX_VERTICES) goto invalid;
                vertices = (size_t)count;
                result.vertices = (nds_mesh_vertex*)calloc(vertices, sizeof(*result.vertices));
                if (!result.vertices) goto oom;
                continue;
            }
            if (sscanf(p, "index_count %lu", &count) == 1) {
                if (count == 0 || count > NDS_MESH_MAX_INDICES || (count % 3) != 0) goto invalid;
                indices = (size_t)count;
                result.indices = (uint16_t*)calloc(indices, sizeof(*result.indices));
                if (!result.indices) goto oom;
                continue;
            }
            goto invalid;
        }
        if (section == 1) {
            if (vi >= vertices || sscanf(p, "%f %f %f", &result.vertices[vi].x,
                                         &result.vertices[vi].y, &result.vertices[vi].z) != 3) goto invalid;
            ++vi;
        } else {
            unsigned long a, b, c;
            if (ii + 2 >= indices || sscanf(p, "%lu %lu %lu", &a, &b, &c) != 3 ||
                a >= vertices || b >= vertices || c >= vertices || a > 65534ul || b > 65534ul || c > 65534ul) goto invalid;
            result.indices[ii++] = (uint16_t)a;
            result.indices[ii++] = (uint16_t)b;
            result.indices[ii++] = (uint16_t)c;
        }
    }

    if (!vertices || !indices || vi != vertices || ii != indices) goto invalid;
    result.vertex_count = vertices;
    result.index_count = indices;
    *out_mesh = result;
    return NDS_OK;

invalid:
    nds_mesh_destroy(&result);
    return NDS_ERR_IO;
oom:
    nds_mesh_destroy(&result);
    return NDS_ERR_UNKNOWN;
}

nds_result nds_mesh_load(const char* path, nds_mesh* out_mesh)
{
    FILE* f;
    long size;
    char* data;
    size_t read_count;
    nds_result result;
    if (!path || !out_mesh) return NDS_ERR_INVALID_ARG;
    f = fopen(path, "rb");
    if (!f) return NDS_ERR_IO;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return NDS_ERR_IO; }
    size = ftell(f);
    if (size < 0) { fclose(f); return NDS_ERR_IO; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return NDS_ERR_IO; }
    data = (char*)malloc((size_t)size + 1);
    if (!data) { fclose(f); return NDS_ERR_UNKNOWN; }
    read_count = fread(data, 1, (size_t)size, f);
    fclose(f);
    if (read_count != (size_t)size) { free(data); return NDS_ERR_IO; }
    data[size] = '\0';
    result = nds_mesh_load_text(data, out_mesh);
    free(data);
    return result;
}
