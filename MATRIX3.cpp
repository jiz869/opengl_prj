#include "MATRIX3.h"

MATRIX3::MATRIX3(float* data)
{
  for (int y = 0; y < 3; y++)
    for (int x = 0; x < 3; x++)
      (*this)(x,y) = data[x + y * 3];
}

MATRIX3 MATRIX3::I() 
{ 
  return MATRIX3(VEC3F(1,0,0), VEC3F(0,1,0), VEC3F(0,0,1)); 
}

MATRIX3 operator*(const MATRIX3& n, const MATRIX3& m)
{
  MATRIX3 A;

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      A(i,j) = n[i] * m.col(j);
  return A;
}
