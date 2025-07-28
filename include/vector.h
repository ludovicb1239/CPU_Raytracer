#ifndef VECTOR_H
#define VECTOR_H

typedef struct
{
    double x, y, z;
} VECTOR;

#include "rmath.h"

inline VECTOR substractVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x - u.x;
    vectorReturn.y = v.y - u.y;
    vectorReturn.z = v.z - u.z;
    return vectorReturn;
}
inline VECTOR multiplyVectors(VECTOR v, double u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x * u;
    vectorReturn.y = v.y * u;
    vectorReturn.z = v.z * u;
    return vectorReturn;
}
inline double dot(VECTOR v, VECTOR u){
    return v.x*u.x + v.y*u.y + v.z*u.z;
}
inline VECTOR reflected(VECTOR vector, VECTOR axis){
    return substractVectors(vector, multiplyVectors(axis, 2.0 * dot(vector, axis)));
}
inline VECTOR addVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x + u.x;
    vectorReturn.y = v.y + u.y;
    vectorReturn.z = v.z + u.z;
    return vectorReturn;
}
inline VECTOR multiplyTwoVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x * u.x;
    vectorReturn.y = v.y * u.y;
    vectorReturn.z = v.z * u.z;
    return vectorReturn;
}
inline VECTOR normalize(VECTOR p){
    const float squared = (p.x*p.x + p.y*p.y + p.z*p.z);

	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = squared * 0.5F;
	y = squared;
	i = * ( long * ) &y; // evil floating point bit level hacking
	i = 0x5f3759df - ( i >> 1 ); // what the fuck?
	y = * ( float * ) &i;
	y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration

    VECTOR vectorReturn;
    vectorReturn.x = p.x * y;
    vectorReturn.y = p.y * y;
    vectorReturn.z = p.z * y;

    return vectorReturn;
}
inline VECTOR randomVECTOR(){
    VECTOR newVec;
    newVec.x = RV(-1.0,1.0);
    newVec.y = RV(-1.0,1.0);
    newVec.z = RV(-1.0,1.0);
    return (normalize(newVec));
}


#endif // VECTOR_H