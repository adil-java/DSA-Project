#include <iostream>
#include <vector>
#include <deque>
#include <queue>
#include <unordered_map>
#include <map>
#include <string>
#include <functional>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
// ------------------------------------ Infection Trends and Statistics ------------------------------------
class InfectionTrendsAndStatistics
{
private:
    string api_key;
    double current_beta;
    double current_gamma;
    int current_cases;
    double current_growth;
    string current_region;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *userp)
    {
        userp->append((char *)contents, size * nmemb);
        return size * nmemb;
    }

    string makeOpenAIRequest(const string &prompt)
    {
        CURL *curl = curl_easy_init();
        string response;

        if (curl)
        {
            string url = "https://api.openai.com/v1/chat/completions";
            struct curl_slist *headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());

            json request = {
                {"model", "gpt-3.5-turbo"},
                {"messages", {{{"role", "user"}, {"content", prompt}}}}};

            string request_data = request.dump();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            if (res != CURLE_OK)
                return "Error making request";

            try
            {
                json response_json = json::parse(response);
                return response_json["choices"][0]["message"]["content"];
            }
            catch (...)
            {
                return "Error parsing response";
            }
        }
        return "Error initializing CURL";
    }

public:
    InfectionTrendsAndStatistics(const string &api_key) : api_key(api_key) {}

    void updateMetrics(double beta, double gamma, int cases, double growth_rate, const string &region)
    {
        current_beta = beta;
        current_gamma = gamma;
        current_cases = cases;
        current_growth = growth_rate;
        current_region = region;
    }

    void printAnalysis()
    {
        string prompt = "Analyze the following epidemic metrics for " + current_region + ":\n"
                                                                                         "Infection rate (beta): " +
                        to_string(current_beta) + "\n"
                                                  "Recovery rate (gamma): " +
                        to_string(current_gamma) + "\n"
                                                   "Active cases: " +
                        to_string(current_cases) + "\n"
                                                   "Growth rate: " +
                        to_string(current_growth) + "\n"
                                                    "Please provide a brief analysis of the situation and recommendations.";

        string analysis = makeOpenAIRequest(prompt);
        cout << "\nAI Analysis for " << current_region << ":\n"
             << analysis << endl;
    }
};

// ------------------------------------ Main Testing Function ------------------------------------
int main()
{
    int simulation_duration = 20;

    // Initialize DynamicIntervention class
    DynamicIntervention intervention_model;

    // Add a "Lockdown" intervention: It activates when infections exceed 20 and deactivates when they fall below 5.
    intervention_model.addIntervention("Lockdown", 3, 50, "lockdown", [](double days, double infected, double beta)
                                       { return infected > 20; }, [](double days, double infected, double beta)
                                       { return infected < 5; }, simulation_duration);

    // Initialize infection and recovery rates
    double beta = 1.0;    // Infection rate
    double gamma = 0.1;   // Recovery rate (gamma)
    double infected = 30; // Initial number of infected people

    // Run the simulation for the specified duration
    for (double day = 0; day <= simulation_duration; day += 1)
    {
        cout << "\nDay: " << day << "\n";
        intervention_model.applyInterventions(beta, gamma, day, infected);
        cout << "Infection Rate (Beta): " << beta << "\n";
        cout << "Recovery Rate (Gamma): " << gamma << "\n";

        // Simulate infection count change
        infected = infected + (beta * infected) - (gamma * infected); // Simulate infection spread and recovery

        cout << "Number of Infected People: " << infected << "\n";
    }

    // Print all interventions applied
    intervention_model.printInterventions();

    // Test the Recovery Simulation
    RecoverySimulation recovery_sim(7); // 7-day sliding window for recovery simulation
    recovery_sim.recordRecovery(5);
    recovery_sim.recordRecovery(10);
    recovery_sim.recordRecovery(15);

    double recovered = 30;
    for (double day = 0; day <= simulation_duration; day += 1)
    {
        cout << "\nDay: " << day << " | Recovery Rate (Gamma): " << gamma * 100 << "%\n";
        recovery_sim.recordRecovery(static_cast<int>(gamma * recovered));
        recovered += gamma * 5; // Simulating increasing recoveries
    }

    // Print recovery statistics
    recovery_sim.printRecoveryStatistics();

    // Initialize InfectionTrendsAndStatistics class (replace with your OpenAI API key)
    string api_key = "your-openai-api-key-here"; // Replace with your actual API key
    InfectionTrendsAndStatistics ai_analyzer(api_key);

    // Update the metrics
    ai_analyzer.updateMetrics(1.5, 0.1, 500, 0.15, "New York");

    // Get and print AI analysis based on current epidemic data
    ai_analyzer.printAnalysis();

    return 0;
}
