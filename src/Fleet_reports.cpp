#include "Fleet_reports.h"

#include <algorithm>
#include <utility>

namespace {

    template<typename T>
    double sumDailyEnergyKwh(const std::vector<const T *> &objects) {
        double total = 0.0;
        for (const T *object: objects) {
            if (const auto *consumer = dynamic_cast<const IEnergyConsumer *>(object)) {
                total += consumer->dailyEnergyKwh();
            }
        }
        return total;
    }

    struct NumberedLine {
        int number;
        InventoryLine line;
    };

    template<typename T>
    void appendLines(std::vector<NumberedLine> &lines, const std::vector<const T *> &objects, bool isVehicle) {
        for (const T *object: objects) {
            const int number = object->inventoryNumber().value();
            lines.push_back({number, {formatInventoryNumber(number), object->name(), isVehicle}});
        }
    }

} // namespace

FleetReports::FleetReports(const Fleet &fleet, const BeginnerPolicy &policy) : fleet_(fleet), policy_(policy) {}

FleetSummary FleetReports::summary() const {
    const std::vector<const Vehicle *> vehicles = fleet_.vehicles();
    const std::vector<const Item *> items = fleet_.items();
    return {vehicles.size(), items.size(), sumDailyEnergyKwh(vehicles) + sumDailyEnergyKwh(items)};
}

std::vector<const Vehicle *> FleetReports::beginnerFriendly() const {
    std::vector<const Vehicle *> result;
    for (const Vehicle *vehicle: fleet_.vehicles()) {
        if (policy_.isSuitable(*vehicle)) {
            result.push_back(vehicle);
        }
    }
    return result;
}

std::vector<InventoryLine> FleetReports::inventory() const {
    std::vector<NumberedLine> numbered;
    appendLines(numbered, fleet_.vehicles(), true);
    appendLines(numbered, fleet_.items(), false);
    std::ranges::sort(numbered, {}, &NumberedLine::number);

    std::vector<InventoryLine> lines;
    lines.reserve(numbered.size());
    for (NumberedLine &entry: numbered) {
        lines.push_back(std::move(entry.line));
    }
    return lines;
}
