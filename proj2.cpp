#include <random>
#include <queue>
#include <tuple>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <map>
#include "Process.h"

// Tge nodes that will make up the double linked list that constitutes our memory
// they contain an id, the starting and ending memory address (in KB) and a pointer
// to the previous and next node
struct Node {
  int id;
  int start;
  int end;
  Node* prev;
  Node* next;
};

// Front and rear sentinel nodes for our memory list
Node front;
Node rear;


void simulate(int scenario);
int my_malloc(int pid, int reqMem);
void my_free(int pid);

int main(){
  int scenario = 1;
  printf("Memory Simulator (Enter -1 to exit)\n1. Scenario 1: System Scheduled\n2. Scenario 2: Custom Next Fit 10MB\n3. Scenario 3: Custom Next Fit 5MB\n4. Scenario 4: Custom Next Fit 1MB\n");
  scanf("%d", &scenario);
  while (scenario != -1){
    if (scenario < 1 || scenario > 5) {
      printf("Invalid Choice\n");
    }
    else {
      simulate(scenario);
    }
    printf("Memory Simulator (Enter -1 to exit)\n1. Scenario 1: System Scheduled\n2. Scenario 2: Custom Next Fit 10MB\n3. Scenario 3: Custom Next Fit 5MB\n4. Scenario 4: Custom Next Fit 1MB\n");
    scanf("%d", &scenario);
  }
}

