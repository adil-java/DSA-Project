class DynamicIntervention {
public:
    // Nested structure to represent an intervention
    struct Intervention {
        std::string name;
        double start_time;
        double end_time;
        double effectiveness;
        std::string type;

        // Conditions for triggering or ending the intervention
        std::function<bool(double, double, double)> activation_condition;
        std::function<bool(double, double, double)> deactivation_condition;

        // Constructor for an intervention
        Intervention(const std::string& name, double start, double end, double eff, const std::string& t)
            : name(name), start_time(start), end_time(end), effectiveness(eff), type(t) {}
    };

    // Add a new intervention with conditions
    void addIntervention(const std::string& name, double start, double end, double effectiveness, 
                         const std::string& type,
                         std::function<bool(double, double, double)> activate_cond,
                         std::function<bool(double, double, double)> deactivate_cond) {
        interventions.push_back({name, start, end, effectiveness, type, activate_cond, deactivate_cond});
    }

    // Check and apply interventions based on the current simulation state
    void applyInterventions(double& beta, double& gamma, double current_time, double infected) {
        for (auto& intervention : interventions) {
            bool is_active = intervention.start_time <= current_time && current_time <= intervention.end_time;
            
            // Check activation and deactivation conditions
            if (!is_active && intervention.activation_condition(current_time, infected, beta)) {
                intervention.start_time = current_time;
                is_active = true;
                std::cout << "Intervention " << intervention.name << " activated.\n";
            }
            if (is_active && intervention.deactivation_condition(current_time, infected, beta)) {
                intervention.end_time = current_time;
                is_active = false;
                std::cout << "Intervention " << intervention.name << " deactivated.\n";
            }

            // Apply intervention if active
            if (is_active) {
                if (intervention.type == "quarantine") {
                    beta *= (1 - intervention.effectiveness); // Reduce transmission rate
                } else if (intervention.type == "vaccination") {
                    gamma += intervention.effectiveness; // Increase recovery rate
                } else if (intervention.type == "lockdown") {
                    beta *= (1 - intervention.effectiveness); // Stronger effect on transmission
                }
            }
        }
    }

    // Print details of all interventions
    void printInterventions() const {
        for (const auto& intervention : interventions) {
            std::cout << "Intervention " << intervention.name << " | Type: " << intervention.type
                      << " | Start: " << intervention.start_time << " | End: " << intervention.end_time
                      << " | Effectiveness: " << intervention.effectiveness << "\n";
        }
    }

private:
    std::vector<Intervention> interventions; // List of all interventions
};

int main() {
    DynamicIntervention model;

    // Adding interventions with activation and deactivation conditions
    model.addIntervention("Quarantine", 0, 10, 0.3, "quarantine",
                          [](double current_time, double infected, double beta) { return infected > 50; },
                          [](double current_time, double infected, double beta) { return infected < 20; });

    model.addIntervention("Vaccination", 5, 15, 0.2, "vaccination",
                          [](double current_time, double infected, double beta) { return current_time > 5; },
                          [](double current_time, double infected, double beta) { return current_time > 15; });

    model.addIntervention("Lockdown", 10, 20, 0.5, "lockdown",
                          [](double current_time, double infected, double beta) { return infected > 100; },
                          [](double current_time, double infected, double beta) { return infected < 50; });

    // Simulate time steps
    double beta = 0.3;
    double gamma = 0.1;
    double infected = 60; // Initial infected count
    for (double time = 0; time <= 20; time += 1) {
        model.applyInterventions(beta, gamma, time, infected);
        std::cout << "Time: " << time << " | Beta: " << beta << " | Gamma: " << gamma << "\n";
        infected += 5; // Example: increase infected count over time
    }

    model.printInterventions();

    return 0;
}
