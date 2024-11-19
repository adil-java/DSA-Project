#include <iostream>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <limits> // For std::numeric_limits
#include <algorithm>
using namespace std;

// ------------------------------------ Dynamic Intervention Class ------------------------------------
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

// ------------------------------------ InfectionTrendsAndStatistics Class ------------------------------------
class InfectionTrendsAndStatistics
{
private:
    deque<int> infection_window;         // Sliding window of recent infection counts
    priority_queue<int> infection_rates; // Max heap for the highest infection rates
    int window_size;                     // Size of the sliding window
    int current_infection_sum;           // Sum of infections in the sliding window

public:
    InfectionTrendsAndStatistics(int size = 7)
        : window_size(size), current_infection_sum(0) {}

    // Record new infection data
    void recordInfection(int count)
    {
        infection_window.push_back(count);
        current_infection_sum += count;

        // Maintain sliding window
        if (infection_window.size() > window_size)
        {
            current_infection_sum -= infection_window.front();
            infection_window.pop_front();
        }

        // Add to priority queue
        infection_rates.push(count);
    }

    // Calculate the average infection rate over the sliding window
    double calculateAverageInfectionRate() const
    {
        return infection_window.empty() ? 0.0 : static_cast<double>(current_infection_sum) / infection_window.size();
    }

    // Get the highest infection rate recorded
    int getHighestInfectionRate()
    {
        return infection_rates.empty() ? 0 : infection_rates.top();
    }

    // Display infection trends
    void displayTrends()
    {
        cout << "Average Infection Rate (Last " << window_size << " Days): " << calculateAverageInfectionRate() << "\n";
        cout << "Highest Infection Rate Recorded: " << getHighestInfectionRate() << "\n";
        cout << "Trend: " << (isIncreasingTrend() ? "Increasing" : "Stable/Decreasing") << "\n";
    }

    // Check if the trend is increasing
    bool isIncreasingTrend() const
    {
        if (infection_window.size() < 2)
            return false;

        for (size_t i = 1; i < infection_window.size(); ++i)
        {
            if (infection_window[i] < infection_window[i - 1])
                return false; // Any decrease breaks the trend
        }
        return true;
    }
};

// ------------------------------------ CustomizableInfectionRates Class ------------------------------------
class CustomizableInfectionRates
{
private:
    priority_queue<double> infection_rates;

public:
    void addInfectionRate(double rate)
    {
        infection_rates.push(rate);
    }

    double getHighestInfectionRate()
    {
        return !infection_rates.empty() ? infection_rates.top() : 0.0;
    }

    void removeHighestInfectionRate()
    {
        if (!infection_rates.empty())
            infection_rates.pop();
    }

    void printInfectionRates()
    {
        cout << "Current Highest Infection Rate: " << getHighestInfectionRate() << "\n";
    }
};

// ------------------------------------ RecoverySimulation Class ------------------------------------
class RecoverySimulation
{
private:
    deque<int> recent_recoveries;
    int window_size;
    int current_recovery_sum;
    vector<DynamicIntervention::Intervention> interventions;

public:
    RecoverySimulation(int size = 7) : window_size(size), current_recovery_sum(0) {}

    void recordRecovery(int count)
    {
        recent_recoveries.push_back(count);
        current_recovery_sum += count;

        if (recent_recoveries.size() > window_size)
        {
            current_recovery_sum -= recent_recoveries.front();
            recent_recoveries.pop_front();
        }
    }

