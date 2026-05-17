#ifndef CORAL_GRAPHICS_OBJECT
#define CORAL_GRAPHICS_OBJECT

/*! \file */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../common/api.h"
#include "../containers/linkedList.h"
#include "./math.h"
#include "./mesh.h"



// OBJECTS

/*! \defgroup CORAL_GRAPHICS_OBJECT Objects
 *  \ingroup CORAL_GRAPHICS
 *  \brief Types representing various objects and functions operating on those objects.
 *  \addtogroup CORAL_GRAPHICS_OBJECT
 *  @{
 */



//- DEFINITIONS

/*! \brief Used to determine the type of an object. */
typedef uint16_t ObjectType_t;


/*! \brief Used to determine the type of a component. */
typedef uint16_t ComponentType_t;


/*! \brief A group of components which can be applied to an object. */
typedef struct ComponentClass ComponentClass_t;



//- - OBJECT FLAGS

/*! \defgroup CORAL_GRAPHICS_OBJECT_FLAGS Object flags
 *  \addtogroup CORAL_GRAPHICS_OBJECT_FLAGS
 *  @{
 */

#define CORAL_GRAPHICS_OBJECT_HAS_FLAGS(OBJECT_FLAGS, CHECK_FLAGS) (((OBJECT_FLAGS) & (CHECK_FLAGS)) == (CHECK_FLAGS))



//- - - USER FLAGS

/*! \brief Marks the range of flags which may be set when calling \p Object_init() or \p Object__init(). */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_RANGE                       (0x00FFFFFFU)


/*! \brief Static objects may not be moved, scaled or rotated after they have been added to a scene.
 *  \note
 *      Applies only to objects of type or composed from the type \p Base3DObject_t,
 *      i.e. of type or composed from the type \p CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D.
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC                      (0x00000010U)


/*! \brief The position, rotation, and scale of an object is to be interpreted as absolute and not relative to its parent object.
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC
 *  \note Applies only to objects within a scene.
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION     (0x00000020U)


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_XYZ
 *  \note If no rotation order is set, \p CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_XYZ is used.
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_XYZ    CORAL_GRAPHICS_ROTATION_ORDER_EULER_XYZ


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_XZY
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_XZY    CORAL_GRAPHICS_ROTATION_ORDER_EULER_XZY


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_YXZ
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_YXZ    CORAL_GRAPHICS_ROTATION_ORDER_EULER_YXZ


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_YZX
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_YZX    CORAL_GRAPHICS_ROTATION_ORDER_EULER_YZX


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZXY
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_ZXY    CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZXY


/*! \copybrief CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZYX
 *  \copydetails CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION
 */
#define CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_ZYX    CORAL_GRAPHICS_ROTATION_ORDER_EULER_ZYX



//- - - RESERVED FLAGS

/*! \brief Marks the range of flags which may only be set internally and not when calling \p Object_init() or \p Object__init(). */
#define CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_RANGE                   (0x0F000000U)


/*! @{ */

/*! \note May not be specified in a call to \p Object_init() or \p Object__init(). */

#define CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_DYNAMICALLY_ALLOCATED   (0x01000000U)

/*! @} */



//- - - RENDERING FLAGS

/*! \brief
 *      Marks the range of flags which may only be set by the rendering implementation and
 *      not when calling \p Object_init() or \p Object__init().
 */
#define CORAL_GRAPHICS_OBJECT_RENDERING_FLAG_RANGE                  (0xF0000000U)

/*! @} */



//- - OBJECT TYPES

/*! \defgroup CORAL_GRAPHICS_OBJECT_TYPES Object types
 *  \addtogroup CORAL_GRAPHICS_OBJECT_TYPES
 *  @{
 */

typedef enum {
    CORAL_GRAPHICS_OBJECT_TYPE_NONE = 0,            /*!< \brief Indicates an object of invalid type. */
    CORAL_GRAPHICS_OBJECT_TYPE_BASE,                /*!< \brief Indicates an object of type \p BaseObject_t. */
    CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D,             /*!< \brief Indicates an object of type \p Base3DObject_t. */
    CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA,  /*!< \brief Indicates an object of type \p PerspectiveCameraObject_t. */
    CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA, /*!< \brief Indicates an object of type \p OrthographicCameraObject_t. */
    CORAL_GRAPHICS_OBJECT_TYPE_SPHERE,              /*!< \brief Indicates an object of type \p SphereObject_t. */
    CORAL_GRAPHICS_OBJECT_TYPE_MESH                 /*!< \brief Indicates an object of type \p MeshObject_t. */
} ObjectType_e;



