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

struct Node {
  int id;
  int start;
  int end;
  Node* prev;
  Node* next;
};

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
  //Clear list from previous runs
  front.start = 0;
  front.end = 0;
  front.prev = NULL;
  front.next = &rear;
  rear.next = NULL;
  rear.prev = &front;
  front.id = -1;
  rear.id = -1;

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

  mt19937::result_type seed;
  printf("Enter the desired seed for the RNG\n");
  cin >> seed;

  map<int, bool> fragMap;

  // The tuples containing the Process object and the remaining time are initialized with a dummy that will
  // act as "Null" and signify when a processor is available for a new process
  Process dummy(-1,-1, -1);
  tuple<Process, long long int> p1 = make_tuple(dummy, -1);
  tuple<Process, long long int> p2 = make_tuple(dummy, -1);
  tuple<Process, long long int> p3 = make_tuple(dummy, -1);
  tuple<Process, long long int> p4 = make_tuple(dummy, -1);

  srand(seed);
  for (int i = 0; i < 38; i++){
      Process temp = Process(rand(), i, remMem/8);
      remMem -= temp.getMem();
      readyQueue.push(temp);
  }
  Process temp = Process(rand(), 39, (remMem <= 0 ? 1 : remMem + 1));
  temp.setMem(remMem);
  readyQueue.push(temp);

  if (scenario > 2)
    memSize = scenario == 3 ? 5000 : 1000;
  else
    memSize = 10000;
  rear.start = memSize;
  rear.end = memSize;


  auto start = chrono::high_resolution_clock::now();

  while(procCount != 39){
    //printf("%d\n", procCount);
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
          p1 = make_tuple(temp, temp.getBurst());
          readyQueue.pop();
        } else {
          myMem1 = my_malloc(temp.getID(), temp.getMem());
          if (myMem1 == -1){
            if (readyQueue.empty()|| fragMap.find(temp.getID())->second){
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
            if (readyQueue.empty()|| fragMap.find(temp.getID())->second){
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
            if (readyQueue.empty()|| fragMap.find(temp.getID())->second){
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
