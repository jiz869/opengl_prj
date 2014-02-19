#ifndef _VEC3_H_
#define _VEC3_H_

//////////////////////////////////////////////////////////////////////
// Credit: Based on libgfx by Michael Garland
//////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>

template<class T>
class TVEC3 {
public:
  //T element[3];
  
  union {
     struct { T x,y,z; };
     struct { T r,g,b; };
     T element[3];
  };

public:
  // Standard constructors
  TVEC3(T s=0) { *this = s; }
  TVEC3(T x, T y, T z) { element[0]=x; element[1]=y; element[2]=z; }

  // Copy constructors & assignment operators
  template<class U> TVEC3(const TVEC3<U>& v) { *this = v; }
  template<class U> TVEC3(const U v[3])
    { element[0]=v[0]; element[1]=v[1]; element[2]=v[2]; }
  template<class U> TVEC3& operator=(const TVEC3<U>& v)
    { element[0]=v[0];  element[1]=v[1];  element[2]=v[2];  return *this; }
  TVEC3& operator=(T s) { element[0]=element[1]=element[2]=s; return *this; }

  // Descriptive interface
  typedef T value_type;
  static int dim() { return 3; }

  // Access methods
  operator       T*()       { return element; }
  operator const T*() const { return element; }

  T& operator[](int i)       { return element[i]; }
  T  operator[](int i) const { return element[i]; }
  operator const T*()        { return element; }

  // Assignment and in-place arithmetic methods
  inline TVEC3& operator+=(const TVEC3& v);
  inline TVEC3& operator-=(const TVEC3& v);
  inline TVEC3& operator*=(T s);
  inline TVEC3& operator/=(T s);

  void normalize() {
    T l = (*this) * (*this);
    if( l!=1.0 && l!=0.0 )  *this /= sqrt(l);
  };
  void clear() {
    T zero = 0.0;
    element[0] = zero;
    element[1] = zero;
    element[2] = zero;
  };

  void equals(const TVEC3& v) {
    element[0] = v.element[0]; element[1] = v.element[1]; element[2] = v.element[2];
  };
};

////////////////////////////////////////////////////////////////////////
// Method definitions
////////////////////////////////////////////////////////////////////////

template<class T> inline TVEC3<T>& TVEC3<T>::operator+=(const TVEC3<T>& v)
  { element[0] += v[0];   element[1] += v[1];   element[2] += v[2];  return *this; }

template<class T> inline TVEC3<T>& TVEC3<T>::operator-=(const TVEC3<T>& v)
  { element[0] -= v[0];   element[1] -= v[1];   element[2] -= v[2];  return *this; }

template<class T> inline TVEC3<T>& TVEC3<T>::operator*=(T s)
  { element[0] *= s;   element[1] *= s;   element[2] *= s;  return *this; }

template<class T> inline TVEC3<T>& TVEC3<T>::operator/=(T s)
  { element[0] /= s;   element[1] /= s;   element[2] /= s;  return *this; }

////////////////////////////////////////////////////////////////////////
// Operator definitions
////////////////////////////////////////////////////////////////////////

template<class T>
inline TVEC3<T> operator+(const TVEC3<T> &u, const TVEC3<T>& v)
  { return TVEC3<T>(u[0]+v[0], u[1]+v[1], u[2]+v[2]); }

template<class T>
inline TVEC3<T> operator-(const TVEC3<T> &u, const TVEC3<T>& v)
  { return TVEC3<T>(u[0]-v[0], u[1]-v[1], u[2]-v[2]); }

template<class T> inline TVEC3<T> operator-(const TVEC3<T> &v)
  { return TVEC3<T>(-v[0], -v[1], -v[2]); }

template<class T> inline TVEC3<T> operator*(const T s, const TVEC3<T> &v)
  { return TVEC3<T>(v[0]*s, v[1]*s, v[2]*s); }

template<class T> inline TVEC3<T> operator*(const TVEC3<T> &v, const T s)
  { return s*v; }

template<class T> inline TVEC3<T> operator/(const TVEC3<T> &v, T s)
  { return TVEC3<T>(v[0]/s, v[1]/s, v[2]/s); }

template<class T> inline T operator*(const TVEC3<T> &u, const TVEC3<T>& v)
  { return u[0]*v[0] + u[1]*v[1] + u[2]*v[2]; }

template<class T> inline TVEC3<T> cross(const TVEC3<T>& u, const TVEC3<T>& v)
  { return TVEC3<T>( u[1]*v[2] - v[1]*u[2], -u[0]*v[2] + v[0]*u[2], u[0]*v[1] - v[0]*u[1] ); }

template<class T>
inline TVEC3<T> operator^(const TVEC3<T>& u, const TVEC3<T>& v)
  { return cross(u, v); }

template<class T>
inline std::ostream &operator<<(std::ostream &out, const TVEC3<T>& v)
  { return out << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")"; }

template<class T>
inline std::istream &operator>>(std::istream &in, TVEC3<T>& v)
  { return in >> v[0] >> v[1] >> v[2]; }

typedef TVEC3<float> VEC3;
typedef TVEC3<float> VEC3F;

#endif

