/******************************************************************************
** @source AJAX 3D vector functions
**
** AjP3dVector objects are structures each of three floats specifying the
**  components of vectors in Cartesian three-space.
**
** The three float values stored in an AjP3dVector are, respectively,
**  the components of the vector it describes in the positive X, Y and
**  Z directions in a conventional right-handed Cartesian system
**
** Alternatively they can be thought of as the coefficients of the 
**  i, j, and k unit vectors in the x y and z directions respectively
**
** @author Copyright (C) 2003 Damian Counsell
** @version $Revision: 1.11 $
** @modified $Date: 2004/05/06 15:09:14 $
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
******************************************************************************/




/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

#include "ajax.h"
#include <math.h>


/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* ==================================================================== */
/* ======================== private functions ========================= */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= constructors ============================= */
/* ==================================================================== */

/* @section 3-D vector Constructors *******************************************
**
** All constructors return a new vector by pointer. It is the responsibility
** of the user to first destroy any previous vector. The target pointer
** does not need to be initialised to NULL, but it is good programming practice
** to do so anyway.
**
******************************************************************************/

/* @func aj3dVectorNew *******************************************************
**
** Default constructor for zeroed AJAX 3D vectors.
**
** @return [AjP3dVector] Pointer to a zeroed 3D vector
** @category new [AjP3dVector] default constructor
** @@
******************************************************************************/

AjP3dVector aj3dVectorNew(void)
{
    AjP3dVector ajp3dVectorReturnedVector = NULL;

    AJNEW0(ajp3dVectorReturnedVector);

    return ajp3dVectorReturnedVector;
}




/* @func aj3dVectorCreate ****************************************************
**
** Constructor for initialized AJAX 3D vectors.
** @param [r] fX [float] x component of 3D vector
** @param [r] fY [float] y component of 3D vector
** @param [r] fZ [float] z component of 3D vector
**
** @return [AjP3dVector] Pointer to an initialized 3D vector
** @category new [AjP3dVector] constructor initializing values of
**                vector components
** @@
******************************************************************************/

AjP3dVector aj3dVectorCreate(float fX, float fY, float fZ)
{
    AjP3dVector ajp3dVectorReturnedVector;

    AJNEW0(ajp3dVectorReturnedVector);
    ajp3dVectorReturnedVector->x = fX;
    ajp3dVectorReturnedVector->y = fY;
    ajp3dVectorReturnedVector->z = fZ;

    return ajp3dVectorReturnedVector;  
}




/* ==================================================================== */
/* =========================== destructor ============================= */
/* ==================================================================== */


/* @section 3D Vector Destructors ********************************************
**
** Destruction is achieved by deleting the pointer to the 3-D vector and
**  freeing the associated memory
**
******************************************************************************/

/* @func aj3dVectorDel *******************************************************
**
** Default destructor for Ajax 3-D Vectors.
**
** If the given pointer is NULL, or a NULL pointer, simply returns.
**
** @param  [d] pthis [AjP3dVector*] Pointer to the 3-D vector to be deleted.
**         The pointer is always deleted.
** @return [void]
** @category delete [AjP3dVector] default destructor
** @@
******************************************************************************/

void aj3dVectorDel(AjP3dVector* pthis)
{
    AjP3dVector thys = NULL;

    thys = pthis ? *pthis :0;

    if(!pthis)
	return;

    if(!*pthis)
	return;

    thys->x = 0.0;
    thys->y = 0.0;
    thys->z = 0.0;
    AJFREE(thys);
    *pthis = NULL;

    return;
}




/* @section 3D Vector Functions ********************************************
**
** General functions
**
******************************************************************************/

/* @func aj3dVectorCrossProduct **********************************************
**
** calculates the cross product of two 3D vectors, that is their "torque"
**
** @param [r] ajp3dVectorFirst [const AjP3dVector] first 3D vector
** @param [r] ajp3dVectorSecond [const AjP3dVector] second 3D vector
** @param [w] ajp3dVectorCrossProduct [AjP3dVector] 3D vector to contain
**            cross product
** @return [void]
** @category use [AjP3dVector] return cross product of two
**                vectors
** @@
******************************************************************************/

void aj3dVectorCrossProduct(const AjP3dVector ajp3dVectorFirst,
			    const AjP3dVector ajp3dVectorSecond,
			    AjP3dVector ajp3dVectorCrossProduct)
{
    float fXOfCrossProduct;
    float fYOfCrossProduct;
    float fZOfCrossProduct;

    /* compute cross product */
    fXOfCrossProduct  = ajp3dVectorFirst->y * ajp3dVectorSecond->z;
    fXOfCrossProduct -= ajp3dVectorFirst->z * ajp3dVectorSecond->y;

    ajp3dVectorCrossProduct->x = fXOfCrossProduct;

    fYOfCrossProduct  = ajp3dVectorFirst->z * ajp3dVectorSecond->x;
    fYOfCrossProduct -= ajp3dVectorFirst->x * ajp3dVectorSecond->z;

    ajp3dVectorCrossProduct->y = fYOfCrossProduct;

    fZOfCrossProduct  = ajp3dVectorFirst->x * ajp3dVectorSecond->y;
    fZOfCrossProduct -= ajp3dVectorFirst->y * ajp3dVectorSecond->x;

    ajp3dVectorCrossProduct->z = fZOfCrossProduct;

    return;
}




