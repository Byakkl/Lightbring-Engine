#pragma once

namespace ECS::Systems
{
    class SystemBase
    {
    public:
        virtual void Execute() = 0;
    };
}