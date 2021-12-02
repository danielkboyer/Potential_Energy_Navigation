using namespace std;
#include "Point.h"
class Map
{

    private:
        Point **points;
    public:
        Map();
        void SetPoint(int x, int y, int z);
        Point GetPoint(int x, int y);
        
};