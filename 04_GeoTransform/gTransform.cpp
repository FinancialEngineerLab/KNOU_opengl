#include    <cstring>
#include    <cmath>
using namespace std;
#include    "gTransform.h"

TransMat4f  TransMat4f::operator * (const TransMat4f& m) const {
    TransMat4f   R;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++)
            R.mat[r][c] = mat[r][0] * m.mat[0][c]
            + mat[r][1] * m.mat[1][c]
            + mat[r][2] * m.mat[2][c]
            + mat[r][3] * m.mat[3][c];
    }
    return R;
}

const TransMat4f&   TransMat4f::operator *= (const TransMat4f& m) {
    TransMat4f  R;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++)
            R.mat[r][c] = mat[r][0] * m.mat[0][c]
            + mat[r][1] * m.mat[1][c]
            + mat[r][2] * m.mat[2][c]
            + mat[r][3] * m.mat[3][c];
    }
    memcpy_s(mat, sizeof(mat), R.mat, sizeof(mat));
    return *this;
}

void TransMat4f::identity() {
    memset(mat, 0, sizeof(mat));
    mat[0][0] = mat[1][1] = mat[2][2] = mat[3][3] = 1.0f;
}

void    TransMat4f::translate(const Vec3f& d) {
    identity();
    mat[0][3] = d.x;
    mat[1][3] = d.y;
    mat[2][3] = d.z;
}

void    TransMat4f::scale(const Vec3f& s) {
    identity();
    mat[0][0] = s.x;
    mat[1][1] = s.y;
    mat[2][2] = s.z;
}

void    TransMat4f::rotateX(float theta) {
    identity();
    mat[1][1] = mat[2][2] = cos(theta);
    mat[2][1] = -(mat[1][2] = sin(theta));
}

void    TransMat4f::rotateY(float theta) {
    identity();
    mat[0][0] = mat[2][2] = cos(theta);
    mat[0][2] = -(mat[2][0] = sin(theta));
}

void    TransMat4f::rotateZ(float theta) {
    identity();
    mat[0][0] = mat[1][1] = cos(theta);
    mat[0][1] = -(mat[1][0] = sin(theta));
}
