#pragma once
#include "Util.h"
class GPU_Util: public Util
{
    public:
        
        void Prune(Agent* agents,int count,Properties properties, Stat stat);
        void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat out, long numberAgents, long agentsToPrune);
        void StepAll(Agent* in,int inCount, Agent* out, int outCount, Properties properties, Map map);

    private:
        Agent AgentStep(Agent in,  float newDirection, Properties properties, Map map);
        void CheckPrune(Agent out, Properties properties, Stat stat);
        Agent AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
        Agent AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
};