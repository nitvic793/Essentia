
#include "component.h"

class TestSystem: public ISystem
{

};

/*
 * Testing component
 */
struct SpeedComponent : public IComponent
{
    float Speed; 
    float Acceleration; 
    M_INT(Test)
    MSystemType(SystemType::Core)
};

//@Skip()
struct SkipComponent : public IComponent
{
    float Speed; 
    float Acceleration; 
};


struct NonSkipComponent : public IComponent
{
    unsigned int TestField;
};