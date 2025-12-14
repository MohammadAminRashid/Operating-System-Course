#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <iomanip>

using namespace std;

int counter_unsafe = 0;
int counter_safe = 0;
pthread_mutex_t mutex;

void *increment_unsafe(void *arg)
{
    int *k = static_cast<int *>(arg);
    for (int i = 0; i < *k; i++)
    {
        counter_unsafe++; 
    }
    return nullptr;
}

void *increment_safe(void *arg)
{
    int *k = static_cast<int *>(arg);
    for (int i = 0; i < *k; i++)
    {
        pthread_mutex_lock(&mutex);
        counter_safe++;
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}
void without_lock(int M, int K)
{

    vector<pthread_t> threads_unsafe(M);
    for (int i = 0; i < M; i++)
    {
        pthread_create(&threads_unsafe[i], nullptr, increment_unsafe, &K);
    }

    for (int i = 0; i < M; i++)
    {
        pthread_join(threads_unsafe[i], nullptr);
    }

    cout << "Result=" << counter_unsafe << endl;

    return;
}

void with_lock(int M, int K)
{

    pthread_mutex_init(&mutex, nullptr);
    vector<pthread_t> threads_safe(M);
    for (int i = 0; i < M; i++)
    {
        pthread_create(&threads_safe[i], nullptr, increment_safe, &K);
    }

    for (int i = 0; i < M; i++)
    {
        pthread_join(threads_safe[i], nullptr);
    }

    pthread_mutex_destroy(&mutex);
      cout << "Result=" << counter_safe << endl;

    return;
}
int main()
{

    int M, K;
    cin >> M;
    cin >> K;
    int expected = M * K;
    cout << "EXPECTED :" << expected << endl;
    without_lock(M, K);
    with_lock(M, K);
    return 0;
}