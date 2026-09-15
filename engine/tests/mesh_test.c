#include "engine/render/mesh.h"

#include <assert.h>

int main(void)
{
    const char* text =
        "NDSMESH 1\n"
        "vertex_count 3\n"
        "index_count 3\n"
        "vertices\n"
        "0 0 0 0 0\n"
        "1 0 0 1 0\n"
        "0 1 0 0 1\n"
        "indices\n"
        "0 1 2\n";
    nds_mesh mesh;
    assert(nds_mesh_load_text(text, &mesh) == NDS_OK);
    assert(mesh.vertex_count == 3);
    assert(mesh.index_count == 3);
    assert(mesh.indices[0] == 0 && mesh.indices[1] == 1 && mesh.indices[2] == 2);
    assert(mesh.vertices[0].u == 0.0f && mesh.vertices[0].v == 0.0f);
    assert(mesh.vertices[1].u == 1.0f && mesh.vertices[1].v == 0.0f);
    assert(mesh.vertices[2].u == 0.0f && mesh.vertices[2].v == 1.0f);
    nds_mesh_destroy(&mesh);

    assert(nds_mesh_load_text(
        "NDSMESH 1\nvertex_count 3\nindex_count 3\nvertices\n"
        "0 0 0\n1 0 0\n0 1 0\nindices\n0 1 2\n", &mesh) == NDS_OK);
    assert(mesh.vertices[0].u == 0.0f && mesh.vertices[0].v == 0.0f);
    assert(mesh.vertices[1].u == 0.0f && mesh.vertices[1].v == 0.0f);
    assert(mesh.vertices[2].u == 0.0f && mesh.vertices[2].v == 0.0f);
    nds_mesh_destroy(&mesh);

    assert(nds_mesh_load_text(
        "NDSMESH 1\nvertex_count 3\nindex_count 3\nvertices\n"
        "0 0 0 0 0\n1 0 0 1\n0 1 0 0 1\nindices\n0 1 2\n", &mesh) != NDS_OK);
    assert(nds_mesh_load_text(
        "NDSMESH 1\nvertex_count 3\nindex_count 3\nvertices\n"
        "0 0 0\n1 0 0\n0 1 0\nindices\n0 1 9\n", &mesh) != NDS_OK);
    return 0;
}