/* @func aj3dVectorBetweenPoints *********************************************
**
** Calculates the vector from one point in space (start) to another (end)
**
** @param [u] ajp3dVectorBetweenPoints [AjP3dVector] vector from
**                                                         start to end
** @param [r] fStartX [float] X coordinate of start
** @param [r] fStartY [float] Y coordinate of start
** @param [r] fStartZ [float] Z coordinate of start
** @param [r] fEndX   [float] X coordinate of end
** @param [r] fEndY   [float] Y coordinate of end
** @param [r] fEndZ   [float] Z coordinate of end
**
** @return [void]
** @category modify [AjP3dVector] return vector between two
**                points
** @@
******************************************************************************/

void aj3dVectorBetweenPoints(AjP3dVector ajp3dVectorBetweenPoints,
			     float fStartX, float fStartY, float fStartZ,
			     float fEndX, float fEndY, float fEndZ)
{
    /* compute vector between points */
    ajp3dVectorBetweenPoints->x = fEndX - fStartX;
    ajp3dVectorBetweenPoints->y = fEndY - fStartY;
    ajp3dVectorBetweenPoints->z = fEndZ - fStartZ;

    return;
}




/* @func aj3dVectorLength ****************************************************
**
** calculates the magnitude of a vector
**
** @param [r] ajp3dVectorToBeSized [const AjP3dVector] vector to be sized
**
** @return fVectorLength [float] length of vector to be sized
** @category cast [AjP3dVector] return length of vector
** @@
******************************************************************************/

float aj3dVectorLength(const AjP3dVector ajp3dVectorToBeSized)
{
    float fSquareOfVectorLength;
    float fVectorLength;

    /* compute vector length */
    fSquareOfVectorLength = ajp3dVectorToBeSized->x *
	ajp3dVectorToBeSized->x;

    fSquareOfVectorLength += ajp3dVectorToBeSized->y *
	ajp3dVectorToBeSized->y;

    fSquareOfVectorLength += ajp3dVectorToBeSized->z *
	ajp3dVectorToBeSized->z;

    fVectorLength = (float)sqrt((double)fSquareOfVectorLength);

    return fVectorLength;
}




/* @func aj3dVectorAngle ******************************************************
**
** cCalculates the angle between two vectors
**
** method adapted from vmd XXXX INSERT CREDIT/REFERENCE HERE
**
** @param [r] ajp3dVectorFirst [const AjP3dVector] first vector
** @param [r] ajp3dVectorSecond [const AjP3dVector] second vector
**
** @return [float] angle between vectors in degrees
** @category use [AjP3dVector] return angle between two
**                vectors
** @@
******************************************************************************/

float aj3dVectorAngle(const AjP3dVector ajp3dVectorFirst,
		      const AjP3dVector ajp3dVectorSecond)
{
    float fLengthOfFirstVector;
    float fLengthOfSecondVector;
    float fLengthOfCrossProduct;
    
    float fDotProduct;
    float fVectorAngleInRadians;
    float fVectorAngleInDegrees;

    AjP3dVector ajp3dVectorCrossProduct=NULL;

    fLengthOfFirstVector    = aj3dVectorLength(ajp3dVectorFirst);
    fLengthOfSecondVector   = aj3dVectorLength(ajp3dVectorSecond);

    if((fLengthOfFirstVector < 0.0001) || (fLengthOfSecondVector < 0.0001))
    {
	fVectorAngleInDegrees = 180;
    }
    else
    {
	ajp3dVectorCrossProduct  = aj3dVectorNew();
	/* compute vector angle */
        aj3dVectorCrossProduct(ajp3dVectorFirst, ajp3dVectorSecond, ajp3dVectorCrossProduct);
	fDotProduct             = aj3dVectorDotProduct(ajp3dVectorFirst, ajp3dVectorSecond);
	fLengthOfCrossProduct   = aj3dVectorLength(ajp3dVectorCrossProduct);
	/* return the arctangent in the range -pi to +pi */
	fVectorAngleInRadians   = (float)atan2((double)fLengthOfCrossProduct, (double)fDotProduct);
	fVectorAngleInDegrees   = ajRadToDeg(fVectorAngleInRadians);
    }
    return( fVectorAngleInDegrees );
}




