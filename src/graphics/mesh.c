#include <stdint.h>
#include "graphics/rendering_internal.h"
#include "graphics/mesh.h"



// MESHES

//- FUNCTIONS

void Mesh_init(Mesh_t* mesh,
               const Vertex_t* vertices,
               const uint32_t* indices,
               uint32_t vertexCount,
               uint32_t triangleCount
) {
    *mesh = ((Mesh_t){vertices,
                      indices,
                      0U,
                      vertexCount,
                      triangleCount
    });

    Rendering_onMeshInit(mesh);
}


void Mesh_destr(Mesh_t* mesh) {
    Rendering_onMeshDestr(mesh);
}
