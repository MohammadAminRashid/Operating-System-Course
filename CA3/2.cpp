#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>

using namespace std;

struct Process
{
    string name;
    int arrival;
    int burst;
    int remaining;
    int start;
    int finish;
    int waiting;
    int turnaround;
};

vector<int> quantums = {1, 4, 8};


void print_results(const vector<Process> &processes, const string &algorithm)
{
    double total_turnaround = 0;
    double total_waiting = 0;
    cout << "\n"<< algorithm << ":" << endl;
    cout << "Proc  Arrival  Burst  Start  Finish  Waiting  Turnaround" << endl;
    for (const auto &p : processes)
    {
        printf("%-5s %-8d %-6d %-6d %-7d %-8d %-10d\n", p.name.c_str(), p.arrival, p.burst, p.start, p.finish, p.waiting, p.turnaround);

        total_waiting += p.waiting;
        total_turnaround += p.turnaround;
    }
    printf("\n Average waiting time %f\n", (total_waiting / processes.size()));
    printf(" Average turnaround time: %f\n", (total_turnaround / processes.size()));
}

vector<Process> fcfs_scheduler(vector<Process> processes)
{
    int n = processes.size();

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = 0; j < n - i - 1; j++)
        {
            if (processes[j].arrival > processes[j + 1].arrival)
            {
                Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }

    int current_time = 0;
    for (auto &p : processes)
    {
        if (current_time < p.arrival)
        {
            current_time = p.arrival;
        }

        p.start = current_time;
        p.finish = current_time + p.burst;
        p.waiting = p.start - p.arrival;
        p.turnaround = p.finish - p.arrival;
        current_time = p.finish;
    }
    return processes;
}

vector<Process> sjf_scheduler(vector<Process> processes)
{
    int n = processes.size();
    vector<bool> completed(n, false);
    int current_time = 0;
    int completed_count = 0;
    int selected_index;
    int shortest_burst;
    while (true)
    {
        if (completed_count == n)
        {
            break;
        }
        selected_index = -1;
        shortest_burst = INT_MAX;

        for (int i = 0; i < n; i++)
        {
            if (completed[i] == false)
            {
                if (processes[i].arrival <= current_time)
                {
                    if (processes[i].burst < shortest_burst)
                    {
                        shortest_burst = processes[i].burst;
                        selected_index = i;
                    }
                }
            }
        }

        if (selected_index == -1)
        {
            current_time++;
            continue;
        }
        else
        {
            processes[selected_index].start = current_time;
            processes[selected_index].finish = current_time + processes[selected_index].burst;
            processes[selected_index].waiting = current_time - processes[selected_index].arrival;
            processes[selected_index].turnaround = processes[selected_index].finish - processes[selected_index].arrival;
            completed[selected_index] = true;
            completed_count++;
            current_time = processes[selected_index].finish;
        }
    }

    return processes;
}

vector<Process> rr_scheduler(vector<Process> processes, int quantum)
{
    int n = processes.size();
    for (auto &p : processes)
    {
        p.remaining = p.burst;
        p.start = -1;
    }

    queue<int> ready_queue;
    vector<bool> in_queue(n, false);
    int current_time = 0;
    int completed_count = 0;

    int first_arrival_index = 0;
    for (int i = 1; i < n; i++)
    {
        if (processes[i].arrival < processes[first_arrival_index].arrival)
        {
            first_arrival_index = i;
        }
    }

    ready_queue.push(first_arrival_index);
    in_queue[first_arrival_index] = true;
    while (true)
    {
        if (completed_count == n)
        {
            break;
        }
        if (ready_queue.empty())
        {
            current_time++;

            for (int i = 0; i < n; i++)
            {
                if (in_queue[i] == false and processes[i].arrival <= current_time and processes[i].remaining > 0)
                {
                    ready_queue.push(i);
                    in_queue[i] = true;
                }
            }
            continue;
        }
        else
        {

            int index = ready_queue.front();
            ready_queue.pop();
            in_queue[index] = false;

            if (processes[index].start == -1)
            {
                processes[index].start = current_time;
            }

            int exec_time;
            if (quantum < processes[index].remaining)
            {
                exec_time = quantum;
            }
            else
            {
                exec_time = processes[index].remaining;
            }

            processes[index].remaining -= exec_time;
            current_time += exec_time;

            for (int i = 0; i < n; i++)
            {
                if (in_queue[i] == false and processes[i].arrival <= current_time and processes[i].remaining > 0 and i != index)
                {
                    ready_queue.push(i);
                    in_queue[i] = true;
                }
            }

            if (processes[index].remaining > 0)
            {
                ready_queue.push(index);
                in_queue[index] = true;
            }
            else
            {
                processes[index].finish = current_time;
                processes[index].waiting = processes[index].finish - processes[index].arrival - processes[index].burst;
                processes[index].turnaround = processes[index].finish - processes[index].arrival;
                completed_count++;
            }
        }
    }

    std::sort(processes.begin(), processes.end(), [](const Process &a, const Process &b)
              {
    if (a.start == b.start){ return a.arrival < b.arrival;}
    else{  
    return a.start < b.start;
    } });

    return processes;
}

void test_scenario(const vector<Process> &original_processes, int scenario_num)
{

    cout << "\nscenario "<<scenario_num<<" :" << endl;
    vector<Process> processes;
    processes = fcfs_scheduler(original_processes);
    print_results(processes, "FCFS");

    processes = sjf_scheduler(original_processes);
    print_results(processes, "SJF");


    for (int q : quantums)
    {
        processes = rr_scheduler(original_processes, q);
        print_results(processes, "RR q=" + to_string(q));

    }
}

int main()
{
    vector<Process> s1 = {{"P1", 0, 8},{"P2", 0, 4},{"P3", 0, 1},{"P4", 0, 3}};
    vector<Process> s2 = {{"P1", 0, 7},{"P2", 2, 4},{"P3", 4, 1},{"P4", 5, 4}};
    vector<Process> s3 = {{"P1", 0, 20},{"P2", 0, 3},{"P3", 0, 3}};

    test_scenario(s1, 1);
    test_scenario(s2, 2);
    test_scenario(s3, 3);
    return 0;
}