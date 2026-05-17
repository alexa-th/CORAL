#include <stddef.h>
#include <stdio.h>
#include <stdalign.h>
#include "unity.h"
#include "common/utils.h"
#include "memory/allocators.h"
#include "memory/dynalloc.h"
#include "math/vec.h"
#include "graphics/graphics.h"



// DEFINITIONS

#define OBJECT_BUFFER_SIZE  128U
#define OBJECT_COUNT        16U



// HELPER FUNCTIONS

void assert_alignment(uintptr_t location, size_t alignment) {
    TEST_ASSERT_MESSAGE(!(location % alignment), "The provided location or offset did not satisfy the required alignment.");
}


void compareVecs(Vec4_t* expected, Vec4_t* actual) {
    char message[128U];

    snprintf(message,
             sizeof(message),
             "Expected: %lf, %lf, %lf; Actual: %lf, %lf, %lf",
             (double)expected->x,
             (double)expected->y,
             (double)expected->z,
             (double)actual->x,
             (double)actual->y,
             (double)actual->z
    );

    TEST_MESSAGE(message);


    for (size_t i = 0U; i < CORAL_ARRAY_LENGTH(expected->raw); i++) {
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.0005, (double)expected->raw[i], (double)actual->raw[i], "Incorrect coordinate.");
    }
}



// TESTS

void test_transform(void) {
    Vec3_t translation = {42.0, 41.0, 40.0};
    Vec3_t rotation = {.pitch = 45.0, .yaw = 45.0, .roll = 60.0};
    Vec3_t scale = {1.0, 2.0, 1.0};

    Mat4_t transformMatrix;
    Graphics_get3DTransformationMatrix(&transformMatrix,
                                       NULL,
                                       &translation,
                                       &rotation,
                                       &scale,
                                       CORAL_GRAPHICS_ROTATION_ORDER_EULER_XYZ
    );

    Vec4_t vec = {1.0, 2.0, 3.0, 1.0};
    Vec4_t expected = {43.4912, 44.9970, 42.7929, 1.0}; // Calculated using Blender
    Vec4_t actual;

    CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &transformMatrix, &vec, &actual);
    compareVecs(&expected, &actual);
}

void test_object(void) {
    ComponentClass_t componentClass;
    ComponentClassElement_t components[] = {{CORAL_GRAPHICS_COMPONENT_TYPE_MATERIAL},
                                            {CORAL_GRAPHICS_COMPONENT_TYPE_POINTER}
    };

    alignas(SphereObject_t) unsigned char objectBuffer[128U];

    {
        ComponentClass_init(&componentClass, NULL, components, CORAL_ARRAY_LENGTH(components));

        assert_alignment(componentClass.components[0U].offsetFromComponents, alignof(MaterialComponent_t));
        assert_alignment(componentClass.components[1U].offsetFromComponents, alignof(PointerComponent_t));
    }

    {
        size_t sizeRequirement = Object_getSizeRequirement(CORAL_GRAPHICS_OBJECT_TYPE_SPHERE, &componentClass, 0U);
        TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(sizeof(objectBuffer), sizeRequirement, "The object's size requirement exceeds the size of objectBuffer.");

        char message[32U];
        snprintf(message, sizeof(message), "Required object size: %zu", sizeRequirement);
        TEST_MESSAGE(message);
    }

    {
        Object__init(CORAL_GRAPHICS_OBJECT_TYPE_SPHERE, &componentClass, 0U, objectBuffer);
        SphereObject_t* sphere = (void*)objectBuffer;

        assert_alignment((uintptr_t)sphere, alignof(SphereObject_t));

        uintptr_t componentsBegin = (uintptr_t)sphere + sphere->base3D.base.offsetToComponents;

        assert_alignment(componentsBegin, componentClass.componentAlignment);
        assert_alignment(componentsBegin + componentClass.components[0U].offsetFromComponents, alignof(MaterialComponent_t));
        assert_alignment(componentsBegin + componentClass.components[1U].offsetFromComponents, alignof(PointerComponent_t));

        Object_destr(&sphere->base3D.base);
    }
}


