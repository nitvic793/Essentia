#include <iostream>

#include "component.h"


struct NonSkipComponent : public IComponent
{
    float Test;  
};

int main()
{
    std::cout << "Test";
    return 0;
}