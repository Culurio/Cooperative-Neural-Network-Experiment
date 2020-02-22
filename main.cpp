#include <SFML/Graphics.hpp>
#include <thread>
#include <fstream>
#include <string>
#include <sstream> 
#include <iostream>
#include <random>
#include <chrono>
#include "Predator.h"
#include "Prey.h"
#include <Windows.h>


const int DWW = 1000; // display window width, in pixels.
const int DWH = 1000; //display window height, in pixels.
const int WSL = 10; //size of both world dimensions, in units-area.
const int WORLDS = 10; //each thread runs a world


using namespace sf;
using namespace std;

bool insideBounds(vector<int> coords) {
    if (coords[0] < 0 || coords[0] > 9) {
        return false;
    }
    if (coords[1] < 0 || coords[1] > 9) {
        return false;
    }
    return true;
}

vector<int> convertToXY(int coord) {
    vector<int> xycoords;
    int x;
    int y;
    if (coord / 10 > 0) {
        x = ((double(coord) / 10 - coord / 10) * 10);
        y = int(coord / 10);
    }
    else {
        x = coord;
        y = 0;
    }
    xycoords.push_back(x);
    xycoords.push_back(y);
    return xycoords;
}

void displayBestOfGen(int ua) {
    RenderWindow window(VideoMode(DWW, DWH), "display");

    int m = 0;
    ifstream movementData;
    movementData.open("movementData.txt");
    string data;
    while (window.isOpen()){
        cout << data[m] << endl;
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }
        if (!movementData.eof()) {
            getline(movementData, data);
        }
        if (m >= data.size()) {
            break;
        }
        if (data[m] == "|"[0]) {
           m++;
           window.display();
           window.clear(Color(0, 255, 255, 255));
           Sleep(1000);
        }
        else {
           if (data[m] == "+"[0]) {
               m++;
           }
           else {
               int numberSize = 1;
               if (data[m + 1] != "+"[0]) {
                  numberSize++;
               }
               stringstream converter(data.substr(m, numberSize));
               int converted;
               converter >> converted;
               vector<int> coordsXY = convertToXY(converted);
               RectangleShape bodyPart(Vector2f(ua, ua));
               bodyPart.setPosition(coordsXY[0] * ua, coordsXY[1] * ua);
               bodyPart.setFillColor(Color(0, 255, 0, 255));
               window.draw(bodyPart);
               m += numberSize;
           }
       }
    }
} 


void mutateNeuralNet(vector<vector<vector<double>>> *weights) {
    unsigned WorldSeed = chrono::system_clock::now().time_since_epoch().count();
    mt19937_64 generator(WorldSeed);
    uniform_int_distribution<int> distribution(0, 100);
    uniform_real_distribution<double> vdistribution(-0.10, 0.10);
    uniform_real_distribution<double> nlvdistribution(-1.00, 1.00);
    for (int l = 0; l < (*weights).size(); l++) { //percorrer camadas
        for (int n = 0; n < (*weights)[l].size(); n++) { //percorrer neuronios da camada
            if (distribution(generator) == 1 && l < (*weights).size() - 1) { //opurtunidade de muta��o para cria��o de novos neur�nios
                vector<double> newNeuron = (*weights)[l][n];
                (*weights)[l].push_back(newNeuron);
            }
            for (int w = 0; w < (*weights)[l][n].size(); w++) { // percorrer pesos do neuronio
                if (distribution(generator) == 1) { // opurtunidade de muta��o para alterar os pesos nos neur�nio 
                    (*weights)[l][n][w] += vdistribution(generator);
                }
            }
        }
        if (distribution(generator) == 1 && l == (*weights).size() - 1) { //opurtunidade de muta�ao de novas camadas
            vector<vector<double>> newLayer;
            vector<double> newLayerWeights;
            for (int n = 0; n < (*weights)[l].size(); n++) {
                for (int w = 0; w < (*weights)[l][n].size(); w++) {
                    newLayerWeights.push_back(nlvdistribution(generator));
                }
                newLayer.push_back(newLayerWeights);
                newLayerWeights.clear();
            }
            (*weights).push_back(newLayer);
        }
    }
}
void generateInitialGeneticMemory(vector<double> *world) {
    ofstream geneticMemory("neuralNetValues.txt");
    geneticMemory << "|";
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < (*world).size() + 2; i++) {
            geneticMemory << "0.00+";
        }
        geneticMemory << "|";
    }
    geneticMemory << "|" << endl;
    geneticMemory.close();
}

bool checkAdjacency(vector<int> *targetBodyParts, int seekerBodyPart) {
    for (int i = 0; i < (*targetBodyParts).size(); i++) {
        int d = abs((*targetBodyParts)[i] - seekerBodyPart);
        if (!insideBounds(convertToXY(d))) {
            continue;
        }
        switch (d) 
        {
            case 11:
                return true;
            case 10:
                return true;
            case 9:
                return true;
            case 1:
                return true;
            default:
                return false;
        }
    }
}

