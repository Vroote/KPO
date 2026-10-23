#pragma once

#include <string>

class Vehicle;

struct InspectionResult {
    bool accepted = false;
    std::string reason;
};

class IInspectionService {
public:
    virtual ~IInspectionService() = default;

    virtual InspectionResult inspect(const Vehicle &vehicle) const = 0;
};
