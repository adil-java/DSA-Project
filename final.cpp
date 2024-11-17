#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#define SIZE 100
using namespace std;

// ------------------------------------ Infection Trend Node Class ------------------------------------
class InfectionTrendNode
{
public:
    int infected;
    int recovered;
    int deaths;
    InfectionTrendNode *next;

    InfectionTrendNode(int inf, int rec, int dth)
        : infected(inf), recovered(rec), deaths(dth), next(nullptr) {}
};

// ------------------------------------ Infection Trends Statistics Class ------------------------------------
class InfectionTrendsStatistics
{
public:
    InfectionTrendNode *head;

    InfectionTrendsStatistics() : head(nullptr) {}

    void addTrend(int infected, int recovered, int deaths)
    {
        InfectionTrendNode *newNode = new InfectionTrendNode(infected, recovered, deaths);
        newNode->next = head;
        head = newNode;
    }

    void displayTrends(const string &regionName)
    {
        cout << "Infection Trends for Region: " << regionName << "\n";
        InfectionTrendNode *current = head;
        while (current)
        {
            cout << "Infected: " << current->infected << ", Recovered: " << current->recovered
                 << ", Deaths: " << current->deaths << "\n";
            current = current->next;
        }
        cout << endl;
    }

    ~InfectionTrendsStatistics()
    {
        while (head)
        {
            InfectionTrendNode *temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// ------------------------------------ MaxHeap Class ------------------------------------
class MaxHeap
{
private:
    vector<pair<double, string>> heap; // Stores pairs of (infection rate, region name)

    void heapifyUp(int index)
    {
        while (index > 0)
        {
            int parentIndex = (index - 1) / 2;
            if (heap[index].first <= heap[parentIndex].first)
            {
                break;
            }
            swap(heap[index], heap[parentIndex]);
            index = parentIndex;
        }
    }

    void heapifyDown(int index)
    {
        int size = heap.size();
        while (index < size)
        {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int largest = index;

            if (left < size && heap[left].first > heap[largest].first)
            {
                largest = left;
            }
            if (right < size && heap[right].first > heap[largest].first)
            {
                largest = right;
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
    void insert(double infectionRate, const string &regionName)
    {
        heap.push_back({infectionRate, regionName});
        heapifyUp(heap.size() - 1);
    }

    pair<double, string> pop()
    {
        if (heap.empty())
        {
            return {-1, "Heap is empty"};
        }
        pair<double, string> top = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        heapifyDown(0);
        return top;
    }

    pair<double, string> top()
    {
        return heap.empty() ? make_pair(-1, "Heap is empty") : heap[0];
    }

    bool empty()
    {
        return heap.empty();
    }
};

// ------------------------------------ Disease Class ------------------------------------
class Disease
{
public:
    string name;              // Name of the disease
    double baseInfectionRate; // Base infection rate for the disease
    double recoveryRate;      // Recovery rate
    double deathRate;         // Death rate

    Disease(string n, double infRate, double recRate, double dRate)
        : name(n), baseInfectionRate(infRate), recoveryRate(recRate), deathRate(dRate) {}
};

// ------------------------------------ Region Class ------------------------------------
class Region
{
public:
    string name;
    int population;
    bool infected;
    double infectionRate;
    int currentlyInfected;
    InfectionTrendsStatistics trends;
    Disease *disease; // Pointer to the disease affecting the region

    Region(string name, int population, Disease *d)
        : name(name), population(population), infected(false), infectionRate(d->baseInfectionRate), currentlyInfected(0), disease(d) {}

    void nextDay()
    {
        if (infected)
        {
            int recovered = static_cast<int>(currentlyInfected * disease->recoveryRate); // Recovery based on disease recovery rate
            int deaths = static_cast<int>(currentlyInfected * disease->deathRate);       // Deaths based on disease death rate
            currentlyInfected -= (recovered + deaths);
            if (currentlyInfected < 0)
                currentlyInfected = 0;

            trends.addTrend(currentlyInfected, recovered, deaths);
            cout << "In " << name << ", infection stats updated: "
                 << "Infected: " << currentlyInfected
                 << ", Recovered: " << recovered
                 << ", Deaths: " << deaths << endl;
        }
    }

    void updateInfection(int newInfected)
    {
        currentlyInfected += newInfected;
        currentlyInfected = min(currentlyInfected, population); // Cap at population
    }

    void resetInfectionRate(double newInfectionRate)
    {
        infectionRate = newInfectionRate;
    }
};

// ------------------------------------ Dynamic Intervention Class ------------------------------------
class DynamicIntervention
{
public:
    struct Intervention
    {
        string name;
        double start_DAYS;
        double end_DAYS;
        double effectiveness; // Effectiveness of the intervention
        string type;          // Type of intervention (e.g., quarantine, lockdown, vaccination)
        function<bool(double, double, double)> activation_condition;
        function<bool(double, double, double)> deactivation_condition;

        Intervention(const string &name, double start, double end, double eff, const string &t)
            : name(name), start_DAYS(start), end_DAYS(end), effectiveness(eff), type(t) {}
    };

    vector<Intervention> interventions;

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
                    beta *= (1 - intervention.effectiveness / 100.0); // Reduce infection rate
                }
                else if (intervention.type == "vaccination")
                {
                    gamma += intervention.effectiveness / 100.0; // Increase recovery rate
                }
            }
            else
            {
                beta = original_beta; // Reset beta if not active
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

    bool removeIntervention(const string &name, unordered_map<string, Region *> &regions)
    {
        auto it = find_if(interventions.begin(), interventions.end(),
                          [&](const Intervention &intervention)
                          { return intervention.name == name; });
        if (it != interventions.end())
        {
            // When removing an intervention, reset the infection rate
            if (it->type == "quarantine" || it->type == "lockdown")
            {
                // Increase the infection rate back to its base value
                for (auto &region : regions)
                {
                    region.second->resetInfectionRate(region.second->disease->baseInfectionRate);
                }
            }
            interventions.erase(it);
            return true;
        }
        return false;
    }
};

// ------------------------------------ Pandemic Simulation Class ------------------------------------
class PandemicSimulation
{
private:
    unordered_map<string, vector<string>> connections;
    MaxHeap infectionHeap;

public:
    DynamicIntervention interventions;
    unordered_map<string, Region *> regions;

    // Add a new region
    void addRegion(string name, int population, Disease *disease)
    {
        if (regions.count(name) == 0)
        {
            regions[name] = new Region(name, population, disease);
            cout << "Added region: " << name << " with population: " << population << " and disease: " << disease->name << endl;
        }
        else
        {
            cout << "Region already exists!\n";
        }
    }

    // Connect two regions (bidirectional)
    void connectRegions(string region1, string region2)
    {
        if (regions.count(region1) && regions.count(region2))
        {
            connections[region1].push_back(region2);
            connections[region2].push_back(region1);
            cout << "Connected " << region1 << " and " << region2 << endl;
        }
        else
        {
            cout << "One or both regions do not exist!\n";
        }
    }

    // Simulate infection spread using BFS
    void simulateSpread(string startRegion)
    {
        if (regions.count(startRegion) == 0)
        {
            cout << "Start region does not exist!\n";
            return;
        }

        double initialInfectionRate = regions[startRegion]->infectionRate;

        class Queue
        {
        private:
            string items[SIZE];
            int front, rear;

        public:
            Queue() : front(-1), rear(-1) {}

            // Check if the queue is full
            bool isFull()
            {
                return (front == 0 && rear == SIZE - 1) || (front == (rear + 1) % SIZE);
            }

            // Check if the queue is empty
            bool isEmpty()
            {
                return front == -1;
            }

            // Add an element to the queue
            void enQueue(const string &element)
            {
                if (isFull())
                {
                    cout << "Queue is full" << endl;
                    return;
                }
                if (front == -1)
                {
                    front = 0;
                }
                rear = (rear + 1) % SIZE;
                items[rear] = element;
            }

            // Remove an element from the queue
            string deQueue()
            {
                if (isEmpty())
                {
                    cout << "Queue is empty" << endl;
                    return "";
                }
                string element = items[front];
                if (front == rear)
                {
                    front = -1;
                    rear = -1;
                }
                else
                {
                    front = (front + 1) % SIZE;
                }
                return element;
            }

            // Get the front element of the queue
            string frontElement()
            {
                if (isEmpty())
                {
                    cout << "Queue is empty" << endl;
                    return "";
                }
                return items[front];
            }
        };

        Queue bfsQueue;
        bfsQueue.enQueue(startRegion);
        regions[startRegion]->infected = true;
        regions[startRegion]->updateInfection(static_cast<int>((initialInfectionRate / 100) * regions[startRegion]->population)); // Set initial infected
        infectionHeap.insert(initialInfectionRate, startRegion);

        cout << "Simulating infection spread...\n";

        while (!bfsQueue.isEmpty())
        {
            string current = bfsQueue.frontElement();
            bfsQueue.deQueue();

            cout << "Region: " << current
                 << " | Infection Rate: " << regions[current]->infectionRate
                 << "% | Population Infected: "
                 << regions[current]->currentlyInfected
                 << "\n";

            for (string neighbor : connections[current])
            {
                if (!regions[neighbor]->infected)
                {
                    double spreadRate = regions[current]->infectionRate * 0.8; // Infection spreads at a reduced rate
                    regions[neighbor]->infected = true;
                    regions[neighbor]->infectionRate = spreadRate;
                    regions[neighbor]->updateInfection(static_cast<int>((spreadRate / 100) * regions[neighbor]->population));
                    infectionHeap.insert(regions[neighbor]->infectionRate, neighbor);
                    bfsQueue.enQueue(neighbor);
                }
            }
        }
    }

    // Apply interventions
    void applyInterventions(double &beta, double &gamma, double current_DAYS, double infected)
    {
        interventions.applyInterventions(beta, gamma, current_DAYS, infected);
    }

    // Display all regions and their statuses
    void displayRegions()
    {
        cout << "Regions:\n";
        for (auto &entry : regions)
        {
            Region *region = entry.second;
            cout << "Name: " << region->name
                 << " | Population: " << region->population
                 << " | Infection Rate: " << region->infectionRate
                 << "% | Currently Infected: " << region->currentlyInfected << "\n";
        }
        cout << endl;
    }

    // Display statistics for each region
    void displayStatistics()
    {
        for (auto &entry : regions)
        {
            entry.second->trends.displayTrends(entry.second->name);
        }
    }

    // Display the region with the highest infection rate
    void displayHighestInfectionRate()
    {
        if (infectionHeap.empty())
        {
            cout << "No infection records available." << endl;
            return;
        }
        auto topRecord = infectionHeap.top();
        cout << "Region with highest infection rate: " << topRecord.second
             << " with infection rate of " << topRecord.first << "%" << endl;
    }

    // Destructor to clean up memory
    ~PandemicSimulation()
    {
        for (auto &entry : regions)
        {
            delete entry.second;
        }
    }
};

// ------------------------------------ Main Interactive Function ------------------------------------
void Display()
{
    PandemicSimulation simulation;

    // Define disease types
    Disease flu("Flu", 5.0, 0.1, 0.02);          // 5% base infection rate, 10% recovery rate, 2% death rate
    Disease covid("COVID-19", 10.0, 0.15, 0.03); // 10% base infection rate, 15% recovery rate, 3% death rate
    Disease measles("Measles", 15.0, 0.2, 0.05); // 15% base infection rate, 20% recovery rate, 5% death rate
    Disease ebola("Ebola", 20.0, 0.3, 0.15);     // 20% base infection rate, 30% recovery rate, 15% death rate
    Disease dengue("Dengue", 8.0, 0.25, 0.01);   // 8% base infection rate, 25% recovery rate, 1% death rate

    // Interactive setup for regions and connections
    int numRegions;
    cout << "Enter the number of regions: ";
    cin >> numRegions;
    cin.ignore(); // To consume the newline character left by cin

    // Adding regions with diseases
    for (int i = 0; i < numRegions; i++)
    {
        string regionName;
        int population;
        char diseaseChoice;

        cout << "\nEnter the name of region " << (i + 1) << ": ";
        getline(cin, regionName);
        cout << "Enter the population of " << regionName << ": ";
        cin >> population;
        cin.ignore();

        cout << "Select the disease (1: Flu, 2: COVID-19, 3: Measles, 4: Ebola, 5: Dengue): ";
        cin >> diseaseChoice;
        cin.ignore();

        switch (diseaseChoice)
        {
        case '1':
            simulation.addRegion(regionName, population, &flu);
            break;
        case '2':
            simulation.addRegion(regionName, population, &covid);
            break;
        case '3':
            simulation.addRegion(regionName, population, &measles);
            break;
        case '4':
            simulation.addRegion(regionName, population, &ebola);
            break;
        case '5':
            simulation.addRegion(regionName, population, &dengue);
            break;
        default:
            cout << "Invalid choice, defaulting to Flu.\n";
            simulation.addRegion(regionName, population, &flu);
            break;
        }
    }

    // Connecting regions
    string connectMore = "y";
    while (connectMore == "y" || connectMore == "Y")
    {
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
    cout << "\nEnter the starting region for infection simulation: ";
    getline(cin, startRegion);

    // Start spreading the infection
    simulation.simulateSpread(startRegion);

    // Interactive menu for simulation
    int choice;
    double beta = 0.0;  // Infection rate
    double gamma = 0.1; // Set a default recovery rate

    do
    {
        cout << "\n=== Simulation Menu ===\n";
        cout << "1. Run Simulation for One Day\n";
        cout << "2. Run Entire Simulation\n";
        cout << "3. Display Regions\n";
        cout << "4. Display Statistics\n";
        cout << "5. Display Highest Infection Rate\n";
        cout << "6. Add Intervention\n";
        cout << "7. Remove Intervention\n";
        cout << "8. View Current Interventions\n";
        cout << "9. Exit\n";
        cout << "Select an option: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            // Run Simulation for One Day
            simulation.applyInterventions(beta, gamma, 1, simulation.regions[startRegion]->currentlyInfected);
            for (auto &entry : simulation.regions)
            {
                entry.second->nextDay();
            }
            break;
        }
        case 2:
        {
            // Run Entire Simulation
            int days;
            cout << "Enter the number of days to simulate: ";
            cin >> days;

            for (int day = 0; day < days; ++day)
            {
                cout << "\nDay " << (day + 1) << " simulation...\n";
                simulation.applyInterventions(beta, gamma, day + 1, simulation.regions[startRegion]->currentlyInfected);
                for (auto &entry : simulation.regions)
                {
                    entry.second->nextDay();
                }
            }
            break;
        }
        case 3:
        {
            // Display Regions
            simulation.displayRegions();
            break;
        }
        case 4:
        {
            // Display Statistics
            simulation.displayStatistics();
            break;
        }
        case 5:
        {
            // Display Highest Infection Rate
            simulation.displayHighestInfectionRate();
            break;
        }
        case 6:
        {
            // Add Intervention
            string name, type;
            double start = 0; // Hardcoded start day for simplicity

            cout << "Enter intervention name: ";
            cin >> ws; // To consume any leading whitespace
            getline(cin, name);
            cout << "Enter intervention type (lockdown/quarantine/vaccination): ";
            getline(cin, type);

            // Hardcoded effectiveness values for each intervention type
            double effectiveness;
            if (type == "lockdown")
            {
                effectiveness = 70.0; // 70% effectiveness
            }
            else if (type == "quarantine")
            {
                effectiveness = 50.0; // 50% effectiveness
            }
            else if (type == "vaccination")
            {
                effectiveness = 80.0; // 80% effectiveness
            }
            else
            {
                cout << "Unknown intervention type. No intervention added.\n";
                break;
            }

            // Define activation and deactivation conditions (simple examples)
            function<bool(double, double, double)> activate_cond = [&](double day, double inf, double b) -> bool
            {
                return day >= start;
            };

            function<bool(double, double, double)> deactivate_cond = [&](double day, double inf, double b) -> bool
            {
                // Example: Deactivate after 5 days
                return day >= (start + 5);
            };

            simulation.interventions.addIntervention(name, start, effectiveness, type, activate_cond, deactivate_cond, 30);
            cout << "Intervention added successfully.\n";
            break;
        }
        case 7:
        {
            // Remove Intervention
            string name;
            cout << "Enter the name of the intervention to remove: ";
            cin >> ws; // To consume any leading whitespace
            getline(cin, name);
            bool removed = simulation.interventions.removeIntervention(name, simulation.regions);
            if (removed)
                cout << "Intervention \"" << name << "\" removed successfully.\n";
            else
                cout << "Intervention \"" << name << "\" not found.\n";
            break;
        }
        case 8:
        {
            // View Current Interventions
            simulation.interventions.printInterventions();
            break;
        }
        case 9:
        {
            // Exit
            cout << "Exiting simulation. Goodbye!\n";
            break;
        }
        default:
            cout << "Please select a valid option (1-9).\n";
        }
    } while (choice != 9);
}

// ------------------------------------ Main Function ------------------------------------
int main()
{
    Display();
    return 0;
}