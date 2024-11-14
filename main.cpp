#include <iostream>
#include <bits/stdc++.h>
using namespace std;

class InfectionSpread
{
    // yaafay
};

#include <iostream>
#include <vector>
#include <string>
#include <functional>

class InfectionTrendsAndStatistics
{
    // ali
};

class CustomizableInfectionRates
{
    // ali
};

class RecoverySimulation
{
    // adil
};

class AddandRemoveRegions
{
    // ali
};

class OptimizedRouteandPolicySuggestions
{
    // yaafay
};

class AdjustmentsBasedonNewData
{
    // adil
};

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
        // Calculate end day based on effectiveness percentage
        double duration = (effectiveness / 100.0) * simulation_duration;
        double end = start + duration;

        interventions.emplace_back(name, start, end, effectiveness, type);
        interventions.back().activation_condition = activate_cond;
        interventions.back().deactivation_condition = deactivate_cond;
    }

    void applyInterventions(double &beta, double &gamma, double current_DAYS, double infected)
    {
        double original_beta = beta; // Store the original beta value to reset each day

        for (auto &intervention : interventions)
        {
            bool is_active = intervention.start_DAYS <= current_DAYS && current_DAYS <= intervention.end_DAYS;

            if (!is_active && intervention.activation_condition(current_DAYS, infected, beta))
            {
                intervention.start_DAYS = current_DAYS;
                is_active = true;
                cout << "Intervention " << intervention.name << " activated.\n";
            }
            if (is_active && intervention.deactivation_condition(current_DAYS, infected, beta))
            {
                intervention.end_DAYS = current_DAYS;
                is_active = false;
                cout << "Intervention " << intervention.name << " deactivated.\n";
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
                beta = original_beta; // Reset to the original beta if no active intervention
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

void displayIntervention()
{
    DynamicIntervention model;
    int start, eff, choice, simulation_duration = 20;
    string name, type;

    cout << "Enter the name of Pandemic: ";
    getline(cin, name);
    cout << "Enter the number of ways you want to add to prevent it spreading (1-3): " << endl;
    cin >> choice;

    while (choice--)
    {
        cout << "How many days passed since the start of the pandemic: ";
        cin >> start;
        // Bhai this is the effectiveness of the intervention means that user have to know how much effective the intervention is.
        // in covid-19 the effectiveness of the vaccine is 95% so user have to know this.

        cout << "Enter the effectiveness of the intervention (%): ";
        cin >> eff;
        cin.ignore();
        cout << "Enter the type of the intervention you want to apply (quarantine(q), vaccination(v), lockdown(d)): ";
        getline(cin, type);

        if (type == "q")
            type = "quarantine";
        else if (type == "v")
            type = "vaccination";
        else if (type == "d")
            type = "lockdown";
        else
        {
            cout << "Invalid Input" << endl;
            continue;
        }

        model.addIntervention(name, start, eff, type, [](double current_DAYS, double infected, double beta)
                              { return current_DAYS == 5; }, [](double current_DAYS, double infected, double beta)
                              { return infected > 100; }, simulation_duration);

        double beta = 0.3;  //(infection rate)
        double gamma = 0.1; //(Recovery rate)
        double infected = 60;
        for (double day = 0; day <= simulation_duration; day += 1)
        {
            model.applyInterventions(beta, gamma, day, infected);
            cout << "Day: " << day << " | Infection Rate (Beta): " << beta * 100 << "%"
                 << " | Recovery Rate (Gamma): " << gamma * 100 << "%" << "\n";
            infected += 5;
        }

        model.printInterventions();
    }
}

int main()
{
    displayIntervention();
    return 0;
}
