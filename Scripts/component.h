/****
 * Generated file. DO NOT MODIFY  
 */

#define M_INT(a) int a;
#define MSystemType(type) static constexpr SystemType sysType=type;

/*
 * base component
 */
struct IComponent
{
};

class ISystem
{
};

typedef enum SystemType
{
    Core,
    Game
} SystemType;