void runWorld(vector<double> *world, vector<int> *fitnessValues, int worldNumber, vector<vector<vector<vector<double>>>> *worldNeuralWeights, vector<vector<int>> *worldsMovementData) {
    vector<int> predatorBodyCoords;
    predatorBodyCoords.push_back(34);
    predatorBodyCoords.push_back(35);
    predatorBodyCoords.push_back(44);
    predatorBodyCoords.push_back(45);
    (*world)[34, 35] = 1.0;
    (*worldsMovementData)[worldNumber].push_back(34);
    (*worldsMovementData)[worldNumber].push_back(35);
    (*world)[44, 45] = 1.0;
    (*worldsMovementData)[worldNumber].push_back(44);
    (*worldsMovementData)[worldNumber].push_back(45);
    Predator predator(predatorBodyCoords);
    vector<Prey> preys;
    vector<int> preyBodyCoords;
    preyBodyCoords.push_back(18);
    preyBodyCoords.push_back(19);
    vector<vector<vector<double>>> weights;
    ifstream geneticMemory;
    string data;
    geneticMemory.open("neuralNetValues.txt");
    while (!geneticMemory.eof()) {
        vector<double> neuronWeights;
        vector<vector<double>> layerWeights;
        getline(geneticMemory, data);
        char c;
        for (int i = 0; i < data.size(); i++) {
            c = data[i];
            if (c != ("|")[0] && c != ("+")[0] && c != (" ")[0]) {
                int sizeNumber = 0;
                for (int si = i; si < 100000000; si++) {
                    //cout << si << endl;
                    if (data[si] == "+"[0]) { 
                        break;
                    }
                    sizeNumber++;
                }
                stringstream converter(data.substr(i, sizeNumber));
                double converted;
                converter >> converted;
                //cout << sizeNumber << "sizeNumber" << worldNumber << endl;
                //cout << converted << "converted" << worldNumber << endl;
                i += sizeNumber;
                neuronWeights.push_back(converted);
            }
            else {
		         if(c == ("|")[0]) {
                    if (neuronWeights.size() > 0) {
                        layerWeights.push_back(neuronWeights);
                        neuronWeights.clear();
                    }
                    if (data[i + 1] == c) {
                        weights.push_back(layerWeights);
                        layerWeights.clear();
                    }
                 }
            }
        }
    }
    geneticMemory.close();
    mutateNeuralNet(&weights);
    int size = weights.size();
    //cout << weights.size() << endl;
    //cout << weights[0].size() << endl;
    //cout << weights[0][0].size() << endl;
    Prey prey1(preyBodyCoords, weights);
    (*world)[18, 19] = 1.0;
    (*worldsMovementData)[worldNumber].push_back(18);
    (*worldsMovementData)[worldNumber].push_back(19);
    preyBodyCoords[0] += 50;
    preyBodyCoords[1] += 50;
    Prey prey2(preyBodyCoords, weights);
    (*world)[68, 69] = 1.0;
    (*worldsMovementData)[worldNumber].push_back(68);
    (*worldsMovementData)[worldNumber].push_back(69);
    preys.push_back(prey1);
    preys.push_back(prey2);
    bool flag1 = false;
    bool flag2 = false;
    bool touchingPrey = false;
    for (int t = 0; t < 20; t++) {
        (*worldsMovementData)[worldNumber].push_back(-1);
        vector<double> input1 = (*world);
        input1.insert(input1.end(), prey1.bodyCoords.begin(), prey1.bodyCoords.end());
        vector<double> input2 = (*world);
        input2.insert(input2.end(), prey2.bodyCoords.begin(), prey2.bodyCoords.end());
        vector<vector<double>> outputs;
        outputs.push_back(prey1.neuralNet(input1));
        outputs.push_back(prey2.neuralNet(input2));
        for (int osi = 0; osi < outputs.size(); osi++) {
            int toMove = 0;
            for (int oi = 0; oi < outputs[osi].size(); oi++) {
                if (outputs[osi][oi] > 0.50) {
                    switch (oi)
                    {
                        case 0:
                            toMove += -1;
                            break;
                        case 1:
                            toMove += 1;
                            break;
                        case 2:
                            toMove += -10;
                            break;
                        case 3:
                            toMove += 10;
                            break;
                        default:
                            break;
                    }
                }
            }
            if (osi == 0) {
                for (int bi = 0; bi < prey1.bodyCoords.size(); bi++) {  //Check to see if both preys are touching predator, so their roles may change.                    
                    if (!insideBounds(convertToXY(prey1.bodyCoords[bi] + toMove))) {
                        break;
                    }
                    (*world)[prey1.bodyCoords[bi]] = 0.0;
                    prey1.bodyCoords[bi] += toMove;
                    //cout << prey1.bodyCoords[bi] << "prey1" << endl;
                    (*world)[prey1.bodyCoords[bi]] = 1.0;
                    (*worldsMovementData)[worldNumber].push_back(prey1.bodyCoords[bi]);
                    if (!flag1) {
                        flag1 = checkAdjacency(&predator.bodyCoords, prey1.bodyCoords[bi]);
                    }
                }
            }
            else {
                for (int bi = 0; bi < prey2.bodyCoords.size(); bi++) {
                    if (!insideBounds(convertToXY(prey2.bodyCoords[bi] + toMove))) {
                        break;
                    }
                    (*world)[prey2.bodyCoords[bi]] = 0.0;
                    prey2.bodyCoords[bi] += toMove;
                    //cout << prey2.bodyCoords[bi] << "prey2" << endl;
                    (*world)[prey2.bodyCoords[bi]] = 1.0;
                    (*worldsMovementData)[worldNumber].push_back(prey2.bodyCoords[bi]);
                    if (!flag2) {
                        flag2 = checkAdjacency(&predator.bodyCoords, prey2.bodyCoords[bi]);
                    }
                }
            }
        }
        if (flag1 && flag2) {
            (*fitnessValues)[worldNumber] += 1000 / t;
            break;
        }
        int predatorMove = predator.chooseMove(preys);
        for (int bi = 0; bi < predator.bodyCoords.size(); bi++) {
            if (!insideBounds(convertToXY(predator.bodyCoords[bi] + predatorMove))) {
                break;
            }
            (*world)[predator.bodyCoords[bi]] = 0.0;
            predator.bodyCoords[bi] += predatorMove;
            //cout << predator.bodyCoords[bi] << "predator" << endl;
            (*world)[predator.bodyCoords[bi]] = 1.0;
            (*worldsMovementData)[worldNumber].push_back(predator.bodyCoords[bi]);
            if (!touchingPrey) {
                touchingPrey = checkAdjacency(&prey1.bodyCoords, predator.bodyCoords[bi]);
            }
        }
        if (touchingPrey) {
            (*fitnessValues)[worldNumber] -= 100 / t;
            break;
        }
    }
    (*worldNeuralWeights)[worldNumber] = weights;
}

