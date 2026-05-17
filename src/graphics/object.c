#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include "common/utils.h"
#include "memory/dynalloc.h"
#include "graphics/rendering_internal.h"
#include "graphics/object.h"
#include "graphics/object_internal.h"



// OBJECTS

//- FUNCTIONS

//- - OBJECT

static size_t Object_getBaseObjectMemberOffset(ObjectType_t objectType) {
    switch (objectType) {
        default:
            return 0U;

        case CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D:
            return offsetof(Base3DObject_t, base);

        case CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA:
            return offsetof(PerspectiveCameraObject_t, base3D.base);

        case CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA:
            return offsetof(OrthographicCameraObject_t, base3D.base);

        case CORAL_GRAPHICS_OBJECT_TYPE_SPHERE:
            return offsetof(SphereObject_t, base3D.base);

        case CORAL_GRAPHICS_OBJECT_TYPE_MESH:
            return offsetof(MeshObject_t, base3D.base);
    }
}

static SizeAndAlignment_t Object_getObjectSizeAndAlignment(ObjectType_t objectType) {
    switch (objectType) {
        case CORAL_GRAPHICS_OBJECT_TYPE_BASE:
            return ((SizeAndAlignment_t){sizeof(BaseObject_t), alignof(BaseObject_t)});

        case CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D:
            return ((SizeAndAlignment_t){sizeof(Base3DObject_t), alignof(Base3DObject_t)});

        case CORAL_GRAPHICS_OBJECT_TYPE_PERSPECTIVE_CAMERA:
            return ((SizeAndAlignment_t){sizeof(PerspectiveCameraObject_t), alignof(PerspectiveCameraObject_t)});

        case CORAL_GRAPHICS_OBJECT_TYPE_ORTHOGRAPHIC_CAMERA:
            return ((SizeAndAlignment_t){sizeof(OrthographicCameraObject_t), alignof(OrthographicCameraObject_t)});

        case CORAL_GRAPHICS_OBJECT_TYPE_SPHERE:
            return ((SizeAndAlignment_t){sizeof(SphereObject_t), alignof(SphereObject_t)});

        case CORAL_GRAPHICS_OBJECT_TYPE_MESH:
            return ((SizeAndAlignment_t){sizeof(MeshObject_t), alignof(MeshObject_t)});
    }

    return ((SizeAndAlignment_t){0U, 1U});
}


static SizeAndAlignment_t Object_getComponentSizeAndAlignment(ComponentType_t componentType) {
    switch (componentType) {
        case CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING:
            return Rendering_getRenderingComponentSizeAndAlignment();

        case CORAL_GRAPHICS_COMPONENT_TYPE_POINTER:
            return ((SizeAndAlignment_t){sizeof(PointerComponent_t), alignof(PointerComponent_t)});

        case CORAL_GRAPHICS_COMPONENT_TYPE_SCENE:
            return ((SizeAndAlignment_t){sizeof(SceneComponent_t), alignof(SceneComponent_t)});

        case CORAL_GRAPHICS_COMPONENT_TYPE_MATERIAL:
            return ((SizeAndAlignment_t){sizeof(MaterialComponent_t), alignof(MaterialComponent_t)});
    }

    return ((SizeAndAlignment_t){0U, 1U});
}


static void Object_initComponent(ComponentType_t componentType, void* component, size_t size) {
    switch (componentType) {
        case CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING:
            CORAL_ASSERT(0, "A RenderingComponent_ot may not be initialized with Object_initComponent, use Rendering_onObjectInit.");

        default:
            memset(component, 0, size);
            return;
    }
}


static void Object_destrComponent(ComponentType_t componentType, void* component, size_t size) {
    switch (componentType) {
        case CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING:
            CORAL_ASSERT(0, "A RenderingComponent_ot may not be destructed with Object_destrComponent, use Rendering_onObjectDestr.");

        default:
            return;
    }
}


