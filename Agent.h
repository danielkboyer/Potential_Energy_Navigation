using namespace std;
class Agent
{

    private:
        //degrees
        float _direction;
        float _positionX;
        float _positionY;
        float _height;
        float _velocity;
        Agent *_parent;
    public:
        Agent(float direction, float positionX, float positionY, float height, float velocity, Agent *parent);
        
};