    double calculateAverageRecoveryRate() const
    {
        return recent_recoveries.empty() ? 0.0 : static_cast<double>(current_recovery_sum) / recent_recoveries.size();
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

// ------------------------------------ InfectionTrendsStatistics Class (from spread.cpp) ------------------------------------
struct DailyData
{
    int value; // Statistic value (infections, recoveries, deaths)
    DailyData *next;
    DailyData(int v) : value(v), next(nullptr) {}
};

// ------------------------------------ Region Class (from spread.cpp) ------------------------------------
class Region
{
public:
    string name;          // Name of the region
    int population;       // Population of the region
    double infectionRate; // Infection rate (percentage of population infected)
    bool infected;        // Whether the region is infected
    int recoveries;       // Number of recoveries
    int deaths;           // Number of deaths
    int currentDay;       // Current day of simulation

    // Linked lists for daily statistics
    DailyData *dailyInfected;
    DailyData *dailyRecovered;
    DailyData *dailyDeaths;

    // Constructor
    Region(string n, int pop) : name(n), population(pop), infectionRate(0.0), infected(false),
                                recoveries(0), deaths(0), currentDay(0),
                                dailyInfected(nullptr), dailyRecovered(nullptr), dailyDeaths(nullptr) {}

    // Add daily statistics data to the linked list
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

    // Display statistics
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

    // Simulate the infection spread for a day
    void simulateDay()
    {
        int newInfected = population * infectionRate / 100;

        addDailyData(dailyInfected, newInfected);
        infected = true;
    }

    // Simulate recoveries and deaths
    void simulateRecoveryAndDeaths()
    {
        if (dailyInfected)
        {
            int recoveriesToday = static_cast<int>(dailyInfected->value * 0.2); // 20% recovery
            int deathsToday = static_cast<int>(dailyInfected->value * 0.05);    // 5% death rate

            addDailyData(dailyRecovered, recoveriesToday);
            addDailyData(dailyDeaths, deathsToday);

            recoveries += recoveriesToday;
            deaths += deathsToday;
            population -= (recoveriesToday + deathsToday);
        }
    }

    // Increment the day and simulate infection and recovery
    void nextDay()
    {
        simulateDay();
        simulateRecoveryAndDeaths();
        currentDay++;
    }
};

// ------------------------------------ MaxHeap Class (from spread.cpp) ------------------------------------
class MaxHeap
{
private:
    pair<double, string> heap[10]; // Store pair of infection rate and region name
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

// ------------------------------------ PandemicSimulation Class (from spread.cpp) ------------------------------------
class PandemicSimulation
{
private:
    unordered_map<string, vector<string>> connections;

public:
    // Add a new region
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

    // Connect two regions (bidirectional)
    void connectRegions(string region1, string region2)
    {
        if (regions.count(region1) && regions.count(region2))
        {
            connections[region1].push_back(region2);
            connections[region2].push_back(region1);
            cout << "Connected " << region1 << " with " << region2 << ".\n"; // Confirmation Message
        }
        else
        {
            cout << "One or both regions do not exist!\n";
        }
    }

    // Enhanced Simulate Infection Spread using BFS with Detailed Statistics
    void simulateSpread(string startRegion, double initialInfectionRate, InfectionTrendsAndStatistics &infectionStats, RecoverySimulation &recoverySim)
    {
        if (regions.count(startRegion) == 0)
        {
            cout << "Start region does not exist!\n";
            return;
        }

        queue<string> bfsQueue;
        bfsQueue.push(startRegion);
        regions[startRegion]->infected = true;
        regions[startRegion]->infectionRate = initialInfectionRate;

        cout << "Simulating infection spread...\n";

        // Initialize cumulative statistics
        int totalInfected = 0;
        int totalRecoveries = 0;
        int totalDeaths = 0;

        // Track daily statistics
        map<int, pair<int, int>> dailyStats; // day -> (new infections, new recoveries)

        // Initialize day counter
        int day = 0;

        while (!bfsQueue.empty())
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
                int newRecoveries = static_cast<int>(newInfections * 0.2); // 20% recovery
                int newDeaths = static_cast<int>(newInfections * 0.05);    // 5% death rate

                // Update cumulative statistics
                totalInfected += newInfections;
                totalRecoveries += newRecoveries;
                totalDeaths += newDeaths;

                // Record daily stats
                dailyStats[day].first += newInfections;
                dailyStats[day].second += newRecoveries;

                // Update region statistics
                currentRegion->recoveries += newRecoveries;
                currentRegion->deaths += newDeaths;
                currentRegion->population -= (newRecoveries + newDeaths);

                // Spread infection to connected regions
                for (string neighbor : connections[current])
                {
                    if (!regions[neighbor]->infected)
                    {
                        regions[neighbor]->infected = true;
                        regions[neighbor]->infectionRate = currentRegion->infectionRate * 0.8; // Reduced infection rate
                        bfsQueue.push(neighbor);
                        cout << "Infection spread to " << neighbor << " with Infection Rate: " << regions[neighbor]->infectionRate << "%\n";
                    }
                }
            }

            // Record infection and recovery data for the day
            infectionStats.recordInfection(totalInfected);
            recoverySim.recordRecovery(totalRecoveries);

            // Display daily statistics
            cout << "\nDaily Statistics for Day " << day << ":\n";
            cout << "New Infections: " << dailyStats[day].first << "\n";
            cout << "New Recoveries: " << dailyStats[day].second << "\n";
            cout << "Total Infected: " << totalInfected << "\n";
            cout << "Total Recovered: " << totalRecoveries << "\n";
            cout << "Total Deaths: " << totalDeaths << "\n";
        }

        // Display summary of the spread
        cout << "\n=== Infection Spread Summary ===\n";
        cout << "Total Days Simulated: " << day << "\n";
        cout << "Total Infected Individuals: " << totalInfected << "\n";
        cout << "Total Recoveries: " << totalRecoveries << "\n";
        cout << "Total Deaths: " << totalDeaths << "\n";
    }

    // Display all regions and their statuses
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

    // Display statistics for each region
    void displayStatistics() const
    {
        int total_infected = 0;
        int total_recovered = 0;
        int total_deaths = 0;
        int population = 0; // Define population

        for (const auto &entry : regions)
        {
            const Region *region = entry.second;
            total_infected += static_cast<int>((region->infectionRate / 100.0) * region->population);
            total_recovered += region->recoveries;
            total_deaths += region->deaths;
            population += region->population; // Sum population
        }

        cout << "\n=== Aggregate Pandemic Statistics ===\n";
        cout << "Total Infected: " << total_infected << "\n";
        cout << "Total Recovered: " << total_recovered << "\n";
        cout << "Total Deaths: " << total_deaths << "\n";
        cout << "Total Population: " << population << "\n";
        cout << "Susceptible Population: " << (population - total_infected - total_recovered - total_deaths) << "\n";
    }

    void advanceOneDay(InfectionTrendsAndStatistics &infectionStats, RecoverySimulation &recoverySim)
    {
        // Iterate through all regions
        for (auto &entry : regions)
        {
            Region *region = entry.second;

            if (region->infected)
            {
                // Simulate recoveries and deaths
                region->simulateRecoveryAndDeaths();

                // Update infection trends
                infectionStats.recordInfection(static_cast<int>((region->infectionRate / 100.0) * region->population));
                recoverySim.recordRecovery(region->recoveries);
            }
        }

        // Optionally, display updated statistics
        displayStatistics();
    }
};

// ------------------------------------ Main Interactive Function ------------------------------------
int main()
{
    DynamicIntervention model;
    InfectionTrendsAndStatistics infectionStats(7);
    RecoverySimulation recoverySim(7);
    CustomizableInfectionRates customInfectionRates; // For customizable infection rates
    PandemicSimulation pandemicSim;                  // Instance of PandemicSimulation
    int simulation_duration;
    double beta, gamma;
    int population;
    int infected;
    int recovered = 0; // Initialize recovered population
    int deaths = 0;    // Initialize deaths population
    bool exit_program = false;
    bool is_simulation_running = false;
    cout << "Do you want to run the simulation? (y/n): ";
    char run_simulation;
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

        cout << "=== Infection Simulation Interactive Setup ===\n";

        // Set simulation parameters
        cout << "Enter simulation duration in days: ";
        cin >> simulation_duration;

        cout << "Enter total population: ";
        cin >> population;

        cout << "Enter initial number of infected individuals: ";
        cin >> infected;
        if (infected > population)
        {
            cout << "Initial infected cannot exceed total population. Setting infected to population.\n";
            infected = population;
        }

        cout << "Enter number of recovered individuals (e.g., 0): ";
        cin >> recovered;
        if (recovered > (population - infected))
        {
            cout << "Initial recovered cannot exceed population minus infected. Setting recovered to "
                 << (population - infected) << ".\n";
            recovered = population - infected;
        }

        cout << "Enter infection rate (beta, e.g., 1.0): ";
        cin >> beta;
        if (beta < 0.0 || beta > 100.0)
        {
            cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
            return 1; // Exit the program due to invalid input
        }
        customInfectionRates.addInfectionRate(beta); // Initialize with initial beta

        cout << "Enter recovery rate (gamma, e.g., 0.1): ";
        cin >> gamma;
        if (gamma < 0.0 || gamma > 1.0)
        {
            cout << "Invalid recovery rate. Please enter a value between 0 and 1.\n";
            return 1; // Exit the program due to invalid input
        }
        recoverySim.recordRecovery(static_cast<int>(gamma * 100)); // Store as percentage

        // Initialize Pandemic Simulation Regions
        int numRegions;
        cout << "\nEnter the number of regions for Pandemic Simulation: ";
        cin >> numRegions;
        cin.ignore(); // To consume the newline character left by cin

        for (int i = 0; i < numRegions; i++)
        {
            string regionName;
            int regionPopulation;

            cout << "\nEnter the name of region " << (i + 1) << ": ";
            getline(cin, regionName);
            cout << "Enter the population of " << regionName << ": ";
            cin >> regionPopulation;
            cin.ignore();

            pandemicSim.addRegion(regionName, regionPopulation);
        }

        // Connecting regions
        string connectMore = "y";
        while (connectMore == "y" || connectMore == "Y")
        {
            string region1, region2;
            cout << "\nEnter two regions to connect:\nFirst Region: ";
            getline(cin, region1);
            cout << "Second Region: ";
            getline(cin, region2);

            pandemicSim.connectRegions(region1, region2);
            cout << "Do you want to add more connections (y/n)? ";
            cin >> connectMore;
            cin.ignore();
        }

        // Start infection spread in Pandemic Simulation with Detailed Statistics
        string startRegion;
        double initialInfectionRate;
        cout << "\nEnter the starting region for infection simulation: ";
        getline(cin, startRegion);
        cout << "Enter the initial infection rate (percentage): ";
        cin >> initialInfectionRate;
        cin.ignore();

        // Validate infection rate
        if (initialInfectionRate < 0.0 || initialInfectionRate > 100.0)
        {
            cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
            return 1; // Exit the program due to invalid input
        }

        pandemicSim.simulateSpread(startRegion, initialInfectionRate, infectionStats, recoverySim);
    }
    double current_day = 0;