void test_scene(void) {
    ComponentClass_t objectComponentClass;
    ComponentClassElement_t objectComponents[] = {{CORAL_GRAPHICS_COMPONENT_TYPE_SCENE}};
    ComponentClass_init(&objectComponentClass, NULL, objectComponents, CORAL_ARRAY_LENGTH(objectComponents));

    size_t elementCount = OBJECT_COUNT;
    size_t elementSize = Object_getSizeRequirement(CORAL_GRAPHICS_OBJECT_TYPE_SPHERE,
                                                   &objectComponentClass,
                                                   CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC
    );

    void* objectBuffer = CORAL_calloc(elementCount, elementSize);

    PoolAllocator_t objectAllocator;
    PoolAllocator_init(&objectAllocator, objectBuffer, elementCount, elementSize);

    Scene_t scene;
    Scene_init(&scene);

    SphereObject_t* spheres[OBJECT_COUNT];

    {
        for (size_t i = 0U; i < OBJECT_COUNT; i++) {
            spheres[i] = PoolAllocator_alloc(&objectAllocator);
            Base3DObject_t* parentSphere = ((i % 2) && i) ?
                                           (&spheres[i / 2U]->base3D) :
                                           (NULL)
            ;

            Object__init(CORAL_GRAPHICS_OBJECT_TYPE_SPHERE,
                         &objectComponentClass,
                         CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC,
                         spheres[i]
            );

            Scene_addObject(&scene, &spheres[i]->base3D, parentSphere);
        }

        TEST_ASSERT_EQUAL_MESSAGE(OBJECT_COUNT, scene.totalObjectCount, "Not all objects were added to the scene.");
    }

    {
        for (size_t i = 0U; i < OBJECT_COUNT; i += 3U) {
            Scene_removeObject(&scene, &spheres[OBJECT_COUNT - 1U - i]->base3D);
        }

        Scene_destr(&scene);

        TEST_ASSERT_EQUAL_MESSAGE(0U, scene.rootObjectCount, "Not all root objects were removed from the scene.");
        TEST_ASSERT_EQUAL_MESSAGE(0U, scene.totalObjectCount, "Not all objects were removed from the scene.");
    }

    for (size_t i = 0U; i < OBJECT_COUNT; i++) {
        Object_destr(&spheres[i]->base3D.base);
    }

    CORAL_free(objectBuffer);
}


void test_sceneTransform(void) {
    ComponentClass_t objectComponentClass;
    ComponentClassElement_t objectComponents[] = {{CORAL_GRAPHICS_COMPONENT_TYPE_SCENE}};
    ComponentClass_init(&objectComponentClass, NULL, objectComponents, CORAL_ARRAY_LENGTH(objectComponents));

    Scene_t scene;
    Scene_init(&scene);

    Base3DObject_t* parentObject = CORAL_TO_PARENT_PTR(Base3DObject_t,
                                                       base,
                                                       Object_init(CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D,
                                                                   &objectComponentClass,
                                                                   CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC
                                                       )
    );

    Base3DObject_t* childObject = CORAL_TO_PARENT_PTR(Base3DObject_t,
                                                      base,
                                                      Object_init(CORAL_GRAPHICS_OBJECT_TYPE_BASE_3D,
                                                                  &objectComponentClass,
                                                                  CORAL_GRAPHICS_OBJECT_USER_FLAG_STATIC
                                                      )
    );

    parentObject->position = ((Vec3_t){12.0, -20.0, 66.6});
    parentObject->rotation = ((Vec3_t){20.0, 0.0, -70.0});
    parentObject->scale = ((Vec3_t){0.5, 5.0, 6.9});

    childObject->position = ((Vec3_t){-5.0, 42.0, -2.0});
    childObject->rotation = ((Vec3_t){16.0, 32.0, 64.0});
    childObject->scale = ((Vec3_t){1.5, 2.5, 0.5});

    uint16_t sceneComponentOffset = objectComponents[0U].offsetFromComponents;
    Scene_addObject(&scene, parentObject, NULL);
    Scene_addObject(&scene, childObject, parentObject);

    {
        Vec4_t vec = {1.0, 2.0, 3.0, 1.0};
        Vec4_t expected = {14.3484, -19.6773, 89.4718, 1.0};    // Calculated using Blender
        Vec4_t actual;

        Mat4_t parentTransformMatrix;
        Scene_getObjectTransformationMatrix(parentObject, &parentTransformMatrix);

        CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &parentTransformMatrix, &vec, &actual);
        compareVecs(&expected, &actual);
    }

    {
        Vec4_t vec = {-2.0, -1.0, 0.0, 1.0};
        Vec4_t expected = {182.7400, 44.3342, 125.7020, 1.0};   // Calculated using Blender
        Vec4_t actual;

        Mat4_t childTransformMatrix;
        Scene_getObjectTransformationMatrix(childObject, &childTransformMatrix);

        CORAL_MATH_VEC_MUL_MAT_GENERIC(4, 4, &childTransformMatrix, &vec, &actual);
        compareVecs(&expected, &actual);
    }

    Scene_destr(&scene);
    Object_destr(&childObject->base);
    Object_destr(&parentObject->base);
}



// UNITY

void setUp(void) {}
void tearDown(void) {}



// MAIN

int main(void) {
    Graphics_startup();

    RUN_TEST(test_transform);
    RUN_TEST(test_object);
    RUN_TEST(test_scene);
    RUN_TEST(test_sceneTransform);

    Graphics_shutdown();

    return UNITY_END();
}
