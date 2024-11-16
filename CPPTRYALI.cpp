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
#include <utility>
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
    bool isIncreasingTrend()
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

// ------------------------------------ Infection Rate Management ------------------------------------
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

// ------------------------------------ Region Management ------------------------------------
class AddandRemoveRegions
{
private:
    unordered_map<string, int> regions;
    map<int, string> sorted_regions;

public:
    void addRegion(const string &region, int initialInfections)
    {
        regions[region] = initialInfections;
        sorted_regions[initialInfections] = region;
    }

    void removeRegion(const string &region)
    {
        if (regions.count(region))
        {
            int infections = regions[region];
            sorted_regions.erase(infections);
            regions.erase(region);
            cout << "Region " << region << " removed.\n";
        }
        else
        {
            cout << "Region " << region << " not found.\n";
        }
    }

    void updateInfections(const string &region, int newInfections)
    {
        if (regions.count(region))
        {
            int oldInfections = regions[region];
            sorted_regions.erase(oldInfections);
            regions[region] = newInfections;
            sorted_regions[newInfections] = region;
        }
        else
        {

            cout << "Region " << region << " not found.\n";
        }
    }

    // void displayRegions() const
    // {
    //     cout << "Regions Sorted by Infection Count:\n";
    //     for (const auto &[count, region] : sorted_regions)
    //         cout << region << ": " << count << " infections\n";
    // }
    void displayRegions() const
    {
        cout << "Regions Sorted by Infection Count:\n";
        for (const auto &pair : sorted_regions)
        {
            const auto &count = pair.first;
            const auto &region = pair.second;
            cout << region << ": " << count << " infections\n";
        }
    }
};

// ------------------------------------ Recovery Simulation ------------------------------------
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
        return recent_recoveries.size() ? static_cast<double>(current_recovery_sum) / recent_recoveries.size() : 0.0;
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

    void printRecoveryStatistics()
    {
        cout << "Average Recovery Rate over Last " << window_size << " Days: " << calculateAverageRecoveryRate() << "\n";
    }
};

// ------------------------------------ Main Interactive Function ------------------------------------
int main()
{
    DynamicIntervention model;
    InfectionTrendsAndStatistics infectionStats(7);
    RecoverySimulation recoverySim(7);
    CustomizableInfectionRates customInfectionRates; // Added for customizable infection rates
    int simulation_duration;
    double beta, gamma;
    int population;
    int infected;
    int recovered = 0; // Initialize recovered population
    bool exit_program = false;
    bool is_simulation_running = false;
    cout << "Do you want to run the simulation? (y/n): ";
    char run_simulation;
    cin >> run_simulation;
    if (run_simulation == 'y')
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
        customInfectionRates.addInfectionRate(beta); // Initialize with initial beta

        cout << "Enter recovery rate (gamma, e.g., 0.1): ";
        cin >> gamma;
        // Assuming gamma is a rate between 0 and 1
        recoverySim.recordRecovery(static_cast<int>(gamma * 100)); // Store as percentage

        bool exit_program = false;
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
        cout << "9. Exit\n";                     // Updated exit option number
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
            // Apply interventions directly to beta and gamma
            model.applyInterventions(beta, gamma, current_day, infected);
            cout << "Infection Rate (Beta): " << beta << "\n";
            cout << "Recovery Rate (Gamma): " << gamma << "\n";

            // Calculate new infections and recoveries
            // Simple model: SIR-like without susceptible tracking
            // New infections: beta * infected
            // New recoveries: gamma * infected
            // Ensure that infections do not exceed population
            double new_infections = beta * infected;
            double new_recoveries = gamma * infected;

            // Adjust to prevent exceeding population
            if ((infected + new_infections - new_recoveries + recovered) > population)
            {
                new_infections = population - infected - recovered + new_recoveries;
                if (new_infections < 0)
                    new_infections = 0;
            }

            infected = infected + static_cast<int>(new_infections) - static_cast<int>(new_recoveries);
            recovered += static_cast<int>(new_recoveries);

            // Ensure counts are within bounds
            if (infected < 0)
                infected = 0;
            if (recovered > population)
                recovered = population;
            if (infected + recovered > population)
            {
                infected = population - recovered;
            }

            // Record infection data
            infectionStats.recordInfection(infected);

            // Record recovery data
            recoverySim.recordRecovery(static_cast<int>(gamma * 100)); // Assuming gamma as percentage

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
                if ((infected + new_infections - new_recoveries + recovered) > population)
                {
                    new_infections = population - infected - recovered + new_recoveries;
                    if (new_infections < 0)
                        new_infections = 0;
                }

                infected = infected + static_cast<int>(new_infections) - static_cast<int>(new_recoveries);
                recovered += static_cast<int>(new_recoveries);

                // Ensure counts are within bounds
                if (infected < 0)
                    infected = 0;
                if (recovered > population)
                    recovered = population;
                if (infected + recovered > population)
                {
                    infected = population - recovered;
                }

                // Record infection data
                infectionStats.recordInfection(infected);

                // Record recovery data
                recoverySim.recordRecovery(static_cast<int>(gamma * 100)); // Assuming gamma as percentage

                // Display current state
                cout << "Current Infected: " << infected << "\n";
                cout << "Total Recovered: " << recovered << "\n";
                cout << "Susceptible Population: " << (population - infected - recovered) << "\n";

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
            cout << "Susceptible Population: " << (population - infected - recovered) << "\n";
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
            // Exit
            exit_program = true;
            cout << "Exiting simulation. Goodbye!\n";
            break;
        }
        default:
            cout << "Please select a valid option (1-9).\n";
        }
    }

    return 0;
}
