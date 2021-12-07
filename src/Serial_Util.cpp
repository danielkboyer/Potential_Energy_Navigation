#include "Agent.h"
#include "Map.h"
#include "Properties.h"
#include "math.h"
#include "Util.h"
#include "Serial_Util.h"


void Serial_Util::StepAll(Agent* in, int inCount, Agent* out, int outCount, Properties properties, Map map){
    for(int x = 0;x<inCount;x++){
		int aIndex = x*properties.numberOfDirectionSpawn;
		for(int y = 0;y<properties.numberOfDirectionSpawn;y++){
			float newDirection = in[x].direction - properties.directionSpawnRadius/2 + properties.directionSpawnRadius/(properties.numberOfDirectionSpawn-1) * y;
			//a[aIndex+y] = Agent();
			out[aIndex+y] = AgentStep(in[x],newDirection,properties,map);
			//printf("Agent position %f,%f\n",a[aIndex+y].positionX,a[aIndex+y].positionY);
		}
	}
}
void Serial_Util::RandPrune(Agent* agents, long numberAgents, long agentsToPrune){
    // shuffle the array to get the points


    // combine the array 
    
    
    srand(time(NULL));
    const int N = numberAgents;
    long numbers [N];
    for (int x=0; x!=agentsToPrune;x++){
        long tmp = (rand()%numberAgents);
        bool isNotAdded = true;
        int i=0;
        while(i<x && isNotAdded) {
            if(numbers[i] == tmp) isNotAdded = false;
            i++;
        }
        while(isNotAdded)
           tmp = (rand() % numberAgents);
        numbers[x] = tmp;
    }
    for(int i=0;i<agentsToPrune;i++){
        agents[numbers[i]].pruned = true;
        //printf("ID pruned: %ld\n", x);
    }

    /// origional random 
    // srand (100);
    // long x = 0;
    // for(int i=0;i<agentsToPrune;i++){
    //     x = rand()%numberAgents;
    //     agents[x].pruned = true;
    //     //printf("ID pruned: %ld\n", x);
    // }
}
//this will be done in serial
void Serial_Util::CalcAvg(Agent* agents, Properties properties, long sampleRate, Stat* out, long numberAgents, long agentsToPrune){
    // get list of random number to interate through the agents 
    int randArrayIDs[sampleRate]; // array of ID's of agents
    //printf("\n randArrayIDs:");
    for(int i=0;i<sampleRate;i++){
        randArrayIDs[i]=rand()%numberAgents;  //Generate number between 0 to number agents
        //printf("  %i  ",randArrayIDs[i]);
    }
    // check for nans, make new list with non nans

    // make array of agent energies for averaging, but check for nans
    float randEnergies[sampleRate];
    long new_sample_rate = 0;
    for (int i=0;i<sampleRate;i++){
        randEnergies[i] = agents[randArrayIDs[i]].Energy(properties.gravity,properties.friction);
        // check for nan values
        if (isnan(randEnergies[i]) == false) {
            randArrayIDs[new_sample_rate] = randArrayIDs[i];
            randEnergies[new_sample_rate] = randEnergies[i];
            new_sample_rate++;
        }
    }
    // if none are nan then put the new sample rate back to the sample rate
    // new_sample_rate could also be zero if they are all nan
    if ((new_sample_rate==0.0) && (isnan(randEnergies[0])==false)) new_sample_rate=sampleRate;
    sampleRate = new_sample_rate;
    printf("new sample rate: %ld\n", new_sample_rate);
    float randDistances[sampleRate];
    for (int i=0;i<sampleRate;i++){
        randDistances[i] = 4*agents[randArrayIDs[i]].DistanceFrom(properties.agentStartX,properties.agentStartY);
        //printf("Rand Distances %d: %f\n",i,randDistances[i]);
        //printf("randEnergies %d: %f\n",i,randEnergies[i]);
        //printf("i, for agents %d: x  %f  y  %f\n",i,agents[randArrayIDs[i]].positionX, agents[randArrayIDs[i]].positionY);
    } 
    // get average distance and average energy for each random ID
    for (int i=0;i<sampleRate;i++){
        //printf("Rand Distances %d: %f\n",i,out.d_avg);
        out->d_avg += randDistances[i]/(float(sampleRate));
        out->E_avg += randEnergies[i]/(float(sampleRate));
    }
    // get the normalized average
    float normalized[sampleRate];
    float avg_normalized = 0;
    for (int i=0;i<sampleRate;i++){
        //TODO: out.d_avg/out.d_avg equals 1 right?
        normalized[i] = sqrt(randDistances[i]*randDistances[i]/(out->d_avg* out->d_avg) + randEnergies[i]*randEnergies[i]/(out->E_avg*out->E_avg));
        avg_normalized +=normalized[i];
    }
    avg_normalized = avg_normalized/float(sampleRate);
    // calcualte the standard deviation
    float stdDeviation = 0;
    for (int i=0;i<sampleRate;i++){
        stdDeviation += (normalized[i] - avg_normalized)*(normalized[i] - avg_normalized);
    }
    stdDeviation = sqrt(stdDeviation/sampleRate);
    printf("Standard Deviation %f\n",stdDeviation);
    out->offset = avg_normalized + (-0.5 + float(agentsToPrune)/float(numberAgents))*10.0*stdDeviation + stdDeviation/5.0;
}
// this is called for all agents to see if they are pruned
void Serial_Util::CheckPrune(Agent* out, Properties properties, Stat stat){
    if ((out->DistanceFrom(properties.agentStartX,properties.agentStartY)/stat.d_avg + out->Energy(properties.gravity, properties.friction)/stat.E_avg) - stat.offset <=0) {
        out->pruned = true;
        // also do we need a counter for the total number of points pruned?
        //prune_counter += 1:
    ///////////////// do we need to do something here to make the list/ directory that we discussed with adi
    }
}
void Serial_Util::Prune(Agent* agents, int count, Properties properties, Stat stat){
    for(int x = 0;x<count;x++){
        CheckPrune(&agents[x],properties,stat);
    }
}
//Must have a non null out agent
Agent Serial_Util::AgentStep(Agent in, float newDirection, Properties properties, Map map){
    //printf("Before\n");
    Agent out;
    out.pruned = false;
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
    
    out = AgentTravel(in,out,newDirection,properties,map);
    //printf("After %f, %f, %f, %f\n",out.positionX,out.positionY,out.time,out.direction);
    out = AgentHeight(in,out,newDirection,properties,map);
    //printf("After Again\n");
    return out;
}

Agent Serial_Util::AgentTravel(Agent in, Agent out, float newDirection, Properties properties, Map map){  
    out.positionX = in.positionX + cos(newDirection) * properties.travelDistance;
    out.positionY = in.positionY + sin(newDirection) * properties.travelDistance;
    //printf("Agent position %f,%f\n",out->positionX,out->positionY);
    out.direction = newDirection;
    return out;
    
}
//must have out positionX and positionY populated
Agent Serial_Util::AgentHeight(Agent in, Agent out, float newDirection, Properties properties, Map map){
    out.height = map.GetHeight(out.positionX,out.positionY);
    if(isnan(out.height) || (2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) < 0){
        //printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
        //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
        //printf("Less than zero\n");
        out.pruned = true;
        return out;
    }
    out.velocity = sqrt(2*properties.gravity*(in.height - out.height) + in.velocity*in.velocity) * properties.friction;
    out.time += properties.travelDistance/out.velocity;
    //printf("InHeight %f, InVelocity: %f\n",in.height,in.velocity);
    //printf("PositionX, %f, PositionY %f, Velocity %f, height %f, gravity %f, friciton %f\n",out.positionX,out.positionY,out.velocity,out.height,properties.gravity,properties.friction);
    return out;
}
