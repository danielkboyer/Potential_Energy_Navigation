using namespace std;
#include "Point.h"
class Map
{

    private:
        Point **_points;
        int _height;
        int _width;
        float _pointDistance;
    public:
        Map();
        void ReadFile(string fileName);
        //takes a float x and y and returns the points surrounding that point
        void GetFourPoints(float x, float y, Point* fourOut);
        
};