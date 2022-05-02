#include    <cstring>
#include    <cmath>
using namespace std;
#include    "gTransform.h"

float   Vec3f::dot(const Vec3f& v) const {
    return x*v.x + y*v.y + z*v.y;
}

Vec3f   Vec3f::cross(const Vec3f& v) const {
    return Vec3f(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

void    Vec3f::normalize() {
    float   len = sqrt(x*x + y*y + z*z);
    x /= len;
    y /= len;
    z /= len;
}

TransMat4f::TransMat4f(const TransMat4f& m) {
    memcpy_s(mat, sizeof(mat), m.mat, sizeof(mat));
}

TransMat4f& TransMat4f::operator =(const TransMat4f& m) {
    memcpy_s(mat, sizeof(mat), m.mat, sizeof(mat));
    return *this;
}

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

void    TransMat4f::viewingTrans(const Vec3f& pOrigin, const Vec3f& pRef, const Vec3f& vUp) {
    Vec3f   n = Vec3f(pOrigin.x - pRef.x, pOrigin.y - pRef.y, pOrigin.z - pRef.z);
    n.normalize();
    Vec3f   u = vUp.cross(n);
    u.normalize();
    Vec3f   v = n.cross(u);
    mat[0][0] = u.x;    mat[0][1] = u.y;    mat[0][2] = u.z;
    mat[1][0] = v.x;    mat[1][1] = v.y;    mat[1][2] = v.z;
    mat[2][0] = n.x;    mat[2][1] = n.y;    mat[2][2] = n.z;
    mat[0][3] = -u.dot(pOrigin);
    mat[1][3] = -v.dot(pOrigin);
    mat[2][3] = -n.dot(pOrigin);
    mat[3][0] = mat[3][1] = mat[3][2] = 0.0f;
    mat[3][3] = 1.0f;
}

void    TransMat4f::orthgonalPrj(float xmin, float xmax, float ymin, float ymax, float znear, float zfar) {
    mat[0][0] = 2.0f / (xmax - xmin);
    mat[0][1] = mat[0][2] = 0.0f;
    mat[0][3] = -(xmax + xmin) / (xmax - xmin);
    mat[1][0] = mat[1][2] = 0;
    mat[1][1] = 2.0f / (ymax - ymin);
    mat[1][3] = -(ymax + ymin) / (ymax - ymin);
    mat[2][0] = mat[2][1] = 0.0f;
    mat[2][2] = 2.0f / (znear - zfar);
    mat[2][3] = (znear + zfar) / (znear - zfar);
    mat[3][0] = mat[3][1] = mat[3][2] = 0.0f;
    mat[3][3] = 1.0f;
}

void    TransMat4f::perspectivePrj(float xmin, float xmax, float ymin, float ymax, float znear, float zfar) {
    identity();
    mat[0][0] = 2.0 * znear / (xmax - xmin);
    mat[0][2] = (xmax + xmin) / (xmax - xmin);
    mat[1][1] = 2.0 * znear / (ymax - ymin);
    mat[1][2] = (ymax + ymin) / (ymax - ymin);
    mat[2][2] = (znear + zfar) / (znear - zfar);
    mat[2][3] = 2.0f *znear * zfar / (znear - zfar);
    mat[3][2] = -1.0;
    mat[3][3] = 0;
}
