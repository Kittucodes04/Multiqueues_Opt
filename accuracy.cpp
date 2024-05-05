#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <x86intrin.h>

using namespace std;
#define p 12
#define k 2
#define pk 24
sem_t mut[pk];
float globalmin = 1000000;
int temp = 1000;        
unsigned int seed = 1333; 
std::mt19937 gen(seed);   
std::uniform_int_distribution<> dis(1, 100);
int value, value1, value2, count1 = 0, count2 = 0, count3 = 0;
float globalError1 = 0, globalError2 = 0, globalError3 = 0, count;
vector<priority_queue<int, vector<int>, greater<int>>> pq;
int minind = INT_MAX;
int randomInt(int min, int max) { return min + dis(gen) % (max - min + 1); }

constexpr double CPU_FREQUENCY_GHZ = 2.4;

void minofAllqueues(void) {
  globalmin = 1000000;
  for (int i = 0; i < pk; i++) {
    if (!pq[i].empty()) {
      globalmin = (float)min(pq[i].top(), (int)globalmin);
      minind = i;
    }
  }
}

void *insert(void *arg) {
  while (1) {
    int index = dis(gen) % pk;
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 100000; i++) {
        int t = 1 + rand() % 1000;
        pq[index].push(t);
      }
      uint64_t end = __rdtsc();
      sem_post(&mut[index]);
      break;
    }
  }
  return NULL;
}

void *deleteMin(void *arg) {
  int n = 0;
  while (n < 1000000) {
    int index1 = rand() % pk;
    int index2 = rand() % pk;
    if (index1 == index2)
      continue;
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    int ops = 0;
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        while (!pq[index2].empty() && n + ops < 1000000) {
          minofAllqueues();
          count1++;
          globalError3 += (abs(globalmin - pq[index1].top()) / (globalmin*100));
          pq[index2].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      } else {
        sem_wait(&mut[index1]);
        while (!pq[index1].empty() && n + ops < 1000000) {
          minofAllqueues();
          count1++;
          globalError3 += (abs(globalmin - pq[index1].top()) / (globalmin*100));
          pq[index1].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      }
      n += ops;
    }
  }
  return NULL;
}

void *OptExactDelete(void *arg) {
  int i = *((int *)(arg));
  int flag = 1;
  int n = 0;
  while (n < 1000000) {
    int index1, index2;
    if (flag) {
      index1 = randomInt(i, i + k);
      index2 = randomInt(i, i + k);
      index1 = index1 % pk;
      index2 = index2 % pk;
      flag = 0;
      if (index1 == index2) {
        flag = 1;
        continue;
      }

    } else {
      if (i < pk / 2) {
        index1 = randomInt(0, pk / 2);
        index2 = randomInt(0, pk / 2);
      } else {
        index1 = randomInt((pk / 2) + 1, pk - 1);
        index2 = randomInt((pk / 2) + 1, pk - 1);
      }
      if (index1 == index2) {
        continue;
      }
    }
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    int ops = 0;
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        while (!pq[index2].empty() && n + ops < 1000000) {
          minofAllqueues();
          while(globalmin <= 0) {
            pq[minind].pop();
            count3++;
            minofAllqueues();
          }
          count3++;
          globalError2 += (abs(globalmin - pq[index1].top()) / (globalmin*100));
          pq[index1].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      } else {
        sem_wait(&mut[index1]);
        while (!pq[index1].empty() && n + ops < 1000000) {
          minofAllqueues();
          while(globalmin <= 0){
            pq[minind].pop();
            count3++;
            minofAllqueues();
          }
          count3++;
          globalError2 += (abs(globalmin - pq[index1].top()) / (globalmin*100));
          pq[index1].pop();
          ops++;
        }
        sem_post(&mut[index1]);
      }
      n += ops;
    }
  }
  return NULL;
}

void *OptHalfDelete(void *arg) {
  int i = *((int *)(arg));
  int check1 = 0;
  while (check1 < 10000) {
    int index1, index2;
    if (i < p / 2) {
      index1 = randomInt(0, pk / 2);
      index2 = randomInt(0, pk / 2);
    } else {
      index1 = randomInt((pk / 2) + 1, pk - 1);
      index2 = randomInt((pk / 2) + 1, pk - 1);
    }
    if (index1 == index2) {
      continue;
    }
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        minofAllqueues();
        if (globalmin <= 0) {
          pq[index2].pop();
          count2++;
          continue;
        }
        globalError1 += (abs(globalmin - pq[index2].top()) / (globalmin*2));
        count2++;
        pq[index2].pop();
        sem_post(&mut[index2]);
        check1++;
      } else {
        sem_wait(&mut[index1]);
        minofAllqueues();
        if (globalmin <= 0) {
          pq[index1].pop();
          count2++;
          continue;
        }
        globalError1 += (abs(globalmin - pq[index1].top()) / (globalmin*2));
        count2++;
        pq[index1].pop();
        sem_post(&mut[index1]);
        check1++;
      }
    }
  }
  return NULL;
}



int main() {
  pq.resize(pk);
  pthread_t th[p];
  for (int i = 0; i < pk; i++) {
    sem_init(&mut[i], 0, 1);
  }
  int s = pk;
  for (int i = 0; i < p; i++) {
    pthread_create(&th[i], NULL, insert, &s);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(th[i], NULL);
  }
  cout << "Insert successfull" << endl;

  pthread_t th1[p];
  for (int i = 0; i < 1; i++) {
    pthread_create(&th1[i], NULL, deleteMin, &i);
  }
  for (int i = 0; i < 1; i++) {
    pthread_join(th1[i], NULL);
  }

  pthread_t thi1[p];
  for (int i = 0; i < p; i++) {
    pthread_create(&thi1[i], NULL, insert, &s);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(thi1[i], NULL);
  }

  pthread_t th2[p];
  for (int i = 0; i < 1; i++) {
    pthread_create(&th1[i], NULL, OptHalfDelete, &i);
  }
  for (int i = 0; i < 1; i++) {
    pthread_join(th1[i], NULL);
  }
  pthread_t thi2[p];
  for (int i = 0; i < p; i++) {
    pthread_create(&thi2[i], NULL, insert, &s);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(thi2[i], NULL);
  }

  pthread_t th3[p];
  for (int i = 0; i < 1; i++) {
    pthread_create(&th3[i], NULL, OptHalfDelete, &i);
  }
  for (int i = 0; i < 1; i++) {
    pthread_join(th3[i], NULL);
  }
  
  cout << "Runned succesfully\n" << endl;
  cout << "Error Percentage of DeleteMin" << (globalError3/count1)/p * 100<< endl;
  cout << "Error Percentage of OptHalfDelete " << (globalError1/count2)/p*100<< endl;
  cout << "Error Percentage of OptExactDelete" << ((globalError2/count3)/p)*100<< endl;

  return 0;
}
