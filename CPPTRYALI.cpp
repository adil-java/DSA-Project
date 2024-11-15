#include <iostream>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>

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

    void displayRegions() const
    {
        cout << "Regions Sorted by Infection Count:\n";
        for (const auto &[count, region] : sorted_regions)
            cout << region << ": " << count << " infections\n";
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

// ------------------------------------ Testing and Simulation Functions ------------------------------------
void testRecoverySimulation()
{
    RecoverySimulation recoverySim(7);
    recoverySim.recordRecovery(5);
    recoverySim.recordRecovery(10);
    recoverySim.recordRecovery(15);

    double gamma = 0.1;
    double recovered = 30;
    for (double day = 0; day <= 20; day += 1)
    {
        cout << "Day: " << day << " | Recovery Rate (Gamma): " << gamma * 100 << "%\n";
        recoverySim.recordRecovery(static_cast<int>(gamma * recovered));
        recovered += gamma * 5;
    }

    recoverySim.printRecoveryStatistics();
}

// ------------------------------------ Main Testing Function ------------------------------------
int main()
{
    DynamicIntervention model;
    int simulation_duration = 20;

    // Instantiate InfectionTrendsAndStatistics
    InfectionTrendsAndStatistics infectionStats(7);

    // Add interventions
    model.addIntervention("Lockdown", 3, 50, "lockdown", [](double days, double infected, double beta) -> bool
                          { return infected > 20; }, [](double days, double infected, double beta) -> bool
                          { return infected < 5; }, simulation_duration);

    double beta = 1.0, gamma = 0.1;
    int infected = 30; // Initial number of infected individuals

    for (double days = 0; days <= simulation_duration; days += 1)
    {
        cout << "\nDay: " << days << "\n";
        model.applyInterventions(beta, gamma, days, infected);
        cout << "Infection Rate (Beta): " << beta << "\n";
        cout << "Recovery Rate (Gamma): " << gamma << "\n";

        // Record infection data
        infectionStats.recordInfection(infected);

        // Update infected count based on a simple model
        infected = static_cast<int>(infected + (beta * infected) - (gamma * infected));
        if (infected < 0)
            infected = 0;
    }

    model.printInterventions();

    // Display infection trends
    infectionStats.displayTrends();

    testRecoverySimulation();

    return 0;
}
