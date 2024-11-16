#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
using namespace std;

// Daily statistics class to hold infection trends
class InfectionTrendsStatistics {
public:
    struct DailyData {
        int value;         // Statistic value (infections, recoveries, deaths)
        DailyData* next;
        DailyData(int v) : value(v), next(nullptr) {}
    };

    DailyData* dailyInfected;
    DailyData* dailyRecovered;
    DailyData* dailyDeaths;

    InfectionTrendsStatistics() : dailyInfected(nullptr), dailyRecovered(nullptr), dailyDeaths(nullptr) {}

    // Add daily statistics data to the linked list
    void addDailyData(DailyData*& head, int value) {
        DailyData* newData = new DailyData(value);
        if (!head) {
            head = newData;
        }
        else {
            DailyData* temp = head;
            while (temp->next) {
                temp = temp->next;
            }
            temp->next = newData;
        }
    }

    // Display statistics
    void displayStatistics() {
        cout << "Daily Infection Stats:\n";
        DailyData* temp = dailyInfected;
        while (temp) {
            cout << "Infected: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;

        cout << "Daily Recovery Stats:\n";
        temp = dailyRecovered;
        while (temp) {
            cout << "Recovered: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;

        cout << "Daily Death Stats:\n";
        temp = dailyDeaths;
        while (temp) {
            cout << "Deaths: " << temp->value << " ";
            temp = temp->next;
        }
        cout << endl;
    }
};

// Region class to store region-specific details and manage infection trends
class Region {
public:
    string name;          // Name of the region
    int population;       // Population of the region
    double infectionRate; // Infection rate (percentage of population infected)
    bool infected;        // Whether the region is infected
    int recoveries;       // Number of recoveries
    int deaths;           // Number of deaths
    int currentDay;       // Current day of simulation

    InfectionTrendsStatistics stats; // Infection trends for the region

    // Constructor
    Region(string n, int pop) : name(n), population(pop), infectionRate(0.0), infected(false),
                                recoveries(0), deaths(0), currentDay(0) {}

    // Simulate the infection spread for a day
    void simulateDay() {
        int newInfected = population * infectionRate / 100;

        if (stats.dailyInfected == nullptr){
            stats.addDailyData(stats.dailyInfected, newInfected);
        }else{
            stats.addDailyData(stats.dailyInfected, newInfected);
        }
        infected = true;
    }

    // Simulate recoveries and deaths
    void simulateRecoveryAndDeaths() {
        if(stats.dailyInfected){
        int recoveriesToday = stats.dailyInfected ? stats.dailyInfected->value * 0.2 : 0; // 20% recovery
        int deathsToday = stats.dailyInfected ? stats.dailyInfected->value * 0.05 : 0;    // 5% death rate

        stats.addDailyData(stats.dailyRecovered, recoveriesToday);
        stats.addDailyData(stats.dailyDeaths, deathsToday);

        recoveries += recoveriesToday;
        deaths += deathsToday;
        population -= (recoveriesToday + deathsToday);
        }
    }

    // Increment the day and simulate infection and recovery
    void nextDay() {
        simulateDay();
        simulateRecoveryAndDeaths();
        currentDay++;
    }
};

// Max-Heap for infection rate tracking
class MaxHeap {
private:
    pair<double, string> heap[10]; // Store pair of infection rate and region name
    int heapSize;

    void heapify(int index) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < heapSize && heap[left].first > heap[largest].first) {
            largest = left;
        }

        if (right < heapSize && heap[right].first > heap[largest].first) {
            largest = right;
        }

        if (largest != index) {
            swap(heap[index], heap[largest]);
            heapify(largest);
        }
    }

public:
    MaxHeap() : heapSize(0) {}

    void insert(double infectionRate, string regionName) {
        if (heapSize >= 10) {
            cout << "Heap is full, cannot insert more regions!" << endl;
            return;
        }
        heap[heapSize] = {infectionRate, regionName};
        int index = heapSize;
        heapSize++;

        while (index > 0 && heap[(index - 1) / 2].first < heap[index].first) {
            swap(heap[index], heap[(index - 1) / 2]);
            index = (index - 1) / 2;
        }
    }

    pair<double, string> pop() {
        if (heapSize == 0) {
            cout << "Heap is empty!" << endl;
            return {-1, ""};
        }

        pair<double, string> topRegion = heap[0];
        heap[0] = heap[heapSize - 1];
        heapSize--;
        heapify(0);

        return topRegion;
    }

