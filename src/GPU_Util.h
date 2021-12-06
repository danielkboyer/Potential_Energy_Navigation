#pragma once
#include "Util.h"
class GPU_Util: public Util
{
    public:
        void AgentStep(Agent in, Agent out, float newDirection, Properties properties, Map map);
        void CheckPrune(Agent out, Properties properties, Stat stat);
        void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat out, long numberAgents, long agentsToPrune);

    private:
        void AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
        void AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
};