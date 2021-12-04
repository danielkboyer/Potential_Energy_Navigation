using namespace std;
class Properties
{

    public:
        Properties(float friction, float travelDistance,long maxAgentCount, float agentStartX, float agentStartY,float gravity = 9.81);

        float friction;
        float travelDistance;
        float gravity;
        long maxAgentCount;
        float agentStartX;
        float agentStartY;
};
