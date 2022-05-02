#pragma once

struct Vec3f {	// 3차원 좌표를 표현하기 위한 구조체
    float x, y, z;
    Vec3f() {}
    Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

class TransMat4f {
    float   mat[4][4];
public:
    TransMat4f() {}
    TransMat4f  operator * (const TransMat4f& m) const;
    const TransMat4f&   operator *= (const TransMat4f& m);
    void    identity();
    void    translate(const Vec3f& d);
    void    scale(const Vec3f& s);
    void    rotateX(float theta);
    void    rotateY(float theta);
    void    rotateZ(float theta);
    const   float*  getPMat() const { return &mat[0][0]; }
};
