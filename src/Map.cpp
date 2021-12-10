#include <fstream>
#include "Map.h"
#include "Point.h"
#include <math.h>  
using namespace std;



Map::Map(){

}

float Map::GetPointDistance(){
    return _pointDistance;
}
float Map::GetHeight(int x, int y){
    return *(new float(points[y*_width+x].height));
}
//takes a float x and y and returns the points surrounding that point
//the returned points are clockwise, dL, uL, uR, dR
//returns if any of the points or OOB
float Map::GetHeight(float x, float y){
    int startX = floor(x/_pointDistance);
    int startY = floor(y/_pointDistance);
    //printf("StartX: %d , StartY: %d\n",startX,startY);
    if(startX < 0)
        return NAN;
    if(startX +1 >= _width)
        return NAN;
    if(startY -1 < 0)
        return NAN;
    if(startY >= _height)
        return NAN;
    //printf("x %f, y %f\n",x/_pointDistance,y/_pointDistance);
    float xPoint = x/_pointDistance - startX;
    float yPoint = y/_pointDistance - startY;
    //printf("yPoint %f, xPoint %f\n",yPoint,xPoint);
    return (_pointDistance - xPoint)*(_pointDistance - yPoint)*(points[startY*_width+startX].height) + 
            (_pointDistance)*(_pointDistance - yPoint)*(points[startY*_width+startX+1].height) +
            (_pointDistance - xPoint)*(_pointDistance)*(points[(startY-1)*_width+startX].height) +
            (_pointDistance)*(_pointDistance)*(points[(startY-1)*_width+startX+1].height);

    
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
        points = new Point[_height * _width];
        printf("Map height %d\n",_height);
        
        //because y will be incremented right away in the loop
        int currentIndex = 0;
        while (std::getline(file, line)) {
            
            float z = stof(line);
            //printf("Creating point at (%d,%d)",currentIndex%_width,currentIndex/_height);
            points[currentIndex] = Point();
            points[currentIndex].x = (int)(currentIndex%_width);
            points[currentIndex].y = (int)(currentIndex/_height);
            points[currentIndex].height = z;
            
            currentIndex++;
    }
    file.close();
}
else{
    printf("Could not find file %s\n",fileName);
}
    
}