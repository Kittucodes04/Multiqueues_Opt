#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <x86intrin.h>

using namespace std;

#define p 12
#define k 2
#define pk 24

vector<priority_queue<int, vector<int>, greater<int>>> pq(pk);
sem_t mut[pk];

unsigned int seed = 1333;
std::mt19937 gen(seed);
std::uniform_int_distribution<> dis(1, 100);

int randomInt(int min, int max) { return min + dis(gen) % (max - min + 1); }

constexpr double CPU_FREQUENCY_GHZ = 2.4;
long long throughput_og = 0;
long long throughput_half = 0;

void BalanceQueues() {
  int largestQueueIndex = 0;
  int shortestQueueIndex = 0;

  for (int i = 1; i < pk; ++i) {
    if (pq[i].size() > pq[largestQueueIndex].size()) {
      largestQueueIndex = i;
    }
    if (pq[i].size() < pq[shortestQueueIndex].size()) {
      shortestQueueIndex = i;
    }
  }

  if (pq[largestQueueIndex].size() >
      (double)accumulate(
          pq.begin(), pq.end(), 0,
          [](int sum, const priority_queue<int, vector<int>, greater<int>> &q) {
            return sum + q.size();
          }) /
          pk * 0.2) {
    sem_wait(&mut[largestQueueIndex]);
    sem_wait(&mut[shortestQueueIndex]);

    size_t sizeToTransfer = pq[largestQueueIndex].size() * 0.3;
    while (sizeToTransfer > 0 && !pq[largestQueueIndex].empty()) {
      pq[shortestQueueIndex].push(pq[largestQueueIndex].top());
      pq[largestQueueIndex].pop();
      --sizeToTransfer;
    }

    sem_post(&mut[largestQueueIndex]);
    sem_post(&mut[shortestQueueIndex]);
  }
}

void *insert(void *arg) {
  int i = *((int *)(arg));
  cout<<"Thread "<<i<<" is Inserting"<<endl;
  while (1) {
    int index = randomInt(0, pk - 1);
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 1000000; i++) {
        pq[index].push(randomInt(0, 100));
      }
      uint64_t end = __rdtsc();

      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9);

      throughput_og += (1000000 / elapsed_seconds);
      sem_post(&mut[index]);
      BalanceQueues();
      break;
    }
  }
  return NULL;
}

void *OptHalfinsert(void *arg) {
  int i = *((int *)(arg));
  cout<<"Thread "<<i<<" is inserting."<<endl;
  while (1) {
    int index;
    if (i < p / 2)
      index = randomInt(0, (pk / 2) - 1);
    else {
      index = randomInt(pk / 2, pk - 1);
    }
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int j= 0; j < 1000000; j++) {
        pq[index].push(randomInt(0, 100));
      }
      uint64_t end = __rdtsc();

      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9);

      throughput_half += (1000000 / elapsed_seconds);
      sem_post(&mut[index]);
      BalanceQueues();
      break;
    }
  }
  return NULL;
}

int main() {
  pthread_t th[p];
  pthread_t th1[p];

  for (int i = 0; i < pk; i++) {
    sem_init(&mut[i], 0, 1);
  }

  for (int i = 0; i < p; i++) {
    pthread_create(&th[i], NULL, insert, &i);
  }

  for (int i = 0; i < p; i++) {
    pthread_join(th[i], NULL);
  }
  BalanceQueues();

  printf("Original Throughput: %lld\n", throughput_og);

  for (int i = 0; i < p; i++) {
    pthread_create(&th1[i], NULL, OptHalfinsert, &i);
  }

  for (int i = 0; i < p; i++) {
    pthread_join(th1[i], NULL);
  }

  printf("Half Insert Throughput: %lld\n", throughput_half);

  BalanceQueues();
  cout << "Ran successfully" << endl;

  return 0;
}
