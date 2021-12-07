#pragma once
#include "Agent.h"
#include "Map.h"
#include "Properties.h"
class Util {
    public:
        virtual void Prune(Agent* agents, Agent* out,long count, long amountToPrune) = 0;
        virtual void StepAll(Agent* in,int inCount,Agent* out,int outCount, Properties properties, Map map) = 0;

    private:
        virtual Agent AgentStep(Agent in,float newDirection, Properties properties, Map map) = 0;
        virtual Agent AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
        virtual Agent AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
  
};