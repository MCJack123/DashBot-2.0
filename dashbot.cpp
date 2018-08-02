#include <iostream>
#include "HAPIH.h"
#include <windows.h>
#include <cmath>
#include <vector>
#include <iterator>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <map>

struct genotype {
	std::vector<float> jumps;
	float fitness;
    float rfitness;
};

struct deathPoint {
    float x;
    int count;
};

genotype bestSpecies;
genotype species[50];
int retries = 0;
bool saving = false;
std::ifstream in;
std::ofstream out;
deathPoint lastWorstDeathPoint;

char * intToChar(float num) {
    char arr[sizeof(float)];
    memcpy(arr, &num, sizeof(float));
    return arr;
}

float charToInt(char * num) {
    float arr;
    memcpy(&arr, num, sizeof(float));
    return arr;
}

int main(int argc, const char * argv[]) {
    std::cout << "DashBot v2.0\nBased on Pizzabot-v4 by Pizzaroot\n";
    std::string savefile = "";
    std::default_random_engine rng(std::chrono::system_clock::now().time_since_epoch().count());
    if (argc > 1) {savefile = std::string(argv[1]); saving = true;}
    else std::cout << "Continuing without saving.\n";
    if (saving) {
        in.open(savefile.c_str(), std::ios::binary);
        if (in.is_open() && !in.eof()) {
            char buf[sizeof(float)];
            for (int i = 0; i < sizeof(float); i++) buf[i] = in.get();
            bestSpecies.fitness = charToInt(buf);
            for (int i = 0; i < sizeof(float); i++) buf[i] = in.get();
            bestSpecies.rfitness = charToInt(buf);
            while (!in.eof()) {
                for (int i = 0; i < sizeof(float); i++) buf[i] = in.get();
                bestSpecies.jumps.push_back(charToInt(buf));
            }
            in.close();
        } else {
            std::cout << "Creating new save file.\n";
            bestSpecies.fitness = 1.0;
            bestSpecies.rfitness = 0.0;
        }
        out.open(savefile.c_str(), std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            std::cout << "Could not open save file\n";
            return 1;
        }
    } else {
        bestSpecies.fitness = 100.0;
        bestSpecies.rfitness = 0.0;
    }
    HackIH GD;

	int widthGD = 600;
	int heightGD = 400;
	float sum;
	float xPos;
	float yPos;
    int generation = 0, speciesn = 0;
    float xMax = 0;
	float lastX = 0;
	float lastDeath = 0;
    int lastaction = 0;
    bool randing = true;
    lastWorstDeathPoint.x = 0;
    lastWorstDeathPoint.count = 0;

	GD.bind("GeometryDash.exe");
    std::cout << "generation " << generation << " species " << speciesn << "\n";
    
    while (true) {
        if (lastaction > 0) {
            Sleep(200);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            Sleep(100);
            lastaction = 0;
        }
        xPos = GD.Read<float>({ GD.BaseAddress , 0x3222D0 , 0x164, 0x224, 0x67C });
		yPos = GD.Read<float>({ GD.BaseAddress , 0x3222D0 , 0x164, 0x38C, 0xB4, 0x224, 0x680 });
        //if (lastX != xPos) std::cout << lastX << " " << xPos << "\n";
        if (lastX > xPos) {
            speciesn++;
            if (speciesn == 50) {
                speciesn = 0;
                generation++;
                float lastFitness = bestSpecies.fitness;
                std::map<float, int> deaths;
                int best;
                for (int i = 0; i < 50; i++) {
                    if (species[i].fitness > bestSpecies.fitness || (species[i].fitness >= bestSpecies.fitness - 10 && species[i].jumps.size() < bestSpecies.jumps.size())) {
                        bestSpecies = species[i]; 
                        best = i; 
                        species[i] = genotype();
                    }
                    std::cout << i << "\n";
                    if (species[i].jumps.size() > 0) {
                        if (deaths.find(species[i].jumps.back()) == deaths.end()) deaths.insert(std::make_pair(species[i].jumps.back(), 1));
                        else deaths[species[i].jumps.back()] += 1;
                    }
                }
                deathPoint worstDeathPoint;
                worstDeathPoint.x = 0;
                worstDeathPoint.count = 0;
                for (std::map<float, int>::iterator it = deaths.begin(); it != deaths.end(); it++) {
                    if (it->second > worstDeathPoint.count) {
                        worstDeathPoint.x = it->first;
                        worstDeathPoint.count = it->second;
                    }
                }
                std::cout << "most deaths at " << worstDeathPoint.x << " with " << worstDeathPoint.count << " deaths\n";
                if (worstDeathPoint.x >= lastWorstDeathPoint.x - 5 && worstDeathPoint.x <= lastWorstDeathPoint.x + 5 && worstDeathPoint.count >= 35 && lastWorstDeathPoint.count >= 35) {
                    std::cout << "too many deaths, stepping back\n";
                    bestSpecies.rfitness = worstDeathPoint.x - 100;
                    bestSpecies.fitness = worstDeathPoint.x;
                }
                lastWorstDeathPoint = worstDeathPoint;
                std::sort(bestSpecies.jumps.begin(), bestSpecies.jumps.end());
                std::cout << "best fitness is " << bestSpecies.fitness << "/" << bestSpecies.jumps.size() << " (" << best << ")\n";
                if (bestSpecies.fitness <= lastFitness && generation != 0) bestSpecies.jumps.resize(bestSpecies.jumps.size() - 1);
                bestSpecies.rfitness = bestSpecies.jumps[bestSpecies.jumps.size()-1] - 1;
                if (saving) {
                    out.seekp(0);
                    out.write(intToChar(bestSpecies.fitness), sizeof(float));
                    out.write(intToChar(bestSpecies.rfitness), sizeof(float));
                    for (float n : bestSpecies.jumps) out.write(intToChar(n), sizeof(float));
                    out.flush();
                }
            }
            std::cout << "generation " << generation << " species " << speciesn << "\n";
            lastX = xPos;
        } else if (xPos > lastX) {
            if (xPos < bestSpecies.rfitness) {
                std::cout << ""; // why is this needed?
                randing = false;
                for (float n : bestSpecies.jumps) if (floor(n) == floor(xPos) && floor(n) != floor(sum)) {sum = floor(n); lastaction = 1; species[speciesn].jumps.push_back(xPos);}
            } else {
                if (!randing) {
                    std::cout << "randing (chances of jump = 1/" << 125 * (1 / (((bestSpecies.fitness < species[speciesn].fitness && generation != 0 ? species[speciesn].fitness : bestSpecies.fitness) - bestSpecies.rfitness) / 100)) << ")\n";
                    randing = true;
                }
                int rnum = (rng()) % (int)ceil(125 * (1 / (((bestSpecies.fitness < species[speciesn].fitness && generation != 0 ? species[speciesn].fitness : bestSpecies.fitness) - bestSpecies.rfitness) / 100)));
                if (rnum == 1) {lastaction = 2; species[speciesn].jumps.push_back(xPos);}
                //std::cout << rnum << " ";
            }
            lastX = xPos;
        }
        if (lastaction > 0) {
            std::cout << "clicking\n";
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        }
        if (species[speciesn].fitness < xPos) species[speciesn].fitness = xPos;
    }
    if (saving) out.close();
    std::cerr << "Why did it break?\n";
    return 0;
}