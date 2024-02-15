#pragma once
#define _USE_MATH_DEFINES
#include "parasolid_kernel.h"
#include <cmath>
#include <math.h>

double PK_VECTOR_dot(const PK_VECTOR_t v1, const PK_VECTOR_t v2)
{
	return v1.coord[0] * v2.coord[0] + v1.coord[1] * v2.coord[1] + v1.coord[2] * v2.coord[2];
}

PK_VECTOR_t PK_VECTOR_cross(const PK_VECTOR_t v1, const PK_VECTOR_t v2)
{
	PK_VECTOR_t cross;
	cross.coord[0] = v1.coord[1] * v2.coord[2] - v1.coord[2] * v2.coord[1];
	cross.coord[1] = v1.coord[2] * v2.coord[0] - v1.coord[0] * v2.coord[2];
	cross.coord[2] = v1.coord[0] * v2.coord[1] - v1.coord[1] * v2.coord[0];

	return cross;
}

// compute angle and rotation axis between two vectors
double PK_VECTOR_angleDeg(const PK_VECTOR_t v1, const PK_VECTOR_t v2, PK_VECTOR_t & rotateAxis) {
    PK_LOGICAL_t is_equal = PK_LOGICAL_false;
    PK_VECTOR_is_equal(v1, v2, &is_equal);
    if (PK_LOGICAL_true == is_equal)
        return 0.0;

    PK_LOGICAL_t is_parallel = PK_LOGICAL_false;
    PK_VECTOR_is_parallel(v1, v2, &is_parallel);
    if (PK_LOGICAL_true == is_parallel)
        return M_PI;

    // compute angle
    double dot = PK_VECTOR_dot(v1, v2);

    double angleDeg = acos(dot);

    // consider rotation direction
    rotateAxis = PK_VECTOR_cross(v1, v2);
    PK_VECTOR_normalise(rotateAxis, &rotateAxis);

    return angleDeg;
}