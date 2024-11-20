#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <limits>
#include <algorithm>
#include <windows.h>
using namespace std;

void setConsoleColor(int color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

class DynamicIntervention
{
public:
    struct Intervention
    {
        string name;
        double start_DAYS;
        double end_DAYS;
        double effectiveness;
        string type;
        function<bool(double, double, double)> activation_condition;
        function<bool(double, double, double)> deactivation_condition;

        Intervention(const string &name, double start, double end, double eff, const string &t)
            : name(name), start_DAYS(start), end_DAYS(end), effectiveness(eff), type(t) {}
    };

    void addIntervention(const string &name, double start, double effectiveness, const string &type,
                         function<bool(double, double, double)> activate_cond,
                         function<bool(double, double, double)> deactivate_cond, int simulation_duration)
    {
        double duration = (effectiveness / 100.0) * simulation_duration;
        double end = start + duration;
        interventions.emplace_back(name, start, end, effectiveness, type);
        interventions.back().activation_condition = activate_cond;
        interventions.back().deactivation_condition = deactivate_cond;
    }

    void applyInterventions(double &beta, double &gamma, double current_DAYS, double infected)
    {
        double original_beta = beta;

        for (auto &intervention : interventions)
        {
            bool is_active = intervention.start_DAYS <= current_DAYS && current_DAYS <= intervention.end_DAYS;

            if (!is_active && intervention.activation_condition(current_DAYS, infected, beta))
            {
                intervention.start_DAYS = current_DAYS;
                cout << "Intervention " << intervention.name << " activated.\n";
                is_active = true;
            }
            if (is_active && intervention.deactivation_condition(current_DAYS, infected, beta))
            {
                intervention.end_DAYS = current_DAYS;
                cout << "Intervention " << intervention.name << " deactivated.\n";
                is_active = false;
            }

            if (is_active)
            {
                if (intervention.type == "quarantine" || intervention.type == "lockdown")
                {
                    beta = original_beta * (1 - intervention.effectiveness / 100.0);
                }
                else if (intervention.type == "vaccination")
                {
                    gamma += intervention.effectiveness / 100.0;
                }
            }
            else
            {
                beta = original_beta;
            }
        }
    }

    void printInterventions() const
    {
        for (const auto &intervention : interventions)
        {

            cout << "Intervention " << intervention.name << " | Type: " << intervention.type
                 << " | Start: " << intervention.start_DAYS << " | End: " << intervention.end_DAYS
                 << " | Effectiveness: " << intervention.effectiveness << "\n";
        }
    }

    bool removeIntervention(const string &name)
    {
        auto it = find_if(interventions.begin(), interventions.end(),
                          [&](const Intervention &intervention)
                          { return intervention.name == name; });
        if (it != interventions.end())
        {
            interventions.erase(it);
            return true;
        }
        return false;
    }

private:
    vector<Intervention> interventions;
};

template <typename T>
class CircularQueue
{
private:
    vector<T> queue;
    int front;
    int rear;
    int capacity;
    int currentSize;

    void resize()
    {
        int newCapacity = capacity * 2;
        vector<T> newQueue(newCapacity);
        for (int i = 0; i < currentSize; i++)
        {
            newQueue[i] = queue[(front + i) % capacity];
        }
        queue = newQueue;
        capacity = newCapacity;
        front = 0;
        rear = currentSize - 1;
    }

public:
    CircularQueue(int maxSize = 999999999)
        : capacity(maxSize), front(-1), rear(-1), currentSize(0)
    {
        queue.resize(capacity);
    }

    bool enQueue(T value)
    {
        if (isFull())
        {
            resize();
        }

        if (isEmpty())
        {
            front = 0;
        }

        rear = (rear + 1) % capacity;
        queue[rear] = value;
        currentSize++;
        return true;
    }

    T deQueue()
    {
        if (isEmpty())
        {
            throw runtime_error("Queue is empty. Cannot dequeue.");
        }

        T frontElement = queue[front];

        if (front == rear)
        {
            front = rear = -1;
        }
        else
        {
            front = (front + 1) % capacity;
        }

        currentSize--;
        return frontElement;
    }

    T getFront() const
    {
        if (isEmpty())
        {
            throw runtime_error("Queue is empty. No front element.");
        }
        return queue[front];
    }

    bool isEmpty() const
    {
        return currentSize == 0;
    }

    bool isFull() const
    {
        return currentSize == capacity;
    }

    int size() const
    {
        return currentSize;
    }

    T operator[](int index) const
    {
        if (index < 0 || index >= currentSize)
        {
            throw out_of_range("Index out of range");
        }
        return queue[(front + index) % capacity];
    }
};

class InfectionTrendsAndStatistics
{
private:
    CircularQueue<int> infection_window;
    vector<int> infection_rates;
    int window_size;
    int current_infection_sum;
    void validateWindowSize(int size)
    {
        if (size <= 0)
        {
            throw invalid_argument("Window size must be positive");
        }
    }

public:
    InfectionTrendsAndStatistics(int size = 7)
        : infection_window(size),
          window_size(size),
          current_infection_sum(0) {}

    void recordInfection(int count)
    {
        if (infection_window.isFull())
        {
            current_infection_sum -= infection_window.getFront();
            infection_window.deQueue();
        }

        infection_window.enQueue(count);
        current_infection_sum += count;

        infection_rates.push_back(count);
    }

    double calculateAverageInfectionRate() const
    {
        return infection_window.isEmpty() ? 0.0 : static_cast<double>(current_infection_sum) / infection_window.size();
    }

    int getHighestInfectionRate()
    {
        if (infection_rates.empty())
            return 0;

        int highest = infection_rates[0];
        for (int rate : infection_rates)
        {
            highest = max(highest, rate);
        }
        return highest;
    }

    void displayTrends()
    {
        cout << "Average Infection Rate (Last " << window_size << " Days): " << calculateAverageInfectionRate() << "\n";
        cout << "Highest Infection Rate Recorded: " << getHighestInfectionRate() << "\n";
        cout << "Trend: " << (isIncreasingTrend() ? "Increasing" : "Stable/Decreasing") << "\n";
    }

    bool isIncreasingTrend() const
    {
        if (infection_window.size() < 2)
            return false;

        for (int i = 1; i < infection_window.size(); ++i)
        {
            if (infection_window[i] <= infection_window[i - 1])
                return false;
        }
        return true;
    }
};

template <typename T>
class PriorityQueue
{
private:
    vector<pair<T, int>> heap;

    void heapifyUp(int index)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;
            if (heap[index].second > heap[parent].second)
            {
                swap(heap[index], heap[parent]);
                index = parent;
            }
            else
            {
                break;
            }
        }
    }

    void heapifyDown(int index)
    {
        int size = heap.size();
        while (true)
        {
            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;
            int largest = index;

            if (leftChild < size && heap[leftChild].second > heap[largest].second)
            {
                largest = leftChild;
            }
            if (rightChild < size && heap[rightChild].second > heap[largest].second)
            {
                largest = rightChild;
            }
            if (largest == index)
            {
                break;
            }
            swap(heap[index], heap[largest]);
            index = largest;
        }
    }