static void Object_init_base(ObjectType_t objectType,
                             ComponentClass_t* componentClass,
                             uint32_t flags,
                             void* objectBuffer,
                             SizeAndAlignment_t* objectSizeAndAlignment
) {
    uint16_t offsetToComponents = 0U;

    if (componentClass) {
        offsetToComponents = CORAL_TO_ALIGNMENT(objectSizeAndAlignment->size, componentClass->componentAlignment);
        CORAL_ASSERT(!((uintptr_t)objectBuffer % componentClass->componentAlignment),
                     "Provided objectBuffer does not satisfy the components' alignment."
        );
    }

    CORAL_ASSERT(!((uintptr_t)objectBuffer % objectSizeAndAlignment->alignment),
                 "Provided objectBuffer does not satisfy the objectType's alignment."
    );

    memset(objectBuffer, 0, objectSizeAndAlignment->size);
    BaseObject_t* baseObject = (void*)((uintptr_t)objectBuffer + Object_getBaseObjectMemberOffset(objectType));
    *baseObject = ((BaseObject_t){componentClass, flags, objectType, offsetToComponents});

    RenderingComponent_ot* renderingComponent = NULL;
    uintptr_t componentsBegin = (uintptr_t)objectBuffer + offsetToComponents;
    uint16_t previousComponentOffset = componentClass->totalComponentSize;

    while (componentClass) {
        for (size_t i = componentClass->componentCount - 1U; i != SIZE_MAX; i--) {
            ComponentClassElement_t* currentComponentElement = &componentClass->components[i];
            uint16_t currentComponentOffset = currentComponentElement->offsetFromComponents;
            void* currentComponent = (void*)(componentsBegin + currentComponentOffset);

            // NOTE: Additional RenderingComponent_ots are skipped and never initialized. May become a problem.
            if (currentComponentElement->type == CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING) { renderingComponent = currentComponent; }
            else { Object_initComponent(currentComponentElement->type, currentComponent, previousComponentOffset - currentComponentOffset); }

            previousComponentOffset = currentComponentOffset;
        }

        componentClass = componentClass->parentClass;
    }

    if (renderingComponent) { Rendering_onObjectInit(objectBuffer, renderingComponent); }
}


/*! \param[out] objectSizeAndAlignment  (Opt.) If provided, will be set to the size and alignment of \p objectType. */
static size_t Object_getSizeRequirement_base(ObjectType_t objectType,
                                             ComponentClass_t* componentClass,
                                             uint32_t flags,
                                             SizeAndAlignment_t* objectSizeAndAlignment
) {
    SizeAndAlignment_t sizeAndAlignment = Object_getObjectSizeAndAlignment(objectType);
    size_t sizeRequirement = sizeAndAlignment.size;

    if (componentClass) {
        size_t maxAlignment = CORAL_MAX(sizeAndAlignment.alignment, componentClass->componentAlignment);
        sizeRequirement = CORAL_TO_ALIGNMENT(sizeRequirement, componentClass->componentAlignment) + componentClass->totalComponentSize;
        sizeRequirement = CORAL_TO_ALIGNMENT(sizeRequirement, maxAlignment);
    }

    if (objectSizeAndAlignment) { *objectSizeAndAlignment = sizeAndAlignment; }

    return sizeRequirement;
}

BaseObject_t* Object_init(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags) {
    CORAL_ASSERT(!(flags & CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_RANGE), "Specified a reserved flag to initialize an object.");

    flags |= CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_DYNAMICALLY_ALLOCATED;

    SizeAndAlignment_t objectSizeAndAlignment;
    size_t sizeRequirement = Object_getSizeRequirement_base(objectType, componentClass, flags, &objectSizeAndAlignment);

    void* objectBuffer = CORAL_malloc(sizeRequirement);
    CORAL_ASSERT(objectBuffer, "Failed to allocate memory for an object.");

    Object_init_base(objectType, componentClass, flags, objectBuffer, &objectSizeAndAlignment);
    return objectBuffer;
}


void Object__init(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags, void* objectBuffer) {
    CORAL_ASSERT(!(flags & CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_RANGE), "Specified a reserved flag to initialize an object.");

    SizeAndAlignment_t objectSizeAndAlignment = Object_getObjectSizeAndAlignment(objectType);
    Object_init_base(objectType, componentClass, flags, objectBuffer, &objectSizeAndAlignment);
}


void Object_destr(BaseObject_t* object) {
    ComponentClass_t* componentClass = object->componentClass;
    RenderingComponent_ot* renderingComponent = NULL;
    uintptr_t componentsBegin = (uintptr_t)object + object->offsetToComponents;
    uint16_t previousComponentOffset = componentClass->totalComponentSize;

    while (componentClass) {
        for (size_t i = componentClass->componentCount - 1U; i != SIZE_MAX; i--) {
            ComponentClassElement_t* currentComponentElement = &componentClass->components[i];
            uint16_t currentComponentOffset = currentComponentElement->offsetFromComponents;
            void* currentComponent = (void*)(componentsBegin + currentComponentElement->offsetFromComponents);

            // NOTE: Same as initialization, additional RenderingComponent_ots are skipped
            if (currentComponentElement->type == CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING) { renderingComponent = currentComponent; }
            else { Object_destrComponent(currentComponentElement->type, currentComponent, previousComponentOffset - currentComponentOffset); }
        }

        componentClass = componentClass->parentClass;
    }

    if (renderingComponent) { Rendering_onObjectDestr(object, renderingComponent);}

    if (CORAL_GRAPHICS_OBJECT_HAS_FLAGS(object->flags, CORAL_GRAPHICS_OBJECT_RESERVED_FLAG_DYNAMICALLY_ALLOCATED)) {
        CORAL_free(object);
    }
}


