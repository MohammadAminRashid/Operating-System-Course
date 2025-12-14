#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <chrono>
#include <cmath>

using namespace std;
using namespace std::chrono;

#define MAX_VALUE_IN_ARRAY 100
#define MAIN_SIZE 10000000

struct Thread
{
    int *array;
    int start_index;
    int end_index;
    long long partial_sum;
};

void *sum_partial_array(void *arg)
{
    Thread *t = static_cast<Thread *>(arg);
    t->partial_sum = 0;
    for (int i = t->start_index; i < t->end_index; i++)
    {
        t->partial_sum += t->array[i];
    }
    pthread_exit(nullptr);
    return nullptr;
}

void generate_random_array(int *array, int size)
{
    for (int i = 0; i<size; i++)
    {
        array[i]=rand()% MAX_VALUE_IN_ARRAY;
    }
}

long long single_threaded_sum(int *array, int size, double &elapsed_time)
{
    auto start = steady_clock::now();

    long long total = 0;
    for (int i = 0; i < size; i++)
    {
        total += array[i];
    }
    auto end = steady_clock::now();
    elapsed_time = duration<double>(end - start).count();
    return total;
}

long long multi_threaded_sum(int *array,int size,int num_threads,double &elapsed_time)
{
    auto start = steady_clock::now();

    vector<pthread_t> threads(num_threads);
    vector<Thread> t(num_threads);

    int x=(size/num_threads);

    int current_start = 0;
    for (int i = 0; i < num_threads; i++)
    {
        int current_end=current_start+x;

        t[i].array = array;
        t[i].start_index = current_start;
        t[i].end_index = current_end;

        pthread_create(&threads[i], nullptr,sum_partial_array, &t[i]);

        current_start = current_end;
    }

    long long total = 0;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], nullptr);
        total+=t[i].partial_sum;
    }

    auto end = steady_clock::now();
    elapsed_time = duration<double>(end - start).count();

    return total;
}

int main()
{
    srand(time(nullptr));
    int final_test;
    cin >> final_test;
    if (final_test != 0)
    {
        vector<int> array_sizes = {10000,100000,1000000,10000000,100000000,200000000};
        vector<int> thread_counts = {2, 4, 8, 16};
        for (int size : array_sizes)
        {

            double single_time;
            long long single_sum;
            int *array = new int[size];
            generate_random_array(array, size);

            single_sum = single_threaded_sum(array, size, single_time);

            for (int t : thread_counts)
            {
                double multi_time;
                long long multi_sum;
                bool correct;
                double speedup;
                double size_per_thread;
                multi_sum = multi_threaded_sum(array, size, t, multi_time);
                correct = (single_sum == multi_sum);
                speedup = single_time / multi_time;
                size_per_thread= size/t;

                printf("Size  Thread_count  SingleTime  MultiTime  Speedup  Status  size/thread\n");
                printf("%d   %d      %f      %f    %f    %s   %f\n\n\n", size, t, single_time, multi_time, speedup, correct ? "OK" : "WRONG" , size_per_thread);
            } 

            delete[] array;
        }
    }
    else
    {

        double single_time;
        long long single_sum;
        long long multi_sum;
        double multi_time;

        int *main_array = new int[MAIN_SIZE];
        generate_random_array(main_array, MAIN_SIZE);

        single_sum = single_threaded_sum(main_array, MAIN_SIZE, single_time);
        multi_sum = multi_threaded_sum(main_array, MAIN_SIZE, 8, multi_time);



        cout<<"single :"<< single_sum << "  "<<single_time<<endl;
        cout<<"multi  :"<< multi_sum << "  "<<multi_time<<endl;

        delete[] main_array;
    }

    return 0;
}