public:
    void push(const T &value, int priority)
    {
        heap.emplace_back(value, priority);
        heapifyUp(heap.size() - 1);
    }

    T popFront()
    {
        if (heap.empty())
        {
            throw runtime_error("Priority queue is empty. Cannot pop.");
        }

        T topElement = heap[0].first;
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
        return topElement;
    }

    T top() const
    {
        if (heap.empty())
        {
            throw runtime_error("Priority queue is empty.");
        }
        return heap[0].first;
    }

    bool isEmpty() const
    {
        return heap.empty();
    }

    int size() const
    {
        return heap.size();
    }
};

class CustomizableInfectionRates
{
private:
    PriorityQueue<double> infection_rates;

public:
    void addInfectionRate(double rate)
    {
        infection_rates.push(rate, static_cast<int>(rate * 100));
    }

    double getHighestInfectionRate()
    {
        return !infection_rates.isEmpty() ? infection_rates.top() : 0.0;
    }

    void removeHighestInfectionRate()
    {
        if (!infection_rates.isEmpty())
            infection_rates.popFront();
    }

    void printInfectionRates()
    {
        cout << "Current Highest Infection Rate: " << getHighestInfectionRate() << "\n";
    }
};

class RecoverySimulation
{
private:
    CircularQueue<int> recent_recoveries;
    int window_size;
    int current_recovery_sum;
    vector<DynamicIntervention::Intervention> interventions;

public:
    RecoverySimulation(int size = 7) : window_size(size), current_recovery_sum(0), recent_recoveries(size) {}

    void recordRecovery(int count)
    {
        recent_recoveries.enQueue(count);
        current_recovery_sum += count;

        if (recent_recoveries.size() > window_size)
        {
            current_recovery_sum -= recent_recoveries.getFront();
            recent_recoveries.deQueue();
        }
    }

    double calculateAverageRecoveryRate() const
    {
        return recent_recoveries.isEmpty() ? 0.0 : static_cast<double>(current_recovery_sum) / recent_recoveries.size();
    }

