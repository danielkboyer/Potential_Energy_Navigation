#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "CPU_Util.h"
//this will be done in serial
void CPU_Util::CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat out, long numberAgents, long agentsToPrune){
    
    // get list of random number to interate through the agents 
    int randArrayIDs[sampleRate]; // array of ID's of agents
    printf("\n randArrayIDs:");
    for(int i=0;i<sampleRate;i++){
        randArrayIDs[i]=rand()%numberAgents;  //Generate number between 0 to 99
        printf("  %i  ",randArrayIDs[i]);
    }

    // make arrays of agent energies and distances for averaging
    float randDistances[sampleRate];
    float randEnergies[sampleRate];
    for (int i=0;i<sampleRate;i++){
        randDistances[i] = agents[randArrayIDs[i]].DistanceFrom(properties.agentStartX,properties.agentStartY);
        randEnergies[i] = agents[randArrayIDs[i]].Energy(properties.gravity,properties.friction);
        printf("Rand Distances %d: %f\n",i,randDistances[i]);
        printf("randEnergies %d: %f\n",i,randEnergies[i]);
        printf("i, for agents %d: x  %f  y  %f\n",i,agents[randArrayIDs[i]].positionX, agents[randArrayIDs[i]].positionY);

    }
    // get average distance and average energy for each random ID
    for (int i=0;i<sampleRate;i++){
        //printf("Rand Distances %d: %f\n",i,out.d_avg);
        out.d_avg += randDistances[i]/(float(sampleRate));
        out.E_avg += randEnergies[i]/(float(sampleRate));
    }
    // get the normalized average
    float normalized[sampleRate];
    float avg_normalized = 0;
    for (int i=0;i<sampleRate;i++){
        //TODO: out.d_avg/out.d_avg equals 1 right?
        normalized[i] = sqrt(randDistances[i]*randDistances[i]/(out.d_avg*out.d_avg) + randEnergies[i]*randEnergies[i]/(out.E_avg*out.E_avg));
        avg_normalized +=normalized[i];
    }
    avg_normalized = avg_normalized/float(sampleRate);
    // calcualte the standard deviation
    float stdDeviation = 0;
    for (int i=0;i<sampleRate;i++){
        stdDeviation += (normalized[i] - avg_normalized)*(normalized[i] - avg_normalized);
    }
    stdDeviation = sqrt(stdDeviation/sampleRate);
    out.offset = avg_normalized + (-0.5 + float(agentsToPrune)/float(numberAgents))*5.0*stdDeviation + stdDeviation/10.0;
}
// this is called for all agents to see if they are pruned
void CPU_Util::CheckPrune(Agent out, Properties properties, Stat stat){
    if ((out.DistanceFrom(properties.agentStartX,properties.agentStartY)/stat.d_avg + out.Energy(properties.gravity, properties.friction)/stat.E_avg) - stat.offset <=0) {
        out.pruned = true;
        // also do we need a counter for the total number of points pruned?
        //prune_counter += 1:
    ///////////////// do we need to do something here to make the list/ directory that we discussed with adi
    }
}
//Must have a non null out agent
Agent CPU_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
    //printf("Before\n");
    Agent out;
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
     
    out = AgentTravel(in,out,newDirection,properties,map);
    //printf("After %f, %f, %f, %f\n",out.positionX,out.positionY,out.time,out.direction);
    out = AgentHeight(in,out,newDirection,properties,map);
    //printf("After Again\n");
    return out;
    
}

Agent CPU_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.time += properties.travelDistance/out.velocity;
    out.direction = newDirection;
    return out;
    
}
//must have out positionX and positionY populated
Agent CPU_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
    
    out.height = map.GetHeight(out.positionX,out.positionY);
    if((2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) < 0){
        printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
        printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
     
        printf("Less than zero\n");
        out.pruned = true;
    }
    

    out.velocity = sqrt(2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) * properties.friction;
    //printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
     
    return out;
}