/* @func aj3dVectorDihedralAngle ********************************************
**
** calculates the angle from the plane perpendicular to A x B to the plane
**  perpendicular to B x C (where A, B and C are vectors)
**
** @param [r] ajp3dVectorA [const AjP3dVector] Vector A
** @param [r] ajp3dVectorB [const AjP3dVector] Vector B
** @param [r] ajp3dVectorC [const AjP3dVector] Vector C 
**
** @return [float] dihedral angle
** @category use [AjP3dVector] return angle between two
**                planes
** @@
******************************************************************************/

float aj3dVectorDihedralAngle(const AjP3dVector ajp3dVectorA,
			      const AjP3dVector ajp3dVectorB,
			      const AjP3dVector ajp3dVectorC)
{ 
    float fDihedralAngle;
    float fNumerator;
    float fDenominator;
    float fBterm;
    float fSignCoefficient = 1.0;

    AjP3dVector ajp3dVectorTorqueFirst    = NULL;
    AjP3dVector ajp3dVectorTorqueSecond   = NULL;
    AjP3dVector ajp3dVectorTorqueThird    = NULL;
    AjP3dVector ajp3dVectorTorqueCombined = NULL;

    ajp3dVectorTorqueFirst     = aj3dVectorNew();
    ajp3dVectorTorqueSecond    = aj3dVectorNew();
    ajp3dVectorTorqueThird     = aj3dVectorNew();
    ajp3dVectorTorqueCombined  = aj3dVectorNew();

    aj3dVectorCrossProduct(ajp3dVectorA, ajp3dVectorB,
			   ajp3dVectorTorqueFirst);
    aj3dVectorCrossProduct(ajp3dVectorB, ajp3dVectorC,
			   ajp3dVectorTorqueSecond);
    fNumerator = aj3dVectorDotProduct(ajp3dVectorTorqueFirst,
					 ajp3dVectorTorqueSecond);
    fDenominator = aj3dVectorLength(ajp3dVectorTorqueFirst) * aj3dVectorLength(ajp3dVectorTorqueSecond);
    aj3dVectorCrossProduct(ajp3dVectorTorqueFirst, ajp3dVectorTorqueSecond, ajp3dVectorTorqueCombined);
    
    fBterm = fNumerator / fDenominator;
    fDihedralAngle = ajRadToDeg( (float)acos((double)fBterm) );

    /* get sign of angle of rotation */
    if( ( aj3dVectorDotProduct(ajp3dVectorB, ajp3dVectorTorqueCombined) ) < 0.0 )
    {
	fSignCoefficient = -1.0;
	
    }

    aj3dVectorDel(&ajp3dVectorTorqueFirst);
    aj3dVectorDel(&ajp3dVectorTorqueSecond);
    aj3dVectorDel(&ajp3dVectorTorqueThird);
    aj3dVectorDel(&ajp3dVectorTorqueCombined);

    return (fSignCoefficient * fDihedralAngle);
}




/* @func aj3dVectorDotProduct ***********************************************
**
** calculates the dot product of two 3D vectors, that is their summed common
**  scalar magnitude
**
** @param [r] ajp3dVectorFirst [const AjP3dVector] first vector
** @param [r] ajp3dVectorSecond [const AjP3dVector] second vector
**
** @return [float] dot product of first and second vectors
** @category use [AjP3dVector] return dot product of two
**                vectors
** @@
******************************************************************************/

float aj3dVectorDotProduct(const AjP3dVector ajp3dVectorFirst,
			   const AjP3dVector ajp3dVectorSecond)
{
    float fDotProduct;

    /* compute dot product */
    fDotProduct  = ajp3dVectorFirst->x * ajp3dVectorSecond->x;
    fDotProduct += ajp3dVectorFirst->y * ajp3dVectorSecond->y;
    fDotProduct += ajp3dVectorFirst->z * ajp3dVectorSecond->z;

    return(fDotProduct);
}




/* @func aj3dVectorSum ********************************************************
**
** calculates the dot product of two 3D vectors, that is their summed common
**  "scalar magnitude"
**
** @param [r] ajp3dVectorFirst [const AjP3dVector] first vector
** @param [r] ajp3dVectorSecond [const AjP3dVector] second vector
** @param [w] ajp3dVectorSum [AjP3dVector] sum of first and second vectors
** @return [void]
** @category use [AjP3dVector] return sum of two vectors
** @@
******************************************************************************/

void aj3dVectorSum(const AjP3dVector ajp3dVectorFirst,
		   const AjP3dVector ajp3dVectorSecond,
		   AjP3dVector ajp3dVectorSum)
{
    /* compute sum of vectors by adding individual components */
    ajp3dVectorSum->x = ajp3dVectorFirst->x + ajp3dVectorSecond->x;
    ajp3dVectorSum->y = ajp3dVectorFirst->y + ajp3dVectorSecond->y;
    ajp3dVectorSum->z = ajp3dVectorFirst->z + ajp3dVectorSecond->z;

    return;
}