    void applyInterventions(double &gamma, double current_DAYS, double recovered)
    {
        for (auto &intervention : interventions)
        {
            bool is_active = intervention.start_DAYS <= current_DAYS && current_DAYS <= intervention.end_DAYS;
            if (is_active)
                gamma += intervention.effectiveness / 100.0;
        }
    }

    void printRecoveryStatistics() const
    {
        cout << "Average Recovery Rate over Last " << window_size << " Days: " << calculateAverageRecoveryRate() << "\n";
    }
};

struct DailyData
{
    int value;
    DailyData *next;
    DailyData(int v) : value(v), next(nullptr) {}
};

class Region
{
public:
    string name;
    int population;
    double infectionRate;
    bool infected;
    int recoveries;
    int deaths;
    int currentDay;

    DailyData *dailyInfected;
    DailyData *dailyRecovered;
    DailyData *dailyDeaths;

    Region(string n, int pop) : name(n), population(pop), infectionRate(0.0), infected(false),
                                recoveries(0), deaths(0), currentDay(0),
                                dailyInfected(nullptr), dailyRecovered(nullptr), dailyDeaths(nullptr) {}

    void addDailyData(DailyData *&head, int value)
    {
        DailyData *newData = new DailyData(value);
        if (!head)
        {
            head = newData;
        }
        else
        {
            DailyData *temp = head;
            while (temp->next)
            {
                temp = temp->next;
            }
            temp->next = newData;
        }
    }

