#pragma once
#include "Util.h"
class MPI_Util: public Util
{
    public:
        
        void Prune(Agent* agents,Agent* out,long count, long amountToPrune);
        void StepAll(Agent* in,int inCount, Agent* out, int outCount, Properties properties, Map map);

    private:
        Agent AgentStep(Agent in,  float newDirection, Properties properties, Map map);
        Agent AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
        Agent AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
};