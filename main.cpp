//#include <SFML/Graphics.hpp>
#include <thread>
#include <fstream>
#include <string>
#include <sstream> 
#include <iostream>
#include <random>
#include <chrono>
#include "Predator.h"
#include "Prey.h"


const int DWW = 1000; // display window width, in pixels.
const int DWH = 1000; //display window height, in pixels.
const int WSL = 10; //size of both world dimensions, in units-area.
const int WORLDS = 10; //each thread runs a world


//using namespace sf;
using namespace std;

void displayBestOfGen(int generation) {
    /* RenderWindow window(VideoMode(DWW, DWH), "SFML works!");

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
        }

        window.clear(Color(255, 50, 150, 255)); //Clear tem de ser chamado ap�s cada update ao renderer, sn est� a desenhar por cima do anterior em vez de o substituir. 
        window.display();
    } */
}

void mutateNeuralNet(vector<vector<vector<double>>> *weights) {
    unsigned WorldSeed = chrono::system_clock::now().time_since_epoch().count();
    mt19937_64 generator(WorldSeed);
    uniform_int_distribution<int> distribution(0, 100);
    uniform_real_distribution<double> vdistribution(-0.10, 0.10);
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
    }
}
void generateInitialGeneticMemory(vector<double> *world) {
    ofstream geneticMemory("neuralNetValues.txt", fstream::app);
    geneticMemory << "|";
    for (int n = 0; n < 4; n++) {
        for (int i = 0; i < (*world).size() + 2; i++) {
            geneticMemory << "0.00-";
        }
        geneticMemory << "|";
    }
    geneticMemory << "|" << endl;
    geneticMemory.close();
}

void runWorld(vector<double> *world) {
    vector<int> predatorBodyCoords;
    predatorBodyCoords.push_back(34);
    predatorBodyCoords.push_back(35);
    predatorBodyCoords.push_back(44);
    predatorBodyCoords.push_back(45);
    (*world)[34, 35] = 1.0;
    (*world)[44, 45] = 1.0;
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
            if (c != ("|")[0] && c != ("\n")[0] && c != ("-")[0]) {
                stringstream converter(data.substr(i, 4));
                double converted;
                converter >> converted;
                //cout << converted << endl;
                i += 3;
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
    cout << weights.size() << endl;
    cout << weights[0].size() << endl;
    cout << weights[0][0].size() << endl;
    /*for (int i = 0; i < weights[0][0].size(); i++) {
        cout << weights[0][0][i] << endl;
    }*/
    Prey prey1(preyBodyCoords, weights);
    (*world)[18, 19] = 1.0;
    preyBodyCoords[0] += 50;
    preyBodyCoords[1] += 50;
    Prey prey2(preyBodyCoords, weights);
    (*world)[68, 69] = 1.0;
    preys.push_back(prey1);
    preys.push_back(prey2);
    vector<double> input1 = (*world);
    input1.insert(input1.end(), prey1.bodyCoords.begin(), prey1.bodyCoords.end());
    vector<double> input2 = (*world);
    input2.insert(input2.end(), prey2.bodyCoords.begin(), prey2.bodyCoords.end());
    vector<double> output1 = prey1.neuralNet(input1);
    vector<double> output2 = prey1.neuralNet(input2);
    cout << output1.size() << endl;
    cout << output1[0] << endl;
    cout << output1[1] << endl;
    cout << output1[2] << endl;
    cout << output1[3] << endl;
    ofstream geneticData;
    geneticData.open("neuralNetValues.txt", fstream::app);
    geneticData.close();
}

int main() {
    int ua = 100; //size of each unit-area side, in pixels (square this number to get unit-area in pixels).
    vector<vector<double>> worlds;
    for(int w = 0; w < WORLDS; w++) {
        vector<double> world(WSL * WSL, 0.0); //each world is represented by a single vector despite technically being a m * n matrix. The first n elements represent all the columns of the first row, and so on and so on.
        worlds.push_back(world);
    }
    //generateInitialGeneticMemory(&worlds[0]);
    runWorld(&worlds[0]);
	return 0;
}