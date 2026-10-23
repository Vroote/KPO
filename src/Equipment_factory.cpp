#include "Equipment_factory.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace {

    constexpr std::string_view kScooterId = "scooter";
    constexpr std::string_view kBicycleId = "bicycle";
    constexpr std::string_view kElectricScooterId = "e-scooter";
    constexpr std::string_view kElectricBicycleId = "e-bicycle";

    constexpr std::string_view kHelmetId = "helmet";
    constexpr std::string_view kDockStationId = "dock-station";
    constexpr std::string_view kChargingCabinetId = "charging-cabinet";

} // namespace

VehicleFactory::VehicleFactory() {
    registerType({std::string(kScooterId), std::string(Scooter::kTitle), false}, [](const VehicleParams &params) {
        return std::make_unique<Scooter>(params.name, params.simplicity, params.wear);
    });
    registerType({std::string(kBicycleId), std::string(Bicycle::kTitle), false}, [](const VehicleParams &params) {
        return std::make_unique<Bicycle>(params.name, params.simplicity, params.wear);
    });
    registerType({std::string(kElectricScooterId), std::string(ElectricScooter::kTitle), true},
                 [](const VehicleParams &params) {
                     return std::make_unique<ElectricScooter>(params.name, params.simplicity, params.wear,
                                                              params.energy);
                 });
    registerType({std::string(kElectricBicycleId), std::string(ElectricBicycle::kTitle), true},
                 [](const VehicleParams &params) {
                     return std::make_unique<ElectricBicycle>(params.name, params.simplicity, params.wear,
                                                              params.energy);
                 });
}

std::unique_ptr<Vehicle> VehicleFactory::create(std::string_view kind, const VehicleParams &params) const {
    const Entry *entry = find(kind);
    if (entry == nullptr) {
        throw std::invalid_argument("Неизвестный вид транспорта: " + std::string(kind));
    }
    return entry->creator(params);
}

std::vector<VehicleType> VehicleFactory::types() const {
    std::vector<VehicleType> result;
    result.reserve(entries_.size());
    for (const Entry &entry: entries_) {
        result.push_back(entry.type);
    }
    return result;
}

bool VehicleFactory::contains(std::string_view kind) const { return find(kind) != nullptr; }

void VehicleFactory::registerType(VehicleType type, Creator creator) {
    if (type.id.empty()) {
        throw std::invalid_argument("Идентификатор вида транспорта не может быть пустым");
    }
    if (!creator) {
        throw std::invalid_argument("Создатель для вида «" + type.id + "» не задан");
    }
    if (contains(type.id)) {
        throw std::invalid_argument("Вид транспорта «" + type.id + "» уже зарегистрирован");
    }
    entries_.push_back({std::move(type), std::move(creator)});
}

const VehicleFactory::Entry *VehicleFactory::find(std::string_view kind) const {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [kind](const Entry &entry) { return entry.type.id == kind; });
    return it == entries_.end() ? nullptr : &*it;
}

ItemFactory::ItemFactory() {
    registerType({std::string(kHelmetId), std::string(Helmet::kTitle), false},
                 [](const ItemParams &) { return std::make_unique<Helmet>(); });
    registerType({std::string(kDockStationId), std::string(DockStation::kTitle), true},
                 [](const ItemParams &params) { return std::make_unique<DockStation>(params.energy); });
    registerType({std::string(kChargingCabinetId), std::string(ChargingCabinet::kTitle), true},
                 [](const ItemParams &params) { return std::make_unique<ChargingCabinet>(params.energy); });
}

std::unique_ptr<Item> ItemFactory::create(std::string_view kind, const ItemParams &params) const {
    const Entry *entry = find(kind);
    if (entry == nullptr) {
        throw std::invalid_argument("Неизвестный вид вещи: " + std::string(kind));
    }
    return entry->creator(params);
}

std::vector<ItemType> ItemFactory::types() const {
    std::vector<ItemType> result;
    result.reserve(entries_.size());
    for (const Entry &entry: entries_) {
        result.push_back(entry.type);
    }
    return result;
}

bool ItemFactory::contains(std::string_view kind) const { return find(kind) != nullptr; }

void ItemFactory::registerType(ItemType type, Creator creator) {
    if (type.id.empty()) {
        throw std::invalid_argument("Идентификатор вида вещи не может быть пустым");
    }
    if (!creator) {
        throw std::invalid_argument("Создатель для вида «" + type.id + "» не задан");
    }
    if (contains(type.id)) {
        throw std::invalid_argument("Вид вещи «" + type.id + "» уже зарегистрирован");
    }
    entries_.push_back({std::move(type), std::move(creator)});
}

const ItemFactory::Entry *ItemFactory::find(std::string_view kind) const {
    auto it = std::find_if(entries_.begin(), entries_.end(),
                           [kind](const Entry &entry) { return entry.type.id == kind; });
    return it == entries_.end() ? nullptr : &*it;
}
