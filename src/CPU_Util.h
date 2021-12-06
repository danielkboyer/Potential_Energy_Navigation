#pragma once
#include "Util.h"
class CPU_Util: public Util
{
    public:
        void AgentStep(Agent in, Agent out, float newDirection, Properties properties, Map map);
        void CheckPrune(Agent out, Properties properties);
    private:
        void AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
        void AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
};