    while (!exit_program)
    {
        cout << "\n=== Simulation Menu ===\n";
        cout << "1. Add Intervention\n";
        cout << "2. Remove Intervention\n";
        cout << "3. View Current Interventions\n";
        cout << "4. Run Simulation Step (" << current_day << " Day)\n";
        cout << "5. Run Entire Simulation\n";
        cout << "6. View Statistics\n";
        cout << "7. Customize Infection Rate\n"; // New option
        cout << "8. Customize Recovery Rate\n";  // New option
        cout << "9. Manage Pandemic Spread\n";   // New option for PandemicSimulation
        cout << "10. Exit\n";                    // Updated exit option number
        cout << "Select an option: ";

        int choice;
        cin >> choice;

        switch (choice)
        {
        case 1:
        {
            // Add Intervention
            string name, type;
            double start_day, effectiveness;
            cout << "Enter intervention name: ";
            cin >> ws; // To consume any leading whitespace
            getline(cin, name);
            cout << "Enter intervention type (lockdown/quarantine/vaccination): ";
            getline(cin, type);

            cout << "Enter effectiveness percentage (0-100): ";
            cin >> effectiveness;

            cout << "Enter start day for the intervention: ";
            cin >> start_day;

            // Define activation and deactivation conditions (simple examples)
            function<bool(double, double, double)> activate_cond = [&](double day, double inf, double b) -> bool
            {
                return day >= start_day;
            };

            function<bool(double, double, double)> deactivate_cond = [&](double day, double inf, double b) -> bool
            {
                // Example: Deactivate after 5 days
                return day >= (start_day + 5);
            };

            model.addIntervention(name, start_day, effectiveness, type, activate_cond, deactivate_cond, simulation_duration);
            cout << "Intervention added successfully.\n";

            break;
        }
        case 2:
        {
            // Remove Intervention
            string name;
            cout << "Enter the name of the intervention to remove: ";
            cin >> ws; // To consume any leading whitespace
            getline(cin, name);

            // Implement removal by name
            bool removed = model.removeIntervention(name);
            if (removed)
                cout << "Intervention \"" << name << "\" removed successfully.\n";
            else
                cout << "Intervention \"" << name << "\" not found.\n";

            break;
        }
        case 3:
        {
            // View Current Interventions
            model.printInterventions();
            break;
        }
        case 4:
        {
            // Run Simulation Step (One Day)
            if (current_day > simulation_duration)
            {
                cout << "Simulation has already reached the end.\n";
                break;
            }

            cout << "\nDay: " << current_day << "\n";

            // Apply interventions
            model.applyInterventions(beta, gamma, current_day, infected);
            cout << "Infection Rate (Beta): " << beta << "\n";
            cout << "Recovery Rate (Gamma): " << gamma << "\n";

            // Advance Pandemic Spread by one day
            pandemicSim.advanceOneDay(infectionStats, recoverySim);

            // Display current state
            cout << "Current Infected: " << infected << "\n";
            cout << "Total Recovered: " << recovered << "\n";
            cout << "Susceptible Population: " << (population - infected - recovered) << "\n";

            current_day += 1;
            break;
        }
        case 5:
        {
            // Run Entire Simulation
            while (current_day <= simulation_duration)
            {
                cout << "\nDay: " << current_day << "\n";
                // Apply interventions directly to beta and gamma
                model.applyInterventions(beta, gamma, current_day, infected);
                cout << "Infection Rate (Beta): " << beta << "\n";
                cout << "Recovery Rate (Gamma): " << gamma << "\n";

                // Calculate new infections and recoveries
                double new_infections = beta * infected;
                double new_recoveries = gamma * infected;

                // Adjust to prevent exceeding population
                if ((infected + new_infections - new_recoveries + recovered + deaths) > population)
                {
                    new_infections = population - infected - recovered - deaths + new_recoveries;
                    if (new_infections < 0)
                        new_infections = 0;
                }

                infected = infected + static_cast<int>(new_infections) - static_cast<int>(new_recoveries);
                recovered += static_cast<int>(new_recoveries);

                // Aggregate deaths from all regions
                deaths = 0;
                for (const auto &entry : pandemicSim.regions)
                {
                    deaths += entry.second->deaths;
                }

                // Ensure counts are within bounds
                if (infected < 0)
                    infected = 0;
                if (recovered > population - deaths)
                    recovered = population - deaths;
                if (infected + recovered + deaths > population)
                {
                    infected = population - recovered - deaths;
                }

                // Record infection data
                infectionStats.recordInfection(infected);

                // Record recovery data
                recoverySim.recordRecovery(static_cast<int>(gamma * 100)); // Assuming gamma as percentage

                // Display current state
                cout << "Current Infected: " << infected << "\n";
                cout << "Total Recovered: " << recovered << "\n";
                cout << "Total Deaths: " << deaths << "\n"; // Display total deaths
                cout << "Susceptible Population: " << (population - infected - recovered - deaths) << "\n";

                current_day += 1;
            }
            cout << "Simulation completed.\n";
            break;
        }
        case 6:
        {
            // View Statistics
            cout << "\n=== Simulation Statistics ===\n";
            infectionStats.displayTrends();
            recoverySim.printRecoveryStatistics();
            cout << "Total Population: " << population << "\n";
            cout << "Current Infected: " << infected << "\n";
            cout << "Total Recovered: " << recovered << "\n";
            cout << "Total Deaths: " << deaths << "\n"; // Added total deaths display
            cout << "Susceptible Population: " << (population - infected - recovered - deaths) << "\n";
            break;
        }
        case 7:
        {
            // Customize Infection Rate
            cout << "\n=== Customize Infection Rate ===\n";
            cout << "1. Add a new Infection Rate\n";
            cout << "2. View Current Infection Rates\n";
            cout << "3. Remove the Highest Infection Rate\n";
            cout << "4. Back to Main Menu\n";
            cout << "Select an option: ";

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
                    cout << "Invalid infection rate. Please enter a value between 0 and 100.\n";
                    break;
                }
                customInfectionRates.addInfectionRate(new_beta);
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
                cout << "Highest infection rate removed.\n";
                break;
            }
            case 4:
            {
                // Back to Main Menu
                break;
            }
            default:
                cout << "Please select a valid option (1-4).\n";
            }