int main() {
    
    //TODO: qualquer coisa bugada ou no movimento ou a passar este para o movementData, favor investigar.
    /* int ua = 100; //size of each unit-area side, in pixels (square this number to get unit-area in pixels).
    vector<vector<double>> worlds;
    vector<vector<int>> worldsMovementData(WORLDS);
    vector<int> fitnessScores;
    vector<vector<vector<vector<double>>>> worldNeuralWeights(WORLDS);
    for(int w = 0; w < WORLDS; w++) {
        vector<double> world(WSL * WSL, 0.0); //each world is represented by a single vector despite technically being a m * n matrix. The first n elements represent all the columns of the first row, and so on and so on.
        worlds.push_back(world);
        fitnessScores.push_back(0);
    }
    //generateInitialGeneticMemory(&worlds[0]);
    for (int w = 0; w < WORLDS; w++) {
        runWorld(&worlds[w], &fitnessScores, w, &worldNeuralWeights, &worldsMovementData);
    }
    int maxF = -10000;
    int maxFi = 0;
    for (int i = 0; i < fitnessScores.size(); i++) {
        if (fitnessScores[i] > maxF) {
            maxF = fitnessScores[i];
            maxFi = i;
        }
    }
    vector<vector<vector<double>>> nextGenNet = worldNeuralWeights[maxFi];
    ofstream geneticData;
    ofstream geneticHistory;
    geneticData.open("neuralNetValues.txt");
    geneticHistory.open("neuralNetHistoricalData.txt", ofstream::app);
    for (int l = 0; l < nextGenNet.size(); l++) {
        for (int n = 0; n < nextGenNet[l].size(); n++) {
            geneticData << "|";
            geneticHistory << "|";
            for (int w = 0; w < nextGenNet[l][n].size(); w++) {
                geneticData << nextGenNet[l][n][w] << "+";
                geneticHistory << nextGenNet[l][n][w] << "+";
            }
        }
        geneticData << "|";
        geneticHistory << "|";
    }
    geneticData << "|";
    geneticHistory << "|";
    geneticData << endl;
    geneticHistory << endl << endl;
    geneticData.close();
    geneticHistory.close();
    vector<int> bestNetMovementData = worldsMovementData[maxFi];
    ofstream movementData;
    movementData.open("movementData.txt");
    movementData << "|";
    for (int c = 0; c < bestNetMovementData.size(); c++) {
        if (bestNetMovementData[c] != -1) {
            movementData << bestNetMovementData[c];
            movementData << "+";
        }
        else {
            movementData << "|";
        }
    }
    movementData << "|";
    movementData.close();
    return 0; */
    displayBestOfGen(100);
}