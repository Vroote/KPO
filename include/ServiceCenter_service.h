#pragma once

#include "Inspection_service_interface.h"

class ServiceCenter final : public IInspectionService {
public:
    static constexpr double kMaxWearPercent = 40.0;
    static constexpr double kMaxDailyEnergyKwh = 2.0;

    InspectionResult inspect(const Vehicle &vehicle) const override;
};
