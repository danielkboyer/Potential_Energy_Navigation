#include <fstream>
#include "Map.h"
#include "Point.h"
#include <math.h>  
using namespace std;



Map::Map(){

}

float Map::GetHeight(int x, int y){
    return *(new float(_points[y][x].height));
}
//takes a float x and y and returns the points surrounding that point
//the returned points are clockwise, dL, uL, uR, dR
//returns if any of the points or OOB
float Map::GetHeight(float x, float y){
    int startX = floor(x/_pointDistance);
    int startY = floor(y/_pointDistance);
    //printf("StartX: %d , StartY: %d",startX,startY);
    if(startX < 0)
        return NAN;
    if(startX >= _width)
        return NAN;
    if(startY < 0)
        return NAN;
    if(startY >= _height)
        return NAN;
    float xPoint = x - startX;
    float yPoint = y - startY;
    return (_pointDistance - xPoint)*(_pointDistance - yPoint)*(_points[startY][startX].height) + 
            (_pointDistance)*(_pointDistance - yPoint)*(_points[startY][startX+1].height) +
            (_pointDistance - xPoint)*(_pointDistance)*(_points[startY-1][startX].height) +
            (_pointDistance)*(_pointDistance)*(_points[startY-1][startX+1].height);

    
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
        printf("Point Distance %f\n",_pointDistance);
        //Get width first
        std::getline(file, line);
        _width = stoi(line);
        printf("Map Width %d\n",_width);
        //Get height next
        std::getline(file, line);
        _height = stoi(line);
        _points = new Point*[_height];
        printf("Map height %d\n",_height);
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
            //printf("Creating point at (%d,%d)",currentX,currentY);
            _points[currentY][currentX] = Point(currentX,currentY,z);
            
            currentX++;
    }
    file.close();
}
else{
    printf("Could not find file %s\n",fileName);
}
    
}