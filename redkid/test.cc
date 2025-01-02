// Unit tests, etc.

#include <iostream>
#include <algorithm>

#include "include/utilities.h"

void TestDeterminant3d() {
    double expected = -16.0;
    double result;
    double matrix[3][3];

    matrix[0][0] = 2;
    matrix[0][1] = 1;
    matrix[0][2] = 3;
    matrix[1][0] = 4;
    matrix[1][1] = 0;
    matrix[1][2] = 1;
    matrix[2][0] = 2;
    matrix[2][1] = -1;
    matrix[2][2] = 2;

    result = determinant3d(matrix);

    std::cout << "Expected " << expected << " got " << result << std::endl;

    if (result != expected) {
        abort();
    }
}

void TestFindIntersection() {
    double a, b, c;

    V2d p1(-2, 0);
    V2d p2(0, 4);
    V2d p3(2, 0);

    findIntersection(p1, p2, p3, a, b, c);

    std::cout << "Got coefficients a: " << a << " b: " << b << " c: " << c << std::endl;
}

int main() {
    TestDeterminant3d();
    TestFindIntersection();
}