#include <fstream>
#include "Map.h"
#include "Point.h"
#include <math.h>  
using namespace std;



Map::Map(){

}
//takes a float x and y and returns the points surrounding that point
//the returned points are clockwise, dL, uL, uR, d,R
//returns if any of the points or OOB
void Map::GetFourPoints(float x, float y, Point* fourOut){
    int startX = floor(x/_pointDistance);
    int startY = floor(y/_pointDistance);

    if(startX < 0)
        return;
    if(startX >= _width)
        return;
    if(startY < 0)
        return;
    if(startY >= _height)
        return;
    fourOut[0] = _points[startY][startX];
    fourOut[1] = _points[startY - 1][startX];
    fourOut[2] = _points[startY - 1][startX + 1];
    fourOut[3] = _points[startY][startX + 1];
}
//takes a file name and reads the map into memory
//must be in order of
//(Pont Distance)1.3
//(width)1000
//(height)1000
//(z values)10
//9
//....
void Map::ReadFile(string fileName){

    std::ifstream file(fileName);
    if (file.is_open()) {
        std::string line;

        std::getline(file,line);
        _pointDistance = stof(line);
        //Get width first
        std::getline(file, line);
        _width = stoi(line);
        
        //Get height next
        std::getline(file, line);
        _height = stoi(line);
        _points = new Point*[_height];
        
        int currentX = 0;
        //because y will be incremented right away in the loop
        int currentY = -1;
        while (std::getline(file, line)) {
            if(currentX%_width == 0){
                currentX = 0;
                currentY++;

                _points[currentY] = new Point[_width];

            }
            float z = stof(line);
            _points[currentY][currentX] = *new Point(currentX,currentY,z);
            
            currentX++;
    }
    file.close();
}
    
}