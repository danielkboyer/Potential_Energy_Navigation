#pragma once
#include "Util.h"
class CPU_Util: public Util
{
    public:
        Agent AgentStep(Agent in,  float newDirection, Properties properties, Map map);
        void CheckPrune(Agent* out, Properties properties, Stat stat);
        void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat* out, long numberAgents, long agentsToPrune);

    private:
        Agent AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
        Agent AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
};