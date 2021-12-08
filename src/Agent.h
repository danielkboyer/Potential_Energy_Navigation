#pragma once
using namespace std;
class Agent
{

    public:
        Agent(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,float percentage,bool pruned);
        Agent();
        Agent(Agent& agent);
        void Set(float direction, float positionX, float positionY, float height, float velocity, float time, int Id, int parentId,float percentage,bool pruned = false);
        float direction;
        float positionX;
        float positionY;
        float height;
        float velocity;
        float percentage;
        //in seconds
        float time;
        int Id;
        int parentId;
        bool pruned;

        float DistanceFrom(float x, float y);
        float Energy(float gravity, float friction);
};