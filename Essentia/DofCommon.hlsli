

static const float GOLDEN_ANGLE = 2.39996323;
static const float MAX_BLUR_SIZE = 10.0;
static const float RAD_SCALE = 0.5; // Smaller = nicer blur, larger = faster
static const float FOCAL_BUFFER = 50.f;

float2 ProjectionConstants(float nearZ, float farZ)
{
    float2 projectionConstants;
    projectionConstants.x = farZ / (farZ - nearZ);
    projectionConstants.y = (-farZ * nearZ) / (farZ - nearZ);
    return projectionConstants;
}

float LinearZ(float depth, float nearZ, float farZ)
{
    float2 projectionConstants = ProjectionConstants(nearZ, farZ);
    float linearZ = projectionConstants.y / (depth - projectionConstants.x);
    return linearZ;
}

// COC
float GetBlurSize(float depth, float focusPoint, float focusScale, float nearZ, float farZ)
{
    float linearZ = LinearZ(depth, nearZ, farZ);
    float coc = clamp((focusPoint - linearZ) * focusScale, -1.0, 1.0);
    return abs(coc) * MAX_BLUR_SIZE;
}