            break;
        }
        case 8:
        {
            // Customize Recovery Rate
            cout << "\n=== Customize Recovery Rate ===\n";
            cout << "1. Add a new Recovery Rate\n";
            cout << "2. View Current Recovery Rates\n";
            cout << "3. Back to Main Menu\n";
            cout << "Select an option: ";

            int sub_choice;
            cin >> sub_choice;

            switch (sub_choice)
            {
            case 1:
            {
                double new_gamma;
                cout << "Enter new recovery rate (gamma, e.g., 0.1): ";
                cin >> new_gamma;
                if (new_gamma < 0.0 || new_gamma > 1.0)
                {
                    cout << "Invalid recovery rate. Please enter a value between 0 and 1.\n";
                    break;
                }
                recoverySim.recordRecovery(static_cast<int>(new_gamma * 100)); // Assuming gamma as percentage
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
                // Back to Main Menu
                break;
            }
            default:
                cout << "Please select a valid option (1-3).\n";
            }

            break;
        }
        case 9:
        {
            // Manage Pandemic Spread
            cout << "\n=== Manage Pandemic Spread ===\n";
            cout << "1. Add a new Region\n";
            cout << "2. Connect Regions\n";
            cout << "3. Simulate Infection Spread with Statistics\n"; // Updated option description
            cout << "4. Display All Regions\n";
            cout << "5. Display Pandemic Statistics\n";
            cout << "6. Back to Main Menu\n";
            cout << "Select an option: ";

            int spread_choice;
            cin >> spread_choice;
            cin.ignore(); // To consume the newline
            bool flag = true;

            while (flag)
            {
                switch (spread_choice)
                {
                case 1:
                {
                    string regionName;
                    int regionPopulation;
                    cout << "Enter the name of the new region: ";
                    getline(cin, regionName);
                    cout << "Enter the population of " << regionName << ": ";
                    cin >> regionPopulation;
                    cin.ignore();
                    pandemicSim.addRegion(regionName, regionPopulation);
                    cout << "Region added successfully.\n";
                    break;
                }
                case 2:
                {
                    string region1, region2;
                    cout << "Enter the first region to connect: ";
                    getline(cin, region1);
                    cout << "Enter the second region to connect: ";
                    getline(cin, region2);
                    pandemicSim.connectRegions(region1, region2);
                    break;
                }
                case 3:
                {
                    string startRegion;
                    double initialInfectionRate;
                    cout << "Enter the starting region for infection simulation: ";
                    getline(cin, startRegion);
                    cout << "Enter the initial infection rate (percentage): ";
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
                    cout << "Please select a valid option (1-6).\n";
                }

                if (flag)
                {
                    cout << "\n=== Manage Pandemic Spread ===\n";
                    cout << "1. Add a new Region\n";
                    cout << "2. Connect Regions\n";
                    cout << "3. Simulate Infection Spread with Statistics\n";
                    cout << "4. Display All Regions\n";
                    cout << "5. Display Pandemic Statistics\n";
                    cout << "6. Back to Main Menu\n";
                    cout << "Select an option: ";
                    cin >> spread_choice;
                    cin.ignore(); // To consume the newline
                }
            }
            break;
        }
        case 10:
        {
            // Exit
            exit_program = true;
            cout << "Exiting simulation. Goodbye!\n";
            break;
        }
        default:
            cout << "Please select a valid option (1-10).\n";
        }
    }

    // Clean up dynamically allocated memory for PandemicSimulation
    for (auto &entry : pandemicSim.regions)
    {
        delete entry.second;
    }

    return 0;
}