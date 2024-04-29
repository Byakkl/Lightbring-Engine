#pragma once

enum ComponentType{
    COMP_TRANSFORM,
    COMP_MATERIAL,
    COMP_MESH
};

class Component{
public:
    //Stores the type of component it is
    ComponentType type;
    //TODO: Determine if this will be used with a pre-defined render data or ignored in the case of something like Vulkan render batching with a recycled pool of descriptor sets
    //Tracks if any changes have been made to the component that need to be reflected in other systems
    bool isDirty;
};