#include "engine/render/math.h"

#include <math.h>
#include <stdio.h>

static int nearly_equal(float a, float b)
{
    return fabsf(a - b) < 0.0001f;
}

int main(void)
{
    nds_mat4 identity;
    nds_mat4 translation;
    nds_mat4 scale;
    nds_mat4 combined;
    nds_mat4 projection;

    nds_mat4_identity(&identity);
    if (!nearly_equal(identity.m[0], 1.0f) ||
        !nearly_equal(identity.m[5], 1.0f) ||
        !nearly_equal(identity.m[10], 1.0f) ||
        !nearly_equal(identity.m[15], 1.0f)) {
        fprintf(stderr, "identity matrix test failed\n");
        return 1;
    }

    nds_mat4_translate(&translation, 2.0f, 3.0f, 4.0f);
    nds_mat4_scale(&scale, 2.0f, 4.0f, 8.0f);
    nds_mat4_mul(&combined, &translation, &scale);

    if (!nearly_equal(combined.m[0], 2.0f) ||
        !nearly_equal(combined.m[5], 4.0f) ||
        !nearly_equal(combined.m[10], 8.0f) ||
        !nearly_equal(combined.m[12], 2.0f) ||
        !nearly_equal(combined.m[13], 3.0f) ||
        !nearly_equal(combined.m[14], 4.0f)) {
        fprintf(stderr, "matrix multiplication test failed\n");
        return 1;
    }

    nds_mat4_perspective(&projection, 70.0f, 4.0f / 3.0f, 0.1f, 1000.0f);
    if (!nearly_equal(projection.m[11], -1.0f) ||
        nearly_equal(projection.m[0], 0.0f) ||
        nearly_equal(projection.m[5], 0.0f)) {
        fprintf(stderr, "perspective matrix test failed\n");
        return 1;
    }

    printf("render math test passed\n");
    return 0;
}