//- - - BASE

typedef struct {
    ComponentClass_t* componentClass;
    uint32_t flags;
    ObjectType_t type;

    /*! \brief The count of bytes from the start of an object's \p BaseObject_t member to the beginning of its components.
     *  \note May be \c 0U if the object does not have any components.
     */
    uint16_t offsetToComponents;
} BaseObject_t;



//- - - BASE 3D

typedef struct {
    BaseObject_t base;
    Vec3_t position;
    Vec3_t rotation;
    Vec3_t scale;
} Base3DObject_t;



//- - - PERSPECTIVE CAMERA

typedef struct {
    Base3DObject_t base3D;
    Float_t aspectRatio;
    Float_t horizontalFOV;
    Float_t distanceToNearPlane;
    Float_t distanceToFarPlane;
} PerspectiveCameraObject_t;



//- - - ORTHOGRAPHIC CAMERA

typedef struct {
    Base3DObject_t base3D;
    Float_t width;
    Float_t height;
    Float_t depth;
} OrthographicCameraObject_t;



//- - - SPHERE

typedef struct {
    Base3DObject_t base3D;
    Float_t radius;
} SphereObject_t;



//- - - MESH

typedef struct {
    Base3DObject_t base3D;
    const Mesh_t* mesh;
} MeshObject_t;

/*! @} */



//- - COMPONENT TYPES

/*! \defgroup CORAL_GRAPHICS_COMPONENT_TYPES Component types
 *  \addtogroup CORAL_GRAPHICS_COMPONENT_TYPES
 *  @{
 */

typedef enum {
    CORAL_GRAPHICS_COMPONENT_TYPE_NONE,         /*!< \brief Indicates a component of invalid type. */
    CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING,    /*!< \brief Indicates a component of type \p RenderingComponent_ot. */
    CORAL_GRAPHICS_COMPONENT_TYPE_POINTER,      /*!< \brief Indicates a component of type \p PointerComponent_t. */
    CORAL_GRAPHICS_COMPONENT_TYPE_SCENE,        /*!< \brief Indicates a component of type \p SceneComponent_t. */
    CORAL_GRAPHICS_COMPONENT_TYPE_MATERIAL      /*!< \brief Indicates a component of type \p MaterialComponent_t. */
} ComponentType_e;



//- - - RENDERING

/*! \note The renderer may require an object to include a \p RenderingComponent_ot for the object to be renderable.
 *  \note The \p RenderingComponent_ot is defined by the renderer.
 */
typedef struct RenderingComponent RenderingComponent_ot;



//- - - POINTER

typedef struct {
    void* ptr;
} PointerComponent_t;



//- - - SCENE

typedef struct SceneComponent {
    /*! \brief Used in order to keep track of the objects in the scene. */
    LinkedList_SingleLink_t link;
    struct SceneComponent* parentObject;

    /*! \brief List of all child objects.
     *
     *  Similiarly to \mlink{Scene_t,rootObjects}, the individual links are the \p link members
     *  of the \p SceneComponent_t components of the child objects.
     */
    LinkedList_t childObjects;
    uint32_t childObjectCount;
    uint32_t offsetToBaseObject;
} SceneComponent_t;



//- - - MATERIAL

typedef struct {
    /*! \brief
     *      The <tt>r, g, b</tt> members store the diffuse colour and the \p a member stores the opacity of the
     *      diffuse colour, with \c 255U being completely opaque and \c 0U being completely transparent.
     */
    Vec4b_u diffuseColour;
    Vec3b_u specularColour;
    Vec3b_u emissionColour;

    /*! \brief Determines the intensity of specular reflections, with \c 255U being most intense. */
    uint8_t specularIntensity;

    /*! \brief Determines the roughness of the material, with \c 255U being very rough and \c 0U being very shiny. */
    uint8_t roughness;

    /*! \brief May be used to determine refractions if \p diffuseColour is not completely opaque.
     *  \note
     *      Even if \p refractiveIndex is not used by the renderer, if \p diffuseColour is not completely opaque,
     *      \p refractiveIndex must be greater than \c 0.0.
     */
    Float_t refractiveIndex;

    /*! \brief Multiplied with the <tt>r, g, b</tt> members of \p emitionColour to enable emitions of higher strength.
     *  \note Must be greater than or equal to \c 0.0.
     */
    Float_t emissionStrength;
} MaterialComponent_t;

/*! @} */



//- - COMPONENT CLASS

