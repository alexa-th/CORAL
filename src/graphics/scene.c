#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "common/utils.h"
#include "graphics/math.h"
#include "graphics/object.h"
#include "graphics/rendering_internal.h"
#include "graphics/scene.h"



// SCENE

//- FUNCTIONS

void Scene_init(Scene_t* scene) {
    *scene = ((Scene_t){{NULL, NULL}, 0U, 0U, 0U});

    Rendering_onSceneInit(scene);
}


void Scene_destr(Scene_t* scene) {
    Rendering_onSceneDestr(scene);

    while (scene->rootObjectCount) {
        LinkedList_SingleLink_t* currentObjectLink = scene->rootObjects.headLink.singleLink;
        SceneComponent_t* currentObjectSceneComponent = CORAL_TO_PARENT_PTR(SceneComponent_t, link, currentObjectLink);
        Base3DObject_t* currentObject = Scene__getObject(currentObjectSceneComponent);

        Scene__removeObject(scene, currentObject, currentObjectSceneComponent);
    }
}


void Scene_addObject(Scene_t* scene, Base3DObject_t* restrict object, Base3DObject_t* restrict parentObject) {
    SceneComponent_t* objectSceneComponent;
    SceneComponent_t* parentObjectSceneComponent;

    objectSceneComponent = Object_getComponent(&object->base, CORAL_GRAPHICS_COMPONENT_TYPE_SCENE, NULL);
    CORAL_ASSERT(objectSceneComponent, "Tried to add an object with no SceneComponent_t to a scene.");

    if (parentObject) {
        parentObjectSceneComponent = Object_getComponent(&parentObject->base, CORAL_GRAPHICS_COMPONENT_TYPE_SCENE, NULL);
        CORAL_ASSERT(parentObjectSceneComponent, "Tried to add an object with an invalid parentObject.");
    }
    else {
        parentObjectSceneComponent = NULL;
    }

    Scene__addObject(scene, object, parentObject, objectSceneComponent, parentObjectSceneComponent);
}


void Scene__addObject(Scene_t* scene,
                      Base3DObject_t* restrict object,
                      Base3DObject_t* restrict parentObject,
                      SceneComponent_t* restrict objectSceneComponent,
                      SceneComponent_t* restrict parentObjectSceneComponent
) {
    objectSceneComponent->offsetToBaseObject = (uintptr_t)objectSceneComponent - (uintptr_t)(&object->base);

    size_t previousTotalObjectCount = scene->totalObjectCount++;
    CORAL_ASSERT(previousTotalObjectCount != SIZE_MAX, "Exceeded total object count.");

    if (parentObject) {
        uint32_t previousChildObjectCount = parentObjectSceneComponent->childObjectCount++;
        CORAL_ASSERT(previousChildObjectCount != UINT32_MAX, "Exceeded child object count.");

        objectSceneComponent->parentObject = parentObjectSceneComponent;
        LinkedList_append_single(&parentObjectSceneComponent->childObjects, &objectSceneComponent->link);
    }
    else {
        uint32_t previousRootObjectCount = scene->rootObjectCount++;
        CORAL_ASSERT(previousRootObjectCount != UINT32_MAX, "Exceeded root object count.");

        LinkedList_append_single(&scene->rootObjects, &objectSceneComponent->link);
    }

    Rendering_onObjectAdditionToScene(scene, object, objectSceneComponent);
}


void Scene_removeObject(Scene_t* scene, Base3DObject_t* object) {
    SceneComponent_t* sceneComponent = Object_getComponent(&object->base,
                                                           CORAL_GRAPHICS_COMPONENT_TYPE_SCENE,
                                                           NULL
    );

    Scene__removeObject(scene, object, sceneComponent);
}


