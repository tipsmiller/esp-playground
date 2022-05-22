#include "MPU6050.h"

struct MPUValues {
    Quaternion q;           // [w, x, y, z]         quaternion container
    VectorFloat gravity;    // [x, y, z]            gravity vector
    float ypr[3];           // [yaw, pitch, roll]   ypr container
};

void setupI2C();
MPUValues readMPU();