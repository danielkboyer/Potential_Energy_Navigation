#pragma once
#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "Stat.h"
class Util {
    public:
        virtual void Prune(Agent* agents,int count,Properties properties, Stat stat) = 0;
        virtual void StepAll(Agent* in,int inCount,Agent* out,int outCount, Properties properties, Map map) = 0;
        
        virtual void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat out, long numberAgents, long agentsToPrune) = 0;
    private:
        virtual Agent AgentStep(Agent in,float newDirection, Properties properties, Map map) = 0;
<<<<<<< HEAD
        virtual void CheckPrune(Agent* out, Properties properties, Stat stat) = 0;
        virtual void CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat* out, long numberAgents, long agentsToPrune) = 0;
   private:
=======
        virtual void CheckPrune(Agent out, Properties properties, Stat stat) = 0;
>>>>>>> a102b5fdaa90e9e0395f74c31aa897ef4c3730b8
        virtual Agent AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
        virtual Agent AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map) = 0;
  
};