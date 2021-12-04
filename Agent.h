using namespace std;
class Agent
{

    public:
        Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int parentId,bool pruned);

        float direction;
        float positionX;
        float positionY;
        float height;
        float velocity;
        //in seconds
        float time;
        int parentId;
        bool pruned;

        float DistanceFrom(float x, float y);
};