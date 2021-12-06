#pragma once
#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "Stat.h"
class Util {
   public:
        virtual void AgentStep(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
        virtual void CheckPrune(Agent out, Properties properties, Stat stat) = 0;
        virtual void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat out, long numberAgents, long agentsToPrune) = 0;
   private:
        virtual void AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
        virtual void AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
  
};