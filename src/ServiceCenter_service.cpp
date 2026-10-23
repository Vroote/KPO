#include "ServiceCenter_service.h"

#include <format>

#include "equipment.h"

InspectionResult ServiceCenter::inspect(const Vehicle &vehicle) const {
    if (vehicle.wearPercent() > kMaxWearPercent) {
        return {false, std::format("износ {} % > {} %", vehicle.wearPercent(), kMaxWearPercent)};
    }
    if (const auto *consumer = dynamic_cast<const IEnergyConsumer *>(&vehicle)) {
        if (consumer->dailyEnergyKwh() > kMaxDailyEnergyKwh) {
            return {false, std::format("расход {} > {} кВт·ч/сут", consumer->dailyEnergyKwh(), kMaxDailyEnergyKwh)};
        }
    }
    return {true, ""};
}
