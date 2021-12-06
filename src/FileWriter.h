#pragma once
using namespace std;
#include "Agent.h"
#include <iostream>
class FileWriter
{

    public:
        void OpenFile(string fileName);
        void CloseFile();
        int Write(Agent* agents, long startingId);

    private:
        string _fileName;

};