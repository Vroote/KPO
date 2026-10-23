#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "Beginner_policy.h"
#include "Fleet.h"

struct FleetSummary {
    std::size_t vehicleCount = 0;
    std::size_t itemCount = 0;
    double dailyEnergyKwh = 0.0;
};

struct InventoryLine {
    std::string inventoryNumber;
    std::string name;
    bool isVehicle = false;
};

class FleetReports {
public:
    FleetReports(const Fleet &fleet, const BeginnerPolicy &policy);

    FleetSummary summary() const;
    std::vector<const Vehicle *> beginnerFriendly() const;
    std::vector<InventoryLine> inventory() const;

private:
    const Fleet &fleet_;
    const BeginnerPolicy &policy_;
};
