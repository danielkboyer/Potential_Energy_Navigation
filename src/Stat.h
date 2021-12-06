#pragma once
using namespace std;
#include <iostream>
class Stat
{

    public:
        Stat(float d_avg, float E_avg, float offset);
        Stat();
        float d_avg;
        float E_avg;
        float offset;

        
};