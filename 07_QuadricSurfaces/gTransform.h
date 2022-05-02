#pragma once

struct Vec3f {  // 3차원 좌표를 표현하기 위한 구조체
    float x, y, z;
    Vec3f() {}
    Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    float   dot(const Vec3f& v) const;
    Vec3f   cross(const Vec3f& v) const;
    void    normalize();
};

class TransMat4f {
    float   mat[4][4];
public:
    TransMat4f() {}
    TransMat4f(const TransMat4f& m);
    const   float*  getPMat() const { return &mat[0][0]; }
    TransMat4f& operator =(const TransMat4f& m);
    TransMat4f  operator * (const TransMat4f& m) const;
    const TransMat4f&   operator *= (const TransMat4f& m);
    void    identity();
    void    translate(const Vec3f& d);
    void    scale(const Vec3f& s);
    void    rotateX(float theta);
    void    rotateY(float theta);
    void    rotateZ(float theta);
    void    viewingTrans(const Vec3f& pOrigin, const Vec3f& pRef, const Vec3f& vUp);
    void    orthgonalPrj(float xmin, float xmax, float ymin, float ymax, float znear, float zfar);
    void    perspectivePrj(float xmin, float xmax, float ymin, float ymax, float znear, float zfar);
};
