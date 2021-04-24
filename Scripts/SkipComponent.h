
#include "component.h"
#include <DirectXMath.h>

//@Serialize()
struct SpeedTestComponent : public IComponent
{
    float Speed; 
    float Acceleration; 
    bool IsActive;
    unsigned int Age;
    DirectX::XMFLOAT3 Velocity;
    M_INT(Test)
    MSystemType(SystemType::Core)
};