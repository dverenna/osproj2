#include <random>
#include <queue>
#include <tuple>
#include <vector>
#include "Process.h"

void simulate(int scenario);

int main(){
  int scenario = 1;
  printf("Scheduling Simulator (Enter -1 to return to method select)\n1. Scenario 1: Identical Speed and Memory\n2. Scenario 2: Identical Speed and Varying Memory\n3. Scenario 3: Varying Speed and Identical Memory\n4. Scenario 4: Identical Speed and Memory, Sequential Arrival\n");
  scanf("%d", &scenario);
  while (scenario != -1){
    if (scenario < 1 || scenario > 5) {
      printf("Invalid Choice\n");
    }
    else {
      simulate(scenario);
    }
    printf("Scheduling Simulator (Enter -1 to return to method select)\n1. Scenario 1: Identical Speed and Memory\n2. Scenario 2: Identical Speed and Varying Memory\n3. Scenario 3: Varying Speed and Identical Memory\n4. Scenario 4: Identical Speed and Memory, Sequential Arrival\n");
    scanf("%d", &scenario);
  }
}

void simulate(int scenario){
  int remMem = 9999;
  queue<Process> readyQueue;
  queue<Process> resultQueue;
  int procCount = 0;

  mt19937::result_type seed;
  printf("Enter the desired seed for the RNG\n");
  cin >> seed;

  srand(seed);
  for (int i = 0; i < 39; i++){
      Process temp = Process(rand(), i, remMem/8);
      remMem -= temp.getMem();
      readyQueue.push(temp);
  }
  Process temp = Process(rand(), 40, (remMem <= 0 ? 1 : remMem));
  temp.setMem(remMem);
  readyQueue.push(temp);

  while(!readyQueue.empty()){
    Process temp = readyQueue.front();
    printf("%s\n", temp.toString().c_str());
    readyQueue.pop();
  }
}
