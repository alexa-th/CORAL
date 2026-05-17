#include "unity.h"
#include "common/error.h"
#include "math/math.h"



// TESTS

void test_vectorAddition(void) {
    Vec3d_u a = {4.0, -2.0, 1.0};
    Vec3d_u b = {12.0, 16.0, 4.0};
    Vec3d_u expected = {16.0, 14.0, 5.0};
    Vec3d_u actual;
    Vec3d_add(&a, &b, &actual);

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U, "Vector addition failed.");
}


void test_vectorMultiplication(void) {
    Vec3d_u a = {24.0, 0.0, -128.0};
    Vec3d_u b = {2.0, 1234.0, 0.5};
    Vec3d_u expected = {48.0, 0.0, -64.0};
    Vec3d_u actual;
    Vec3d_mul(&a, &b, &actual);

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U, "Vector multiplication failed.");
}


void test_vectorDotProduct(void) {
    Vec3d_u a = {2.0, 2.0, -3.0};
    Vec3d_u b = {-6.0, -6.0, 9.0};
    double expected = -1.0;
    double actual = Vec3d_dot(&a, &b);

    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected, actual, "Vector dot product failed.");
}


void test_vectorCrossProduct(void) {
    Vec3d_u a = {-10.0, -5.0, 6.0};
    Vec3d_u b = {-10.0, 0.0, 0.0};
    Vec3d_u expected = {0.0, -60.0, -50.0};
    Vec3d_u actual;
    Vec3d_cross(&a, &b, &actual);

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U, "Vector cross product failed.");
}


void test_matrixVectorMultiplication(void) {
    Mat3x3d_u mat = {.columnVecs = {{1.0, 2.0, -3.0},
                                    {1.5, -0.5, 4.0},
                                    {-3.0, 6.0, 1.5}
    }};
    Vec3d_u vec = {3.0, 5.0, 2.0};
    Vec3d_u expected = {4.5, 15.5, 14};
    Vec3d_u actual;
    Vec3d_mul_Mat3x3(&mat, &vec, &actual);

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U, "Matrix-vector multiplication failed.");
}


void test_matrixMatrixMultiplication(void) {
    Mat3x3d_u a = {.columnVecs = {{-1.0, 2.0, 5.0},
                                  {4.0, 0.5, -2.0},
                                  {6.0, 1.0, -1.5}
    }};
    Mat3x3d_u b = {.columnVecs = {{1.0, 2.0, 3.0},
                                  {-2.5, 2.0, 1.0},
                                  {10.0, -3.5, 5.5}
    }};
    Mat3x3d_u expected = {.columnVecs = {{25.0, 6.0, -3.5},
                                         {16.5, -3.0, -18.0},
                                         {9.0, 23.75, 48.75}
    }};
    Mat3x3d_u actual;
    Mat3x3d_mul(&a, &b, &actual);

    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U * 3U, "Matrix-matrix multiplication failed.");
}


void test_matrixDeterminant(void) {
    Mat3x3d_u mat = {.columnVecs = {{25.0, 6.0, -3.5},
                                    {16.5, -3.0, -18.0},
                                    {9.0, 23.75, 48.75}
    }};
    double expected = -233.0625;
    double actual = Mat3x3d_determinant(&mat);

    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected, actual, "Matrix determinant failed.");
}


void test_matrixInvert(void) {
    Mat3x3d_u mat = {.columnVecs = {{5.0, -6.0, 3.5},
                                    {1.5, 3.0, -8.0},
                                    {4.0, 7.5, 2.0}
    }};
    Mat3x3d_u expected = {.columnVecs = {{176.0 / 1433.0, 102.0 / 1433.0, 100.0 / 1433.0},
                                         {-280.0 / 4299.0, -32.0 / 4299.0, 362.0 / 4299.0},
                                         {-2.0 / 1433.0, -164.0 / 1433.0, 64.0/ 1433.0}
    }};
    Mat3x3d_u actual;
    Error_t error = Mat3x3d_invert(&mat, &actual);

    TEST_ASSERT_EQUAL_MESSAGE(CORAL_ERROR_NONE, error, "Matrix was not invertable.");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected.raw, actual.raw, 3U * 3U, "Matrix inversion failed.");
}



// UNITY

void setUp(void) {}
void tearDown(void) {}



// MAIN

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_vectorAddition);
    RUN_TEST(test_vectorMultiplication);
    RUN_TEST(test_vectorDotProduct);
    RUN_TEST(test_vectorCrossProduct);
    RUN_TEST(test_matrixVectorMultiplication);
    RUN_TEST(test_matrixMatrixMultiplication);
    RUN_TEST(test_matrixDeterminant);
    RUN_TEST(test_matrixInvert);

    return UNITY_END();
}
