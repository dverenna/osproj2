#include <random>
#include <iostream>
#include <sstream>
#include <functional>
#include "Process.h"

//The constructor for Process, first takes the inputed seed and uses it to seed the random number
//generator that will be used to determine the memory requirements. Then the burst time rng is created,
//and the mt19937 is seeded. The memory requirement is then generated, the id is set, the memory requirement
//is set, the burst time is generated and set, and the arrival time is set to 0
Process::Process(mt19937::result_type seed, int counter, int maxMem){
  srand((unsigned) seed);
  auto burst_rand = std::uniform_int_distribution<unsigned long long int>(10000000,10000000000000);
  mt19937 gen(seed);
  id = counter;
  memReq =  (rand() % maxMem) + 1;
  burst = burst_rand(gen);
  arrival = 0;
}
int Process::getMem(){
  return memReq;
}
int Process::getID(){
  return id;
}
long long int Process::getBurst(){
  return burst;
}
long long int Process::getArrival(){
  return arrival;
}
long long int Process::getEnd(){
  return end;
}
void Process::setMem(int newReq){
  memReq = newReq;
}
void Process::setArriv(long long int arrTime){
  arrival = arrTime;
}
void Process::setEnd(long long int newEnd){
  end = newEnd;
}
string Process::toString() const{
  stringstream out;
  out << "Process ID: " << id  << " Service Time: " << burst << " Memory Required: " << memReq << " Wait Time: " << (end-arrival) - burst << " Total Turnaround Time: " << end - arrival;
  return out.str();
}
