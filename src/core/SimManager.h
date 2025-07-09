//
// Created by zhou on 25-6-12.
//

#ifndef SIMMANAGER_H
#define SIMMANAGER_H

#include "SimTime.h"
#include "SystemStateHub.h"

namespace core {

class SimManager {
    double convergence_threshold = 0.001;
    int max_iterations = 50;
    double timestep=3600.0;

    json run_periods;
public:
    SimTime time;

    SimManager();
    ~SimManager()=default;

    void parse_file(const std::string& in_file);

    bool run_a_step(const SimTime& time);
    static bool check_global_convergence();
    void run();
};

} // core

#endif //SIMMANAGER_H