    void displayStatistics() const
    {
        cout << "Daily Infection Stats:\n";
        DailyData *temp = dailyInfected;
        while (temp)
        {
            cout << "Infected: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;

        cout << "Daily Recovery Stats:\n";
        temp = dailyRecovered;
        while (temp)
        {
            cout << "Recovered: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;

        cout << "Daily Death Stats:\n";
        temp = dailyDeaths;
        while (temp)
        {
            cout << "Deaths: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;
    }

    void simulateDay()
    {
        int newInfected = population * infectionRate / 100;

        addDailyData(dailyInfected, newInfected);
        infected = true;
    }

    void simulateRecoveryAndDeaths()
    {
        if (dailyInfected)
        {
            int recoveriesToday = static_cast<int>(dailyInfected->value * 0.2);
            int deathsToday = static_cast<int>(dailyInfected->value * 0.05);

            addDailyData(dailyRecovered, recoveriesToday);
            addDailyData(dailyDeaths, deathsToday);

            recoveries += recoveriesToday;
            deaths += deathsToday;
            population -= (recoveriesToday + deathsToday);
        }
    }

    void nextDay()
    {
        simulateDay();
        simulateRecoveryAndDeaths();
        currentDay++;
    }
};

class MaxHeap
{
private:
    pair<double, string> heap[10];
    int heapSize;

    void heapify(int index)
    {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < heapSize && heap[left].first > heap[largest].first)
        {
            largest = left;
        }

        if (right < heapSize && heap[right].first > heap[largest].first)
        {
            largest = right;
        }

        if (largest != index)
        {
            swap(heap[index], heap[largest]);
            heapify(largest);
        }
    }

public:
    MaxHeap() : heapSize(0) {}

    void insert(double infectionRate, string regionName)
    {
        if (heapSize >= 10)
        {
            cout << "Heap is full, cannot insert more regions!" << endl;
            return;
        }
        heap[heapSize] = {infectionRate, regionName};
        int index = heapSize;
        heapSize++;

        while (index > 0 && heap[(index - 1) / 2].first < heap[index].first)
        {
            swap(heap[index], heap[(index - 1) / 2]);
            index = (index - 1) / 2;
        }
    }

    pair<double, string> pop()
    {
        if (heapSize == 0)
        {
            cout << "Heap is empty!" << endl;
            return {-1, ""};
        }

        pair<double, string> topRegion = heap[0];
        heap[0] = heap[heapSize - 1];
        heapSize--;
        heapify(0);

        return topRegion;
    }

    pair<double, string> top() const
    {
        if (heapSize > 0)
        {
            return heap[0];
        }
        return {-1, ""};
    }

    int size() const { return heapSize; }
};

template <typename T>
class CustomQueue
{
private:
    struct Node
    {
        T data;
        Node *next;
        Node(T value) : data(value), next(nullptr) {}
    };

    Node *frontNode;
    Node *rearNode;
    int queueSize;

public:
    CustomQueue() : frontNode(nullptr), rearNode(nullptr), queueSize(0) {}

    void push(T value)
    {
        Node *newNode = new Node(value);

        if (isEmpty())
        {
            frontNode = rearNode = newNode;
        }
        else
        {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        queueSize++;
    }

    T front()
    {
        if (isEmpty())
        {
            throw runtime_error("Queue is empty");
        }
        return frontNode->data;
    }

    void pop()
    {
        if (isEmpty())
        {
            throw runtime_error("Queue is empty");
        }

        Node *temp = frontNode;
        frontNode = frontNode->next;
        delete temp;

        if (frontNode == nullptr)
        {
            rearNode = nullptr;
        }
        queueSize--;
    }

    bool isEmpty() const
    {
        return queueSize == 0;
    }

    int size() const
    {
        return queueSize;
    }

    ~CustomQueue()
    {
        while (!isEmpty())
        {
            pop();
        }
    }
};

class PandemicSimulation
{
private:
    unordered_map<string, vector<string>> connections;

public:
    unordered_map<string, Region *> regions;
    void addRegion(string name, int population)
    {
        if (regions.count(name) == 0)
        {
            regions[name] = new Region(name, population);
            cout << "Region " << name << " added successfully with population " << population << ".\n";
        }
        else
        {
            cout << "Region already exists!\n";
        }
    }

    void connectRegions(string region1, string region2)
    {
        if (regions.count(region1) && regions.count(region2))
        {
            connections[region1].push_back(region2);
            connections[region2].push_back(region1);
            cout << "Connected " << region1 << " with " << region2 << ".\n";
        }
        else
        {
            cout << "One or both regions do not exist!\n";
        }
    }

    void simulateSpread(string startRegion, double initialInfectionRate, InfectionTrendsAndStatistics &infectionStats, RecoverySimulation &recoverySim)
    {
        if (regions.count(startRegion) == 0)
        {
            cout << "Start region does not exist!\n";
            return;
        }

        CustomQueue<string> bfsQueue;
        bfsQueue.push(startRegion);
        regions[startRegion]->infected = true;
        regions[startRegion]->infectionRate = initialInfectionRate;

        cout << "Simulating infection spread...\n";

        int totalInfected = 0;
        int totalRecoveries = 0;
        int totalDeaths = 0;

        map<int, pair<int, int>> dailyStats;

        int day = 0;

        while (!bfsQueue.isEmpty())
        {
            int currentLevelSize = bfsQueue.size();
            day++;
            cout << "\n--- Day " << day << " ---\n";

            for (int i = 0; i < currentLevelSize; ++i)
            {
                string current = bfsQueue.front();
                bfsQueue.pop();

                Region *currentRegion = regions[current];
                cout << "Region: " << current
                     << " | Infection Rate: " << currentRegion->infectionRate
                     << "% | Population Infected: "
                     << static_cast<int>((currentRegion->infectionRate / 100) * currentRegion->population)
                     << "\n";

                int newInfections = static_cast<int>((currentRegion->infectionRate / 100.0) * currentRegion->population);
                int newRecoveries = static_cast<int>(newInfections * 0.2);
                int newDeaths = static_cast<int>(newInfections * 0.05);

                totalInfected += newInfections;
                totalRecoveries += newRecoveries;
                totalDeaths += newDeaths;

                dailyStats[day].first += newInfections;
                dailyStats[day].second += newRecoveries;

                currentRegion->recoveries += newRecoveries;
                currentRegion->deaths += newDeaths;
                currentRegion->population -= (newRecoveries + newDeaths);

                for (string neighbor : connections[current])
                {
                    if (!regions[neighbor]->infected)
                    {
                        regions[neighbor]->infected = true;
                        regions[neighbor]->infectionRate = currentRegion->infectionRate * 0.8;
                        bfsQueue.push(neighbor);
                        cout << "Infection spread to " << neighbor << " with Infection Rate: " << regions[neighbor]->infectionRate << "%\n";
                    }
                }
            }

            infectionStats.recordInfection(totalInfected);
            recoverySim.recordRecovery(totalRecoveries);

            setConsoleColor(4);
            cout << "\t Daily Statistics for Day " << "[ " << day << " ]" << ":\n";
            cout << "\t New Infections: " << dailyStats[day].first << "\n";
            cout << "\t New Recoveries: " << dailyStats[day].second << "\n";
            cout << "\t Total Infected: " << totalInfected << "\n";
            cout << "\t Total Recovered: " << totalRecoveries << "\n";
            cout << "\t Total Deaths: " << totalDeaths << "\n";
        }

        cout << "\n=== Infection Spread Summary ===\n";
        cout << "Total Days Simulated: " << day << "\n";
        cout << "Total Infected Individuals: " << totalInfected << "\n";
        cout << "Total Recoveries: " << totalRecoveries << "\n";
        cout << "Total Deaths: " << totalDeaths << "\n";
    }

    void displayRegions() const
    {
        cout << "Regions:\n";
        for (auto &entry : regions)
        {
            const Region *region = entry.second;
            cout << "Name: " << region->name
                 << " | Population: " << region->population
                 << " | Infection Rate: " << region->infectionRate
                 << "% | Infected: " << (region->infected ? "Yes" : "No") << "\n";
        }
    }

    void displayStatistics() const
    {
        int total_infected = 0;
        int total_recovered = 0;
        int total_deaths = 0;
        int population = 0;

        for (const auto &entry : regions)
        {
            const Region *region = entry.second;
            total_infected += static_cast<int>((region->infectionRate / 100.0) * region->population);
            total_recovered += region->recoveries;
            total_deaths += region->deaths;
            population += region->population;
        }
        setConsoleColor(1);
        cout << "\t=== Aggregate Pandemic Statistics ===\n";
        cout << "\tTotal Infected: " << total_infected << "\n";
        cout << "\tTotal Recovered: " << total_recovered << "\n";
        cout << "\tTotal Deaths: " << total_deaths << "\n";
        cout << "\tTotal Population: " << population << "\n";
        cout << "\tSusceptible Population: " << (population - total_infected - total_recovered - total_deaths) << "\n";
        setConsoleColor(15);
    }

    void advanceOneDay(InfectionTrendsAndStatistics &infectionStats, RecoverySimulation &recoverySim)
    {
        for (auto &entry : regions)
        {
            Region *region = entry.second;

            if (region->infected)
            {
                region->simulateRecoveryAndDeaths();

                infectionStats.recordInfection(static_cast<int>((region->infectionRate / 100.0) * region->population));
                recoverySim.recordRecovery(region->recoveries);
            }
        }

        displayStatistics();
    }
};

int main()
{
    DynamicIntervention model;
    InfectionTrendsAndStatistics infectionStats(7);
    RecoverySimulation recoverySim(7);
    CustomizableInfectionRates customInfectionRates;
    PandemicSimulation pandemicSim;
    int simulation_duration;
    double beta, gamma;
    int population;
    int infected;
    int recovered = 0;
    int deaths = 0;
    bool exit_program = false;
    bool is_simulation_running = false;
    setConsoleColor(1);
    cout << "Do you want to run the simulation? (y/n): ";
    char run_simulation;
    setConsoleColor(15);
    cin >> run_simulation;
    if (run_simulation == 'y' || run_simulation == 'Y')
    {
        is_simulation_running = true;
    }
    else
    {
        exit_program = true;
    }

    if (is_simulation_running)
    {
        cout << "\n=== Infection Simulation Interactive Setup ===\n";

        setConsoleColor(2);
        cout << "Enter simulation duration in days: ";
        setConsoleColor(15);
        cin >> simulation_duration;
        setConsoleColor(2);
        cout << "Enter total population: ";
        setConsoleColor(15);
        cin >> population;
        setConsoleColor(2);
        cout << "Enter initial number of infected individuals: ";
        setConsoleColor(15);
        cin >> infected;
        if (infected > population)
        {
            setConsoleColor(4);
            cout << "Initial infected cannot exceed total population. Setting infected to population.\n";
            infected = population;
            setConsoleColor(15);
        }
        setConsoleColor(2);
        cout << "Enter number of recovered individuals (e.g., 0): ";
        setConsoleColor(15);
        cin >> recovered;
        if (recovered > (population - infected))
        {
            setConsoleColor(4);
            cout << "Initial recovered cannot exceed population minus infected. Setting recovered to "
                 << (population - infected) << ".\n";
            recovered = population - infected;
            setConsoleColor(15);
        }
        setConsoleColor(2);
        cout << "Enter infection rate (beta, e.g., 1.0): ";
        setConsoleColor(15);
        cin >> beta;
        if (beta < 0.0 || beta > 100.0)
        {
            setConsoleColor(4);
            cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
            setConsoleColor(15);
            return 1;
        }
        customInfectionRates.addInfectionRate(beta);
        setConsoleColor(2);
        cout << "Enter recovery rate (gamma, e.g., 0.1): ";
        setConsoleColor(15);
        cin >> gamma;
        if (gamma < 0.0 || gamma > 1.0)
        {
            setConsoleColor(4);
            cout << "Invalid recovery rate. Please enter a value between 0 and 1.\n";
            setConsoleColor(15);
            return 1;
        }
        recoverySim.recordRecovery(static_cast<int>(gamma * 100));

        int numRegions;
        setConsoleColor(2);
        cout << "\nEnter the number of regions for Pandemic Simulation: ";
        setConsoleColor(15);
        cin >> numRegions;
        cin.ignore();

        for (int i = 0; i < numRegions; i++)
        {
            string regionName;
            int regionPopulation;
            setConsoleColor(2);
            cout << "Enter the name of region " << (i + 1) << ": ";
            setConsoleColor(15);
            getline(cin, regionName);
            setConsoleColor(2);
            cout << "Enter the population of " << regionName << ": ";
            setConsoleColor(15);
            cin >> regionPopulation;
            cin.ignore();

            pandemicSim.addRegion(regionName, regionPopulation);
        }

        string connectMore = "y";
        while (connectMore == "y" || connectMore == "Y")
        {
            string region1, region2;
            setConsoleColor(2);
            cout << "\nEnter two regions to connect:\nFirst Region: ";
            setConsoleColor(15);
            getline(cin, region1);
            setConsoleColor(2);
            cout << "Second Region: ";
            setConsoleColor(15);
            getline(cin, region2);

            pandemicSim.connectRegions(region1, region2);
            setConsoleColor(2);
            cout << "Do you want to add more connections (y/n)? ";
            setConsoleColor(15);
            cin >> connectMore;
            cin.ignore();
        }

        string startRegion;
        double initialInfectionRate;
        setConsoleColor(2);
        cout << "\nEnter the starting region for infection simulation: ";
        getline(cin, startRegion);
        setConsoleColor(15);
        cout << "Enter the initial infection rate (percentage %): ";
        setConsoleColor(15);
        cin >> initialInfectionRate;
        cin.ignore();

        if (initialInfectionRate < 0.0 || initialInfectionRate > 100.0)
        {
            cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
            return 1;
        }

        pandemicSim.simulateSpread(startRegion, initialInfectionRate, infectionStats, recoverySim);
    }
    double current_day = 0;

    while (!exit_program)
    {
        setConsoleColor(1);
        cout << "\t====== Simulation Menu =====\n";
        cout << "|\t1. Add Intervention\t\t|\n";
        cout << "|\t2. Remove Intervention\t\t|\n";
        cout << "|\t3. View Current Interventions\t|\n";
        cout << "|\t4. Run Simulation Step (" << current_day << " Day)\t|\n";
        cout << "|\t5. Run Entire Simulation\t|\n";
        cout << "|\t6. View Statistics\t\t|\n";
        cout << "|\t7. Customize Infection Rate\t|\n";
        cout << "|\t8. Customize Recovery Rate\t|\n";
        cout << "|\t9. Manage Pandemic Spread\t|\n";
        cout << "|\t10. Exit\t\t|";
        cout << "\t===============================\n";
        cout << "Select an option: ";
        setConsoleColor(15);
        int choice;
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            string name, type;
            double start_day, effectiveness;
            setConsoleColor(2);
            cout << "Enter intervention name: ";
            setConsoleColor(15);
            cin >> ws;
            getline(cin, name);
            setConsoleColor(2);
            cout << "Enter intervention type (lockdown/quarantine/vaccination): ";
            setConsoleColor(15);
            getline(cin, type);
            setConsoleColor(2);
            cout << "Enter effectiveness percentage (0-100): ";
            setConsoleColor(15);
            cin >> effectiveness;
            setConsoleColor(2);
            cout << "Enter start day for the intervention: ";
            setConsoleColor(15);
            cin >> start_day;

            function<bool(double, double, double)> activate_cond = [&](double day, double inf, double b) -> bool
            {
                return day >= start_day;
            };

            function<bool(double, double, double)> deactivate_cond = [&](double day, double inf, double b) -> bool
            {
                return day >= (start_day + 5);
            };

            model.addIntervention(name, start_day, effectiveness, type, activate_cond, deactivate_cond, simulation_duration);
            setConsoleColor(1);
            cout << "Intervention added successfully.\n";
            setConsoleColor(15);

            break;
        }
        case 2:
        {
            string name;
            setConsoleColor(1);
            cout << "Enter the name of the intervention to remove: ";
            setConsoleColor(15);
            cin >> ws;
            getline(cin, name);

            bool removed = model.removeIntervention(name);
            setConsoleColor(2);
            if (removed)

                cout << "Intervention \"" << name << "\" removed successfully.\n";
            else
                cout << "Intervention \"" << name << "\" not found.\n";

            break;
        }
        case 3:
        {
            model.printInterventions();
            break;
        }
        case 4:
        {
            if (current_day > simulation_duration)
            {
                cout << "Simulation has already reached the end.\n";
                break;
            }

            cout << "\nDay: " << current_day << "\n";

            model.applyInterventions(beta, gamma, current_day, infected);
            setConsoleColor(15);
            cout << "Infection Rate (Beta): " << beta << "\n";
            cout << "Recovery Rate (Gamma): " << gamma << "\n";

            pandemicSim.advanceOneDay(infectionStats, recoverySim);

            cout << "Current Infected: " << infected << "\n";
            cout << "Total Recovered: " << recovered << "\n";
            cout << "Susceptible Population: " << (population - infected - recovered) << "\n";

            current_day += 1;
            break;
        }
        case 5:
        {
            while (current_day <= simulation_duration)
            {
                cout << "\nDay: " << current_day << "\n";
                model.applyInterventions(beta, gamma, current_day, infected);
                cout << "Infection Rate (Beta): " << beta << "\n";
                cout << "Recovery Rate (Gamma): " << gamma << "\n";

                double new_infections = beta * infected;
                double new_recoveries = gamma * infected;

                if ((infected + new_infections - new_recoveries + recovered + deaths) > population)
                {
                    new_infections = population - infected - recovered - deaths + new_recoveries;
                    if (new_infections < 0)
                        new_infections = 0;
                }

                infected = infected + static_cast<int>(new_infections) - static_cast<int>(new_recoveries);
                recovered += static_cast<int>(new_recoveries);

                deaths = 0;
                for (const auto &entry : pandemicSim.regions)
                {
                    deaths += entry.second->deaths;
                }

                if (infected < 0)
                    infected = 0;
                if (recovered > population - deaths)
                    recovered = population - deaths;
                if (infected + recovered + deaths > population)
                {
                    infected = population - recovered - deaths;
                }

                infectionStats.recordInfection(infected);

                recoverySim.recordRecovery(static_cast<int>(gamma * 100));

                cout << "Current Infected: " << infected << "\n";
                cout << "Total Recovered: " << recovered << "\n";
                cout << "Total Deaths: " << deaths << "\n";
                cout << "Susceptible Population: " << (population - infected - recovered - deaths) << "\n";

                current_day += 1;
            }
            cout << "Simulation completed.\n";
            break;
        }
        case 6:
        {
            cout << "\n=== Simulation Statistics ===\n";
            infectionStats.displayTrends();
            recoverySim.printRecoveryStatistics();
            cout << "Total Population: " << population << "\n";
            cout << "Current Infected: " << infected << "\n";
            cout << "Total Recovered: " << recovered << "\n";
            cout << "Total Deaths: " << deaths << "\n";
            cout << "Susceptible Population: " << (population - infected - recovered - deaths) << "\n";
            break;
        }
        case 7:
        {
            setConsoleColor(1);
            cout << "\n=== Customize Infection Rate ===\n";
            cout << "1. Add a new Infection Rate\n";
            cout << "2. View Current Infection Rates\n";
            cout << "3. Remove the Highest Infection Rate\n";
            cout << "4. Back to Main Menu\n";
            cout << "Select an option: ";
            setConsoleColor(15);

            int sub_choice;
            cin >> sub_choice;

            switch (sub_choice)
            {
            case 1:
            {
                double new_beta;
                cout << "Enter new infection rate (beta): ";
                cin >> new_beta;
                if (new_beta < 0.0 || new_beta > 100.0)
                {
                    setConsoleColor(4);
                    cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
                    break;
                }
                customInfectionRates.addInfectionRate(new_beta);
                setConsoleColor(1);
                cout << "New infection rate added.\n";
                break;
            }
            case 2:
            {
                customInfectionRates.printInfectionRates();
                break;
            }
            case 3:
            {
                customInfectionRates.removeHighestInfectionRate();
                setConsoleColor(2);
                cout << "Highest infection rate removed.\n";
                break;
            }
            case 4:
            {
                break;
            }
            default:
                setConsoleColor(4);
                cout << "Please select a valid option (1-4).\n";
            }

            break;
        }
        case 8:
        {
            setConsoleColor(1);
            cout << "\n=== Customize Recovery Rate ===\n";
            cout << "1. Add a new Recovery Rate\n";
            cout << "2. View Current Recovery Rates\n";
            cout << "3. Back to Main Menu\n";
            cout << "Select an option: ";
            setConsoleColor(15);
            int sub_choice;
            cin >> sub_choice;

            switch (sub_choice)
            {
            case 1:
            {
                double new_gamma;

                cout << "Enter new recovery rate (between 0 ot 100): ";
                cin >> new_gamma;
                new_gamma /= 100;
                if (new_gamma < 0.0 || new_gamma > 1.0)
                {
                    setConsoleColor(4);
                    cout << "Invalid recovery rate. Please enter a value between 0 and 100.\n";
                    setConsoleColor(15);
                    break;
                }
                recoverySim.recordRecovery(static_cast<int>(new_gamma * 100));
                cout << "New recovery rate added.\n";
                break;
            }
            case 2:
            {
                recoverySim.printRecoveryStatistics();
                break;
            }
            case 3:
            {
                break;
            }
            default:
                setConsoleColor(4);
                cout << "Please select a valid option (1-3).\n";
                setConsoleColor(15);
            }

            break;
        }
        case 9:
        {
            setConsoleColor(1);
            cout << "\n=== Manage Pandemic Spread ===\n";
            cout << "1. Add a new Region\n";
            cout << "2. Connect Regions\n";
            cout << "3. Simulate Infection Spread with Statistics\n";
            cout << "4. Display All Regions\n";
            cout << "5. Display Pandemic Statistics\n";
            cout << "6. Back to Main Menu\n";
            setConsoleColor(15);
            cout << "Select an option: ";
            int spread_choice;
            cin >> spread_choice;
            cin.ignore();
            bool flag = true;

            while (flag)
            {
                switch (spread_choice)
                {
                case 1:
                {
                    string regionName;
                    int regionPopulation;
                    setConsoleColor(2);
                    cout << "Enter the name of the new region: ";
                    setConsoleColor(15);
                    getline(cin, regionName);
                    setConsoleColor(2);
                    cout << "Enter the population of " << regionName << ": ";
                    setConsoleColor(15);
                    cin >> regionPopulation;
                    cin.ignore();
                    pandemicSim.addRegion(regionName, regionPopulation);
                    setConsoleColor(2);
                    cout << "Region added successfully.\n";
                    setConsoleColor(15);
                    break;
                }
                case 2:
                {
                    string region1, region2;
                    setConsoleColor(2);
                    cout << "Enter the first region to connect: ";
                    setConsoleColor(15);
                    getline(cin, region1);
                    setConsoleColor(2);
                    cout << "Enter the second region to connect: ";
                    setConsoleColor(15);
                    getline(cin, region2);
                    pandemicSim.connectRegions(region1, region2);
                    break;
                }
                case 3:
                {
                    string startRegion;
                    double initialInfectionRate;
                    setConsoleColor(2);
                    cout << "Enter the starting region for infection simulation: ";
                    setConsoleColor(15);
                    getline(cin, startRegion);
                    setConsoleColor(2);
                    cout << "Enter the initial infection rate (percentage 0 to 100): ";
                    setConsoleColor(15);
                    cin >> initialInfectionRate;
                    cin.ignore();
                    if (initialInfectionRate < 0.0 || initialInfectionRate > 100.0)
                    {
                        cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
                        break;
                    }
                    pandemicSim.simulateSpread(startRegion, initialInfectionRate, infectionStats, recoverySim);
                    break;
                }
                case 4:
                {
                    pandemicSim.displayRegions();
                    break;
                }
                case 5:
                {
                    pandemicSim.displayStatistics();
                    break;
                }
                case 6:
                {
                    flag = false;
                    break;
                }
                default:
                    setConsoleColor(4);
                    cout << "Please select a valid option (1-6).\n";
                    setConsoleColor(15);
                }

                if (flag)
                {
                    setConsoleColor(1);
                    cout << "\n=== Manage Pandemic Spread ===\n";
                    cout << "1. Add a new Region\n";
                    cout << "2. Connect Regions\n";
                    cout << "3. Simulate Infection Spread with Statistics\n";
                    cout << "4. Display All Regions\n";
                    cout << "5. Display Pandemic Statistics\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Select an option: ";
                    setConsoleColor(15);
                    cin >> spread_choice;
                    cin.ignore();
                }
            }
            break;
        }
        case 10:
        {
            exit_program = true;
            setConsoleColor(2);
            cout << "Exiting simulation. Goodbye!\n";
            setConsoleColor(15);
            break;
        }
        default:
            setConsoleColor(4);
            cout << "Please select a valid option (1-10).\n";
            setConsoleColor(15);
        }
    }

    for (auto &entry : pandemicSim.regions)
    {
        delete entry.second;
    }

    return 0;
}