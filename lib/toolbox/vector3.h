#pragma once

typedef struct {
    float x;
    float y;
    float z;
} Vector3;

static inline Vector3 vector3_lerp(Vector3 a, Vector3 b, float t) {
    Vector3 result;
    result.x = a.x + (b.x - a.x) * t;
    result.y = a.y + (b.y - a.y) * t;
    result.z = a.z + (b.z - a.z) * t;
    return result;
}
