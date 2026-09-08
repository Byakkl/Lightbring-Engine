#pragma once

#include "ecsStructures.h"

namespace ECS::Systems
{

    /// @brief Provides a default System for rendering ECS Camera Components
    class Renderer : public ECS::Systems::SystemBase
    {
    public:
        void Execute() override;
    private:
        //Query for finding Archetypes with Camera Components
        ECS::ArchetypeQuery cameraQuery;
        //Query for finding Archetypes with Render Components
        ECS::ArchetypeQuery renderQuery;
    };
}