void simulate(int scenario){
  //Clear memory linked list from previous runs
  front.start = 0;
  front.end = 0;
  front.prev = NULL;
  front.next = &rear;
  rear.next = NULL;
  rear.prev = &front;
  front.id = -1;
  rear.id = -1;

  //Initialize necesssary variables such as the  memory size and procesor speed;
  int remMem = 9999, memSize;
  long long int pSpeed = 3000000000;
  long long int p1Count = 0,p2Count = 0, p3Count = 0,  p4Count = 0;
  bool p1Go = true, p2Go = true, p3Go = true, p4Go = true;
  int *memPoint1, *memPoint2, *memPoint3, *memPoint4;
  int myMem1, myMem2, myMem3, myMem4;

  // The ready queue and result queues are initialized and the amount of completed processes is initialized
  queue<Process> readyQueue;
  queue<Process> resultQueue;
  int procCount = 0;

  // Initialize the seed for the RNG
  mt19937::result_type seed;
  printf("Enter the desired seed for the RNG\n");
  cin >> seed;

  // Initialize the map that will be used to store processes that memory is too fragmented to currently accept
  map<int, bool> fragMap;

  // The tuples containing the Process object and the remaining time are initialized with a dummy that will
  // act as "Null" and signify when a processor is available for a new process
  Process dummy(-1,-1, -1);
  tuple<Process, long long int> p1 = make_tuple(dummy, -1);
  tuple<Process, long long int> p2 = make_tuple(dummy, -1);
  tuple<Process, long long int> p3 = make_tuple(dummy, -1);
  tuple<Process, long long int> p4 = make_tuple(dummy, -1);

  // Generate the processes, making sure that no processes takes up too large a chunk of the memory, and allocating
  // any remaining memory to the final process to make sure that all 10MB is allocated
  srand(seed);
  for (int i = 0; i < 38; i++){
      Process temp = Process(rand(), i, remMem/8);
      remMem -= temp.getMem();
      readyQueue.push(temp);
  }
  Process temp = Process(rand(), 39, (remMem <= 0 ? 1 : remMem + 1));
  temp.setMem(remMem);
  readyQueue.push(temp);

  // Setting the total memory size in KB so that the rear sentinel node of the linked list has the proper value for
  // each scenario
  if (scenario > 2)
    memSize = scenario == 3 ? 5000 : 1000;
  else
    memSize = 10000;
  rear.start = memSize;
  rear.end = memSize;

  // Start the timer
  auto start = chrono::high_resolution_clock::now();

  // While the 40 processes have not been completed each of the processors performs the following actions
  //  -Checks if the processor is currently set to "dummy" and thus waiting for a process to be put on.
  //   If it is set to dummy, if first checks if the process's total amount of memory required is larger than the amount
  //   of memory in the system for that scenario. Then, if it has a valid amount of memory it allocates the memory based on
  //   which of the scenarios is currently in progress.
  //   - If it is scenario 1, the memory is allocated using malloc() and the address that was allocated is then saved into the
  //     process object so it can print where it was in memory during it's execution.
  //   - If it is scenario 2, 3, or 4, the memory is allocated via my_malloc, which places it in a double-linked list
  //     - If there was not room for the process in the linked list due to fragmentation, its id is placed into the fragMap,
  //       to denote that it was swapped out once.
  //       - If the process has already been placed into the fragMap and appears again while the readyQueue is empty, it is
  //         decided to be skipped since the memory is currently too fragmented to let the process enter and there is nothing
  //         left to swap it out with
  //     - Once a process is swappped off, it is placed into a storage queue. A new process is taken off of the ready queue
  //       and tested if it has space to be allocated currently. This proces repeats until a process that can fit is placed
  ///      into the system. Once a process is found, the processes from the storage queue are placed back into the ready queue
  //   - Once a process is found that can fit into the linked list, the location where the process is being stored is loaded
  //     into the object so it is available for the toString() method
  //   - Finally, regardless of what allocated the memory the process is loaded into a tuple that also contains burst time
  //     so the execution can begin
  //  -Incremements the cycle counter by the processors speed, gets the current remaining time and the process object
  //   for the process that is currently on the CPU, and decrements the remaining time by the clock speed of the CPU.
  //   - If the process is not completed at that point, the tuple is updated with the new remaining time.
  //   - If the process is completed  at that point, the end time is then set to the cycle where the process actually
  //     ended, and the clock is rolled back to the final cycle of the process. The process is then put into the result
  //     queue, and the processor is set to having a dummy process so it knows it is ready for a new process. Additionally
  //     the memory is freed so it can be used by later processes.
  while(procCount != 39){
    if (get<1>(p1) == -1 && !readyQueue.empty()){
      Process temp = readyQueue.front();
      if (temp.getMem() > memSize){
        printf("P1: Process %d rejected\n", temp.getID());
        p1 = make_tuple(dummy, -1); // Add dummy back
        procCount++;
        readyQueue.pop();
      } else {
        if (scenario == 1){
          memPoint1 = (int*) malloc(temp.getMem()*1000);
          temp.setLocat(memPoint1);
        } else {
          myMem1 = my_malloc(temp.getID(), temp.getMem());
          if (myMem1 == -1){
            if (readyQueue.empty() || fragMap.find(temp.getID())->second){
              printf("P1: Process %d rejected\n", temp.getID());
              p1 = make_tuple(dummy, -1); // Add dummy back
              procCount++;
              readyQueue.pop();
            }
            fragMap.insert(pair<int, bool>(temp.getID(), true));
            queue<Process> storage;
            while (myMem1 == -1 && !readyQueue.empty()){
              storage.push(temp);
              readyQueue.pop();
              temp = readyQueue.front();
              myMem1 = my_malloc(temp.getID(), temp.getMem());
            }
            while (!storage.empty()) {
              readyQueue.push(storage.front());
              storage.pop();
            }
          }
          temp.setLocat(&myMem1);
        }
        p1 = make_tuple(temp, temp.getBurst());
        readyQueue.pop();
      }
    }
    else if (get<1>(p1) != -1){
      p1Count += pSpeed;
      long long int remTime = get<1>(p1);
      Process temp = get<0>(p1);
      remTime -= pSpeed;
      if (remTime <= 0){
        temp.setEnd(p1Count-remTime); // Set the end to when it actually ended
        p1Count -= remTime; // Roll clock back to when it actually ended
        if (scenario == 1)
          free(memPoint1);
        else
          my_free(temp.getID());
        resultQueue.push(temp); // Add it to resultQueue
        p1 = make_tuple(dummy, -1); // Add dummy back
        procCount++; // Increment
      } else {
        p1 = make_tuple(temp, remTime);
      }
    }

    if (get<1>(p2) == -1 && !readyQueue.empty()){
      Process temp = readyQueue.front();
      if (temp.getMem() > memSize){
        printf("P2: Process %d rejected\n", temp.getID());
        p1 = make_tuple(dummy, -1); // Add dummy back
        readyQueue.pop();
        procCount++;
      } else {
        if (scenario == 1){
          memPoint2 = (int*) malloc(temp.getMem()*1000);
          temp.setLocat(memPoint2);
        } else {
          myMem2 = my_malloc(temp.getID(), temp.getMem());
          if (myMem2 == -1){
            if (readyQueue.empty() || fragMap.find(temp.getID())->second){
              printf("P2: Process %d rejected\n", temp.getID());
              p2 = make_tuple(dummy, -1); // Add dummy back
              procCount++;
              readyQueue.pop();
            }
            fragMap.insert(pair<int, bool>(temp.getID(), true));
            queue<Process> storage;
            while (myMem1 == -1 && !readyQueue.empty()){
              storage.push(temp);
              readyQueue.pop();
              temp = readyQueue.front();
              myMem2 = my_malloc(temp.getID(), temp.getMem());
            }
            while (!storage.empty()) {
              readyQueue.push(storage.front());
              storage.pop();
            }
          }
          temp.setLocat(&myMem2);
        }
        p2 = make_tuple(temp, temp.getBurst());
        readyQueue.pop();
      }
    }
    else if (get<1>(p2) != -1){
      p2Count += pSpeed;
      long long int remTime = get<1>(p2);
      Process temp = get<0>(p2);
      remTime -= pSpeed;
      if (remTime <= 0){
        temp.setEnd(p2Count-remTime); //Set the end to when it actually ended
        p2Count -= remTime; //Roll clock back to when it actually ended
        resultQueue.push(temp); //Add it to resultQueue
        if (scenario == 1)
          free(memPoint2);
        else
          my_free(temp.getID());
        p2 = make_tuple(dummy, -1); //Add dummy back
        procCount++; //Increment
      } else {
        p2 = make_tuple(temp, remTime);
      }
    }

    if (get<1>(p3) == -1 && !readyQueue.empty()){
      Process temp = readyQueue.front();
      if (temp.getMem() > memSize){
        printf("P3: Process %d rejected\n", temp.getID());
        p1 = make_tuple(dummy, -1); // Add dummy back
        readyQueue.pop();
        procCount++;
      } else {
        if (scenario == 1){
          memPoint3 = (int*) malloc(temp.getMem()*1000);
          temp.setLocat(memPoint3);
        } else {
          myMem3 = my_malloc(temp.getID(), temp.getMem());
          if (myMem3 == -1){
            if (readyQueue.empty() || fragMap.find(temp.getID())->second){
              printf("P3: Process %d rejected\n", temp.getID());
              p3 = make_tuple(dummy, -1); // Add dummy back
              procCount++;
              readyQueue.pop();
            }
            fragMap.insert(pair<int, bool>(temp.getID(), true));
            queue<Process> storage;
            while (myMem1 == -1 && !readyQueue.empty()){
              storage.push(temp);
              readyQueue.pop();
              temp = readyQueue.front();
              myMem3 = my_malloc(temp.getID(), temp.getMem());
            }
            while (!storage.empty()) {
              readyQueue.push(storage.front());
              storage.pop();
            }
          }
          temp.setLocat(&myMem3);
        }
        p3 = make_tuple(temp, temp.getBurst());
        readyQueue.pop();
      }
    }
    else if (get<1>(p3) != -1) {
      p3Count += pSpeed;
      long long int remTime = get<1>(p3);
      Process temp = get<0>(p3);
      remTime -= pSpeed;
      if (remTime <= 0){
        temp.setEnd(p3Count-remTime); //Set the end to when it actually ended
        p3Count -= remTime; //Roll clock back to when it actually ended
        resultQueue.push(temp); //Add it to resultQueue
        if (scenario == 1)
          free(memPoint3);
        else
          my_free(temp.getID());
        p3 = make_tuple(dummy, -1); //Add dummy back
        procCount++; //Increment
      } else {
        p3 = make_tuple(temp, remTime);
      }
    }

    if (get<1>(p4) == -1 && !readyQueue.empty()){
      Process temp = readyQueue.front();
      if (temp.getMem() > memSize){
        printf("P4: Process %d rejected\n", temp.getID());
        p1 = make_tuple(dummy, -1); // Add dummy back
        readyQueue.pop();
        procCount++;
      } else {
        if (scenario == 1){
          memPoint4 = (int*) malloc(temp.getMem()*1000);
          temp.setLocat(memPoint4);
        } else {
          myMem4 = my_malloc(temp.getID(), temp.getMem());
          if (myMem4 == -1){
            if (readyQueue.empty() || fragMap.find(temp.getID())->second){
              printf("P4: Process %d rejected\n", temp.getID());
              p4 = make_tuple(dummy, -1); // Add dummy back
              procCount++;
              readyQueue.pop();
            }
            fragMap.insert(pair<int, bool>(temp.getID(), true));
            queue<Process> storage;
            while (myMem4 == -1 && !readyQueue.empty()){
              storage.push(temp);
              readyQueue.pop();
              temp = readyQueue.front();
              myMem4 = my_malloc(temp.getID(), temp.getMem());
            }
            while (!storage.empty()) {
              readyQueue.push(storage.front());
              storage.pop();
            }
          }
          temp.setLocat(&myMem4);
        }
        p4 = make_tuple(temp, temp.getBurst());
        readyQueue.pop();
      }
    }
    else if (get<1>(p4) != -1){
      p4Count += pSpeed;
      long long int remTime = get<1>(p4);
      Process temp = get<0>(p4);
      remTime -= pSpeed;
      if (remTime <= 0){
        temp.setEnd(p4Count-remTime); //Set the end to when it actually ended
        p4Count -= remTime; //Roll clock back to when it actually ended
        resultQueue.push(temp); //Add it to resultQueue
        if (scenario == 1)
          free(memPoint4);
        else
          my_free(temp.getID());
        p4 = make_tuple(dummy, -1); //Add dummy back
        procCount++; //Increment
      } else {
        p4 = make_tuple(temp, remTime);
      }
    }
  }

  // End the timer and print the total time in nanoseconds
  auto end = chrono::high_resolution_clock::now();
  cout << "Total Time: " << chrono::duration_cast<chrono::nanoseconds>(end-start).count() << "ns\n";

  // Once all 40 processes have been completed, the result queue is then emptied and it's contents are printed
  // in the order they complted
  while(!resultQueue.empty()){
    Process temp = resultQueue.front();
    printf("%s\n", temp.toString().c_str());
    resultQueue.pop();
  }
  printf("\n");
}

// Since we are using first-fit, my_malloc traverses the double linked list of nodes
// unit it finds a node where the gap between it's starting memory location and the
// previous node's ending memory location that is sufficiently large enough to fit
// the new process. At which point is performs a standard linked list insertion, making
// sure that the next process starts where the previous process ended and the new process
// ends reqMem units after it starts.
int my_malloc(int pid, int reqMem){
  Node* ptr = front.next;
  while(ptr != NULL){
    if(ptr->start - ptr->prev->end >= reqMem){
      Node* temp;
      temp = new Node();
      temp->id = pid;
      temp->start = ptr->prev->end;
      temp->end = temp->start + reqMem;

      temp->next = ptr;
      temp->prev = ptr->prev;

      ptr->prev->next = temp;
      ptr->prev = temp;

      return temp->start;
    }
    ptr = ptr->next;
  }
  return -1;
}

// Frees the memory by removing the node for the completed process
void my_free(int pid){
  Node* ptr = &front;
  while(ptr != NULL){
    if (ptr->id == pid){
      ptr->prev->next = ptr->next;
      ptr->next->prev = ptr->prev;
    }
    ptr = ptr->next;
  }
}