typedef struct {
    ComponentType_t type;

    /*! \brief Count of bytes from the start of the component to the start of an object's components. */
    uint16_t offsetFromComponents;
} ComponentClassElement_t;


struct ComponentClass {
    struct ComponentClass* parentClass;
    ComponentClassElement_t* components;
    uint16_t componentCount;        /*!< \brief Count of components in \p components. */
    uint16_t parentComponentCount;  /*!< \brief Count of components of \p parentClass, including components of its parents. */
    uint16_t componentAlignment;    /*!< \brief Alignment of all components, including the parents' components. */
    uint16_t totalComponentSize;    /*!< \brief Aligned size of all components, including the parents' components. */
};



//- FUNCTIONS

//- - OBJECT

/*! \brief Initializes an object of the type described by \p objectType.
 *  \param[in]  componentClass  Class of the object to be created; may be \c NULL.
 *  \param[in]  flags           Special attributes of the object to be created.
 *  \returns A pointer to the created object.
 */
CORAL_API BaseObject_t* Object_init(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags);


/*! \copybrief Object_init
 *  \param[in]  componentClass          Class of the object to be created; may be \c NULL.
 *  \param[in]  flags                   Special attributes of the object to be created.
 *  \param[in]  objectBuffer            Will hold the created object and must be of sufficient size to do so.
 *  \remarks
 *      To check whether \p objectBuffer is of sufficient size, call
 *      \p Object_getSizeRequirement() with the same paremeters.
 */
CORAL_API void Object__init(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags, void* objectBuffer);


/*! \brief Destructs \p object. */
CORAL_API void Object_destr(BaseObject_t* object);


/*! \brief Retrieves the first component of \p object of type \p componentType.
 *  \param[in,out]  componentIndex  (Opt.) If provided, will retrieve the first component of type \p componentType
 *                                  after and including the index pointed to by \p componentIndex and will be set
 *                                  to the index of the returned component if one is found. Note that index refers
 *                                  to the index into all components of \p object.
 *
 *  \returns A pointer to the component of type \p componentType or \c NULL if \p object does not have a component of that type.
 */
CORAL_API void* Object_getComponent(const BaseObject_t* object, ComponentType_t componentType, size_t* componentIndex);


/*! \returns The size required to hold an object of type \p objectType that inherits components from \p componentClass. */
CORAL_API size_t Object_getSizeRequirement(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags);


/*! \brief
 *      Sets the active material of \p object to \p materialComponent such that it will
 *      be drawn with the parameters of \p materialComponent.
 *
 *  \param[in]  materialComponent   May not be a component of \p object; if \c NULL, will remove the currently active
 *                                  material of \p object and revert it to the renderer's default behaviour.
 *
 *  \returns
 *      \c true if the operation was successful, otherwise \c false, which may occur if the renderer determines
 *      \p object to not be renderable.
 *
 *  \note
 *      If the operation is successful and \p object is within a scene, \p Scene_changedObjectAttributes()
 *      or \p Scene__changedObjectAttributes() must be called.
 */
CORAL_API bool Object_setActiveMaterial(BaseObject_t* object, const MaterialComponent_t* materialComponent);



//- - COMPONENT CLASS

/*! \brief Initializes \p componentClass.
 *  \param[in]  parentClass     Must have already been initialized.
 *  \param[in]  components      The \p type member of each struct must have been set and the array must
 *                              remain in scope for as long as \p componentClass will be used.
 */
CORAL_API void ComponentClass_init(ComponentClass_t* componentClass,
                                   ComponentClass_t* parentClass,
                                   ComponentClassElement_t components[],
                                   size_t componentCount
);


/*! \brief
 *      Retrieves the first \p ComponentClassElement_t of \p componentClass which describes
 *      a component of type \p componentType.
 *
 *  \param[in,out]  componentIndex  (Opt.) If provided, will retrieve the first \p ComponentClassElement_t
 *                                  describing a component of type \p componentType after and including the
 *                                  index pointed to by \p componentIndex and will be set to the index of
 *                                  the returned \p ComponentClassElement_t if one is found. Note that index
 *                                  refers to the index into all components described by \p componentClass.
 *
 *  \returns
 *      A pointer to the \p ComponentClassElement_t describing a component of type \p componentType
 *      or \c NULL if \p componentClass does not have such a \p ComponentClassElement_t.
 */
CORAL_API ComponentClassElement_t* ComponentClass_getComponent(const ComponentClass_t* componentClass,
                                                               ComponentType_t componentType,
                                                               size_t* componentIndex
);

/*! @} */

#endif
