#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

using namespace std;

#define SLEEP 3

struct Thread
{
    int thread_id;
    int total_threads;
};

void *f(void *arg)
{
    Thread *data = static_cast<Thread *>(arg);

    int sleep_time = rand()%(SLEEP+1);
    sleep(sleep_time);

    printf("I'm %d of %d thread\n", data->thread_id, data->total_threads);

    pthread_exit(nullptr);
    return nullptr;
}

int main()
{
    int N;
    cout << "Enter N: ";
    cin >> N;

    srand(time(nullptr));

    pthread_t threads[N];
    Thread thread_data[N];

    for (int i = 0; i < N; i++)
    {
        thread_data[i].thread_id = i;
        thread_data[i].total_threads = N;

        int result = pthread_create(&threads[i],nullptr,f, &thread_data[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(threads[i], nullptr);
    }

    cout <<"FINISH\n";
    return 0;
}