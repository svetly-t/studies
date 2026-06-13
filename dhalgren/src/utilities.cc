#include "utilities.h"

double signOf(double d) {
    return (d > 0) ? 1 : ((d < 0) ? -1 : 0);
}

double degToRad(double degree) {
    return degree / 180.0 * 3.14159;
}

double determinant2d(double a11, double a12, double a21, double a22) {
    return a11 * a22 - a12 * a21;
}

void findDeterminantIndices3d(int idx, int &idx_1, int &idx_2) {
    int temp;
    idx_1 = (idx + 1) % 3;
    idx_2 = (idx + 2) % 3;
    if (idx_1 > idx_2) {
        temp = idx_2;
        idx_2 = idx_1;
        idx_1 = temp;
    }
}

double determinant3d(double A[][3]) {
    int col;
    int row_1, row_2, col_1, col_2;
    double result;
    double sign;

    result = 0;

    findDeterminantIndices3d(0, row_1, row_2);

    for (col = 0; col < 3; ++col) {
        sign = 1.0;
        if (((0 + col) % 2) == 1)
            sign = -1.0;

        findDeterminantIndices3d(col, col_1, col_2);
        result += A[0][col] * sign * determinant2d(A[row_1][col_1], A[row_2][col_1], A[row_1][col_2], A[row_2][col_2]);
    }

    return result;
}

// Adjunct of a matrix https://th.bing.com/th/id/OIP.7VRe9mj1dfaDsAXgyUD-wQAAAA?rs=1&pid=ImgDetMain

// I just realized this is totally bunk. Ignore this function
void findIntersection(V2d p1, V2d p2, V2d p3, double &a, double &b, double &c) {
    double B[3];
    double A[3][3];
    double result[3];
    double det_A;
    double sign;
    int row, col;
    int row_1, row_2, col_1, col_2;

    V2d *p[3] = { &p1, &p2, &p3 };

    for (row = 0; row < 3; ++row) {
        B[row] = p[row]->y;
        A[row][0] = p[row]->x * p[row]->x;
        A[row][1] = p[row]->x;
        A[row][2] = 1;
    }

    det_A = determinant3d(A);

    // if det is zero then throw an exception...

    for (row = 0; row < 3; ++row) {
        result[row] = 0;
        findDeterminantIndices3d(row, row_1, row_2);

        for (col = 0; col < 3; ++col) {
            sign = 1.0;
            if (((row + col) % 2) == 1)
                sign = -1.0;

            findDeterminantIndices3d(col, col_1, col_2);
            result[row] += B[row] * sign * determinant2d(A[row_1][col_1], A[row_2][col_1], A[row_1][col_2], A[row_2][col_2]);
        }

        result[row] /= det_A;
    }

    a = result[0];
    b = result[1];
    c = result[2];
}

double findPolynomialScale(double zero_1, double zero_2, V2d through_this_point) {
    return through_this_point.y / (through_this_point.x - zero_1) / (through_this_point.x - zero_2); 
}

double singleRopeConstraint(V2d &pos1, V2d &pos2, double w1, double w2, double dist) {
    V2d real = pos2 - pos1;

    V2d dir = real.Normalized();

    double real_dist = real.Magnitude();

    double offset = real_dist - dist;

    // move pos1 towards pos2 by weight 1
    pos1 += dir * offset * w1;

    pos2 -= dir * offset * w2;

    return offset;
}