    pair<double, string> top() {
        if (heapSize > 0) {
            return heap[0];
        }
        return {-1, ""};
    }

    int size() { return heapSize; }
};

// Pandemic Simulation class using regions
class PandemicSimulation {
private:
    unordered_map<string, vector<string>> connections;

public:
    // Add a new region
    unordered_map<string, Region*> regions;
    void addRegion(string name, int population) {
        if (regions.count(name) == 0) {
            regions[name] = new Region(name, population);
        }
        else {
            cout << "Region already exists!\n";
        }
    }

    // Connect two regions (bidirectional)
    void connectRegions(string region1, string region2) {
        if (regions.count(region1) && regions.count(region2)) {
            connections[region1].push_back(region2);
            connections[region2].push_back(region1);
        }
        else {
            cout << "One or both regions do not exist!\n";
        }
    }

    // Simulate infection spread using BFS
    void simulateSpread(string startRegion, double initialInfectionRate) {
        if (regions.count(startRegion) == 0) {
            cout << "Start region does not exist!\n";
            return;
        }

        queue<string> bfsQueue;
        bfsQueue.push(startRegion);
        regions[startRegion]->infected = true;
        regions[startRegion]->infectionRate = initialInfectionRate;

        cout << "Simulating infection spread...\n";

        while (!bfsQueue.empty()) {
            string current = bfsQueue.front();
            bfsQueue.pop();

            cout << "Region: " << current
                 << " | Infection Rate: " << regions[current]->infectionRate
                 << "% | Population Infected: "
                 << static_cast<int>((regions[current]->infectionRate / 100) * regions[current]->population)
                 << "\n";

            for (string neighbor : connections[current]) {
                if (!regions[neighbor]->infected) {
                    regions[neighbor]->infected = true;
                    regions[neighbor]->infectionRate = regions[current]->infectionRate * 0.8; // Reduced infection rate
                    bfsQueue.push(neighbor);
                }
            }
        }
    }

    // Display all regions and their statuses
    void displayRegions() {
        cout << "Regions:\n";
        for (auto &entry : regions) {
            Region *region = entry.second;
            cout << "Name: " << region->name
                 << " | Population: " << region->population
                 << " | Infection Rate: " << region->infectionRate
                 << "% | Infected: " << (region->infected ? "Yes" : "No") << "\n";
                 cout<<endl;
        }
    }

    // Display statistics for each region
    void displayStatistics() {
        for (auto& entry : regions) {
            cout << "\nStatistics for Region: " << entry.second->name << "\n";
            entry.second->stats.displayStatistics();
        }
    }
};

int main() {
    PandemicSimulation simulation;
    int numRegions;

    // User input for number of regions
    cout << "Enter the number of regions: ";
    cin >> numRegions;
    cin.ignore(); // To consume the newline character left by cin

    // Adding regions
    for (int i = 0; i < numRegions; i++) {
        string regionName;
        int population;

        cout << "\nEnter the name of region " << (i + 1) << ": ";
        getline(cin, regionName);
        cout << "Enter the population of " << regionName << ": ";
        cin >> population;
        cin.ignore();

        simulation.addRegion(regionName, population);
    }

    // Connecting regions
    string connectMore = "y";
    while (connectMore == "y" || connectMore == "Y") {
        string region1, region2;
        cout << "\nEnter two regions to connect: ";
        getline(cin, region1);
        getline(cin, region2);

        simulation.connectRegions(region1, region2);
        cout << "Do you want to add more connections (y/n)? ";
        cin >> connectMore;
        cin.ignore();
    }

    // Start simulation
    string startRegion;
    double initialInfectionRate;
    cout << "\nEnter the starting region for infection simulation: ";
    getline(cin, startRegion);
    cout << "Enter the initial infection rate (percentage): ";
    cin >> initialInfectionRate;

    simulation.simulateSpread(startRegion, initialInfectionRate);

    // Simulate the pandemic for some days (example)
    int days;
    cout << "\nEnter the number of days to simulate: ";
    cin >> days;

    for (int day = 0; day < days; ++day) {
        cout << "\nDay " << (day + 1) << " simulation...\n";
        for (auto& entry : simulation.regions) {
            entry.second->nextDay();
        }
    }

    // Display the results
    simulation.displayRegions();
    simulation.displayStatistics();

    return 0;
}