void Scene__removeObject(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) {
    SceneComponent_t* currentObjectSceneComponent = sceneComponent;

    while (true) {
        if (currentObjectSceneComponent->childObjectCount) {
            LinkedList_SingleLink_t* childObjectLink = currentObjectSceneComponent->childObjects.headLink.singleLink;
            currentObjectSceneComponent = CORAL_TO_PARENT_PTR(SceneComponent_t, link, childObjectLink);
            continue;
        }

        LinkedList_SingleLink_t* nextObjectLink = currentObjectSceneComponent->link.next;
        Base3DObject_t* currentObject = Scene__getObject(currentObjectSceneComponent);

        Rendering_onObjectRemovalFromScene(scene, currentObject, currentObjectSceneComponent);

        if (currentObjectSceneComponent->parentObject) {
            currentObjectSceneComponent->parentObject->childObjectCount--;
            LinkedList_remove_single(&currentObjectSceneComponent->parentObject->childObjects,
                                     &currentObjectSceneComponent->link
            );
        }
        else {
            scene->rootObjectCount--;
            LinkedList_remove_single(&scene->rootObjects, &currentObjectSceneComponent->link);
        }

        scene->totalObjectCount--;

        if (currentObject == object) { return; }
        if (!nextObjectLink) { nextObjectLink = &currentObjectSceneComponent->parentObject->link; }

        currentObjectSceneComponent = CORAL_TO_PARENT_PTR(SceneComponent_t, link, nextObjectLink);
    };
}


void Scene_changedObjectAttributes(Scene_t* scene, Base3DObject_t* object) {
    SceneComponent_t* sceneComponent = Object_getComponent(&object->base,
                                                                 CORAL_GRAPHICS_COMPONENT_TYPE_SCENE,
                                                                 NULL
    );

    Rendering_onObjectAttributesChanged(scene, object, sceneComponent);
}


void Scene__changedObjectAttributes(Scene_t* scene, Base3DObject_t* object, SceneComponent_t* sceneComponent) {
    Rendering_onObjectAttributesChanged(scene, object, sceneComponent);
}



//- - SCENE OBJECTS

Base3DObject_t* Scene_getObject(const LinkedList_SingleLink_t* objectSceneComponentLink) {
    return Scene__getObject(CORAL_TO_PARENT_PTR(SceneComponent_t, link, objectSceneComponentLink));
}


Base3DObject_t* Scene__getObject(const SceneComponent_t* objectSceneComponent) {
    BaseObject_t* objectBase = (void*)((uintptr_t)objectSceneComponent - objectSceneComponent->offsetToBaseObject);
    return CORAL_TO_PARENT_PTR(Base3DObject_t, base, objectBase);
}


void Scene_getObjectTransformationMatrix(const Base3DObject_t* object, Mat4_t* transformMatrix) {
    SceneComponent_t* sceneComponent = Object_getComponent(&object->base,
                                                           CORAL_GRAPHICS_COMPONENT_TYPE_SCENE,
                                                           NULL
    );

    Scene__getObjectTransformationMatrix(object, sceneComponent, transformMatrix);
}


void Scene__getObjectTransformationMatrix(const Base3DObject_t* object, const SceneComponent_t* sceneComponent, Mat4_t* transformMatrix) {
    const Base3DObject_t* currentObject = object;
    const SceneComponent_t* currentObjectSceneComponent = sceneComponent;

    while (true) {
        Mat4_t currentObjectTransformMatrix;
        uint32_t currentObjectRotationOrder = currentObject->base.flags & CORAL_GRAPHICS_ROTATION_ORDER_BIT_RANGE;

        if (!currentObjectRotationOrder) {
            currentObjectRotationOrder = CORAL_GRAPHICS_OBJECT_USER_FLAG_ROTATION_ORDER_EULER_XYZ;
        }

        Graphics_get3DTransformationMatrix(&currentObjectTransformMatrix,
                                           (currentObject != object) ? (transformMatrix) : (NULL),
                                           &currentObject->position,
                                           &currentObject->rotation,
                                           &currentObject->scale,
                                           currentObjectRotationOrder
        );

        *transformMatrix = currentObjectTransformMatrix;
        SceneComponent_t* nextObjectSceneComponent = currentObjectSceneComponent->parentObject;

        bool end = CORAL_GRAPHICS_OBJECT_HAS_FLAGS(currentObject->base.flags, CORAL_GRAPHICS_OBJECT_USER_FLAG_DETACHED_TRANSFORMATION) ||
                   !nextObjectSceneComponent;

        if (end) { return; }
        else {
            currentObjectSceneComponent = nextObjectSceneComponent;
            currentObject = Scene__getObject(currentObjectSceneComponent);
        }
    }
}
