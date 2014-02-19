#ifndef _MAT3_H_
#define _MAT3_H_

#include <VEC3.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//////////////////////////////////////////////////////////////////////
// 3x3 Matrix class - based on libgfx by Michael Garland
//////////////////////////////////////////////////////////////////////

class MATRIX3
{
private:
  VEC3F row[3];

public:
  // Standard constructors
  MATRIX3() { *this = 0.0; }
  MATRIX3(const VEC3F& r0,const VEC3F& r1,const VEC3F& r2) { row[0]=r0; row[1]=r1; row[2]=r2; }
  MATRIX3(const MATRIX3& m) { *this = m; }
  MATRIX3(float* data);

  // Access methods
  float& operator()(int i, int j)       { return row[i][j]; }
  float  operator()(int i, int j) const { return row[i][j]; }
  VEC3F&       operator[](int i)       { return row[i]; }
  const VEC3F& operator[](int i) const { return row[i]; }
  inline VEC3F col(int i) const {return VEC3F(row[0][i],row[1][i],row[2][i]);}

  operator       float*()       { return row[0]; }
  operator const float*()       { return row[0]; }
  operator const float*() const { return row[0]; }

  // Assignment methods
  inline MATRIX3& operator=(const MATRIX3& m);
  inline MATRIX3& operator=(float s);

  inline MATRIX3& operator+=(const MATRIX3& m);
  inline MATRIX3& operator-=(const MATRIX3& m);
  inline MATRIX3& operator*=(float s);
  inline MATRIX3& operator/=(float s);

  // Construction of standard matrices
  static MATRIX3 I();

  MATRIX3 transpose() const {
    return MATRIX3(this->col(0), this->col(1), this->col(2));
  };
  MATRIX3 adjoint() {
    return MATRIX3(row[1]^row[2], row[2]^row[0], row[0]^row[1]);
  };

  MATRIX3 inverse() {
    MATRIX3 A = adjoint();
    float d = A[0] * row[0];
    if (d == 0.0f) return MATRIX3::I();
    A = A.transpose();
    A /= d;
    return A;
  }

  void clear() {
    row[0].clear(); row[1].clear(); row[2].clear();
  };
};

////////////////////////////////////////////////////////////////////////
// Method definitions
////////////////////////////////////////////////////////////////////////

inline MATRIX3& MATRIX3::operator=(const MATRIX3& m)
	{ row[0] = m[0]; row[1] = m[1]; row[2] = m[2];  return *this; }

inline MATRIX3& MATRIX3::operator=(float s)
	{ row[0]=s;  row[1]=s;  row[2]=s;  return *this; }

inline MATRIX3& MATRIX3::operator+=(const MATRIX3& m)
	{ row[0] += m[0]; row[1] += m[1]; row[2] += m[2]; return *this; }

inline MATRIX3& MATRIX3::operator-=(const MATRIX3& m)
	{ row[0] -= m[0]; row[1] -= m[1]; row[2] -= m[2]; return *this; }

inline MATRIX3& MATRIX3::operator*=(float s)
	{ row[0] *= s; row[1] *= s; row[2] *= s;  return *this; }

inline MATRIX3& MATRIX3::operator/=(float s)
	{ row[0] /= s; row[1] /= s; row[2] /= s;  return *this; }

////////////////////////////////////////////////////////////////////////
// Operator definitions
////////////////////////////////////////////////////////////////////////

inline MATRIX3 operator+(const MATRIX3& n, const MATRIX3& m)
	{ return MATRIX3(n[0]+m[0], n[1]+m[1], n[2]+m[2]); }

inline MATRIX3 operator-(const MATRIX3& n, const MATRIX3& m)
	{ return MATRIX3(n[0]-m[0], n[1]-m[1], n[2]-m[2]); }

inline MATRIX3 operator-(const MATRIX3& m)
	{ return MATRIX3(-m[0], -m[1], -m[2]); }

inline MATRIX3 operator*(float s, const MATRIX3& m)
	{ return MATRIX3(m[0]*s, m[1]*s, m[2]*s); }
inline MATRIX3 operator*(const MATRIX3& m, float s)
	{ return s*m; }

inline MATRIX3 operator/(const MATRIX3& m, float s)
	{ return MATRIX3(m[0]/s, m[1]/s, m[2]/s); }

inline VEC3F operator*(const MATRIX3& m, const VEC3F& v)
	{ return VEC3F(m[0]*v, m[1]*v, m[2]*v); }

extern MATRIX3 operator*(const MATRIX3& n, const MATRIX3& m);

inline std::ostream &operator<<(std::ostream &out, const MATRIX3& M)
	{ return out << "[" << std::endl 
               << M[0] << std::endl << M[1] << std::endl << M[2]
               << std::endl << "]" << std::endl; }

inline std::istream &operator>>(std::istream &in, MATRIX3& M)
	{ return in >> M[0] >> M[1] >> M[2]; }

////////////////////////////////////////////////////////////////////////
// Misc. function definitions
////////////////////////////////////////////////////////////////////////

inline float det(const MATRIX3& m) { return m[0] * (m[1] ^ m[2]); }
inline float trace(const MATRIX3& m) { return m(0,0) + m(1,1) + m(2,2); }

#endif
