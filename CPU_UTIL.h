#include "Agent.h"
#include "Map.h"
#include "Properties.h"
namespace CPU_UTIL
{
    void AgentStep(Agent in, Agent out, float newDirection, Properties properties, Map map);

    void AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map);
    void AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map);
}