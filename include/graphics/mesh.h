#ifndef CORAL_GRAPHICS_MESH
#define CORAL_GRAPHICS_MESH

/*! \file */

#include <stdint.h>
#include "../common/api.h"
#include "./math.h"



// MESHES

/*! \defgroup CORAL_GRAPHICS_MESH Mesh_t
 *  \ingroup CORAL_GRAPHICS
 *  \brief Types representing vertices and vertex meshes.
 *  \addtogroup CORAL_GRAPHICS_MESH
 *  @{
 */



//- DEFINITIONS

//- - VERTEX

typedef struct {
    Vec3_t position;
    Vec3_t normal;
} Vertex_t;



//- <

/*! \brief Represents a mesh of triangles.
 *  \note The vertices of triangles are ordered counter-clockwise.
 *  \note The normal of the first vertex of a triangle will be used as the normal of that triangle.
 */
typedef struct {
    const Vertex_t* vertices;
    const uint32_t* indices;
    uintptr_t meshDrawingResource;
    uint32_t vertexCount;
    uint32_t triangleCount;
} Mesh_t;



//- FUNCTIONS

/*! \brief Initializes \p mesh. */
CORAL_API void Mesh_init(Mesh_t* mesh,
                         const Vertex_t* vertices,
                         const uint32_t* indices,
                         uint32_t vertexCount,
                         uint32_t triangleCount
);


/*! \brief Destructs \p mesh. */
CORAL_API void Mesh_destr(Mesh_t* mesh);

/*! @} */

#endif
