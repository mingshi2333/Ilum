#include "E:/Workspace/Ilum/Source/Ilum/Test/SerializationTest.hpp"
#include <rttr/registration.h>

namespace Ilum_4316304629775190912
{
RTTR_REGISTRATION
{
    
    rttr::registration::class_<TestStruct>("TestStruct")
        .property("a", &TestStruct::a)
        .property("b", &TestStruct::b)
    
    
    ;
}


}                                