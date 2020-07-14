#ifdef __cplusplus
extern "C"
{
#endif




#ifndef aj3dvector_h
#define aj3dvector_h

/* @data AjP3dVector *********************************************************
**
** Ajax 3-D vector object.
**
** Holds three floats
**
** AjP3dVector is implemented as a pointer to a C data structure.
**
** @alias AjS3dVector
** @alias AjO3dVector
**
** @new aj3dVectorNew default constructor
** @new aj3dVectorCreate constructor initializing values of vector components
**
** @delete aj3dVectorDel default destructor
**
** @use aj3dVectorSum return sum of two vectors
** @use aj3dVectorDotProduct return dot product of two vectors
** @use aj3dVectorCrossProduct return cross product of two vectors
** @use aj3dVectorAngle return angle between two vectors
** @use aj3dVectorDihedralAngle return angle between two planes
** @modify aj3dVectorBetweenPoints return vector between two points
**
** @cast aj3dVectorLength return length of vector
**
** @attr x [float] x coordinate
** @attr y [float] y coordinate
** @attr z [float] z coordinate
** @@
******************************************************************************/

typedef struct AjS3dVector
{
    float x;
    float y;
    float z;
} AjO3dVector;
#define AjP3dVector AjO3dVector*




/* ========================================================================= */
/* =================== All functions in alphabetical order ================= */
/* ========================================================================= */

/* aj3dvector.h () $Date: 2004/07/14 15:36:49 $                    DJC Oct03 */

float       aj3dVectorAngle(const AjP3dVector ajp3dVectorFirst,
			    const AjP3dVector ajp3dVectorSecond);
void        aj3dVectorBetweenPoints(AjP3dVector ajp3dVectorBetweenPoints,
				    float startX, float startY,
				    float startZ, float endX, float endY,
				    float endZ);
AjP3dVector aj3dVectorCreate(float fX, float fY, float fZ);
void        aj3dVectorCrossProduct(const AjP3dVector ajp3dVectorFirst,
				   const AjP3dVector ajp3dVectorSecond,
				   AjP3dVector ajp3dVectorCrossProduct);
float       aj3dVectorDihedralAngle(const AjP3dVector ajp3dVectorA,
				    const AjP3dVector ajp3dVectorB,
				    const AjP3dVector ajp3dVectorC);
void        aj3dVectorDel(AjP3dVector* ajp3dVectorToBeDestroyed);
float       aj3dVectorDotProduct(const AjP3dVector ajp3dVectorFirst,
				 const AjP3dVector ajp3dVectorSecond);
float       aj3dVectorLength(const AjP3dVector ajp3dVectorToBeSized);
AjP3dVector aj3dVectorNew(void);
void        aj3dVectorSum(const AjP3dVector ajp3dVectorFirst,
			  const AjP3dVector ajp3dVectorSecond,
			  AjP3dVector ajp3dVectorSum);

#endif




#ifdef __cplusplus
}
#endif