void* Object_getComponent(const BaseObject_t* object, ComponentType_t componentType, size_t* componentIndex) {
    ComponentClassElement_t* foundComponent = ComponentClass_getComponent(object->componentClass, componentType, componentIndex);

    if (!foundComponent) { return NULL; }

    uintptr_t component = (uintptr_t)object + object->offsetToComponents + foundComponent->offsetFromComponents;
    return (void*)component;
}


size_t Object_getSizeRequirement(ObjectType_t objectType, ComponentClass_t* componentClass, uint32_t flags) {
    return Object_getSizeRequirement_base(objectType, componentClass, flags, NULL);
}


bool Object_setActiveMaterial(BaseObject_t* object, const MaterialComponent_t* materialComponent) {
    RenderingComponent_ot* renderingComponent = Object_getComponent(object,
                                                                    CORAL_GRAPHICS_COMPONENT_TYPE_RENDERING,
                                                                    NULL
    );

    return Rendering_onObjectSetActiveMaterial(object,
                                               renderingComponent,
                                               materialComponent
    );
}



//- - COMPONENT CLASS

void ComponentClass_init(ComponentClass_t* componentClass,
                         ComponentClass_t* parentClass,
                         ComponentClassElement_t components[],
                         size_t componentCount
) {
    CORAL_ASSERT(components, "Tried to initialize a ComponentClass_t with no components.");

    uint16_t parentComponentCount;
    uint16_t currentComponentOffset;
    uint16_t componentAlignment;

    if (parentClass) {
        ComponentClassElement_t* lastParentComponent = &parentClass->components[parentClass->componentCount - 1U];
        SizeAndAlignment_t lastParentComponentSizeAndAlignment = Object_getComponentSizeAndAlignment(lastParentComponent->type);

        parentComponentCount = parentClass->parentComponentCount + parentClass->componentCount;
        currentComponentOffset = lastParentComponent->offsetFromComponents + lastParentComponentSizeAndAlignment.size;
        componentAlignment = parentClass->componentAlignment;
    }
    else {
        parentComponentCount = 0U;
        currentComponentOffset = 0U;
        componentAlignment = 1U;
    }

    for (size_t i = 0U; i < componentCount; i++) {
        SizeAndAlignment_t currentComponentSizeAndAlignment = Object_getComponentSizeAndAlignment(components[i].type);

        if (currentComponentSizeAndAlignment.alignment > componentAlignment) {
            componentAlignment = currentComponentSizeAndAlignment.alignment;
        }

        currentComponentOffset = CORAL_TO_ALIGNMENT(currentComponentOffset, currentComponentSizeAndAlignment.alignment);
        components[i].offsetFromComponents = currentComponentOffset;
        currentComponentOffset += currentComponentSizeAndAlignment.size;
    }

    size_t totalComponentSize = CORAL_TO_ALIGNMENT(currentComponentOffset, componentAlignment);

    *componentClass = ((ComponentClass_t){parentClass, components, componentCount, parentComponentCount, componentAlignment, totalComponentSize});
}


ComponentClassElement_t* ComponentClass_getComponent(const ComponentClass_t* componentClass,
                                                     ComponentType_t componentType,
                                                     size_t* componentIndex
) {
    const ComponentClass_t* classHierarchy[32U] = {componentClass};
    uint8_t currentClassIndex = 0U;

    size_t startIndex = (componentIndex) ? (*componentIndex) : (0U);

    while (startIndex < classHierarchy[currentClassIndex]->parentComponentCount) {
        // Recursion case should be rare, because caching ptrs in classHierarchy protects most calls from any recursion
        // Only a call with a componentClass with an extremely long inheritance chain would experience this case,
        // and then most likely only a single time
        if (currentClassIndex == CORAL_ARRAY_LENGTH(classHierarchy) - 1U) {
            ComponentClassElement_t* foundComponent = ComponentClass_getComponent(classHierarchy[currentClassIndex]->parentClass,
                                                                                  componentType,
                                                                                  componentIndex
            );

            if (foundComponent) { return foundComponent; }

            startIndex = classHierarchy[currentClassIndex]->parentComponentCount;
            break;
        }
        else {
            classHierarchy[currentClassIndex + 1U] = classHierarchy[currentClassIndex]->parentClass;
            currentClassIndex++;
        }
    }

    size_t localStartIndex = startIndex - classHierarchy[currentClassIndex]->parentComponentCount;

    while (true) {
        const ComponentClass_t* currentComponentClass = classHierarchy[currentClassIndex];

        for (size_t i = localStartIndex; i < currentComponentClass->componentCount; i++) {
            if (currentComponentClass->components[i].type == componentType) {
                if (componentIndex) { *componentIndex = currentComponentClass->parentComponentCount + i; }
                return &currentComponentClass->components[i];
            }
        }

        if (!currentClassIndex) { return NULL; }
        currentClassIndex--;
        localStartIndex = 0U;
    }
}
