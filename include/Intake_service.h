#pragma once

#include <memory>
#include <string>

#include "Fleet.h"
#include "Inspection_service_interface.h"

struct IntakeResult {
    bool accepted = false;
    std::string inventoryNumber;
    std::string reason;
};

class IntakeService {
public:
    IntakeService(const IInspectionService &inspection, Fleet &fleet);

    IntakeResult accept(std::unique_ptr<Vehicle> vehicle);
    std::string registerItem(std::unique_ptr<Item> item);

private:
    const IInspectionService &inspection_;
    Fleet &fleet_;
};
