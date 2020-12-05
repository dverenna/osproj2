#include <random>
#include <iostream>
using namespace std;

class Process {
  private:
    int id, memReq;
    long long int burst, arrival, end;
    int *memLocat;
  public:
    Process(mt19937::result_type seed, int counter, int maxMem);
    int getMem();
    int getID();
    long long int getBurst();
    long long int getArrival();
    long long int getEnd();
    void setMem(int newReq);
    void setLocat(int* memStart);
    void setArriv(long long int arrTime);
    void setEnd(long long int newEnd);
    string toString() const;
};
