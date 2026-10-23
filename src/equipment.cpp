#include "equipment.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace {

    constexpr int kMinSimplicity = 1;
    constexpr int kMaxSimplicity = 10;
    constexpr double kMinWearPercent = 0.0;
    constexpr double kMaxWearPercent = 100.0;

    std::string requireText(std::string value, const char *message) {
        if (value.find_first_not_of(" \t\r\n") == std::string::npos) {
            throw std::invalid_argument(message);
        }
        return value;
    }

    int checkedSimplicity(int simplicity) {
        if (simplicity < kMinSimplicity || simplicity > kMaxSimplicity) {
            throw std::invalid_argument("Простота должна быть от 1 до 10");
        }
        return simplicity;
    }

    double checkedWearPercent(double wearPercent) {
        if (std::isnan(wearPercent)) {
            throw std::invalid_argument("Износ не может быть NaN");
        }
        if (wearPercent < kMinWearPercent || wearPercent > kMaxWearPercent) {
            throw std::invalid_argument("Износ должен быть от 0 до 100 процентов");
        }
        return wearPercent;
    }

    double checkedDailyEnergyKwh(double dailyEnergyKwh) {
        if (!std::isfinite(dailyEnergyKwh)) {
            throw std::invalid_argument("Суточное потребление энергии должно быть конечным числом");
        }
        if (dailyEnergyKwh < 0.0) {
            throw std::invalid_argument("Суточное потребление энергии не может быть отрицательным");
        }
        return dailyEnergyKwh;
    }

    void assignOnce(std::optional<int> &slot, int number) {
        if (slot.has_value()) {
            throw std::logic_error("Инвентарный номер уже присвоен");
        }
        if (number <= 0) {
            throw std::invalid_argument("Инвентарный номер должен быть положительным");
        }
        slot = number;
    }

} // namespace

Vehicle::Vehicle(std::string kindTitle, std::string nickname, int simplicity, double wearPercent) :
    kindTitle_(requireText(std::move(kindTitle), "Вид транспорта не может быть пустым")),
    nickname_(requireText(std::move(nickname), "Кличка не может быть пустой")),
    simplicity_(checkedSimplicity(simplicity)), wearPercent_(checkedWearPercent(wearPercent)) {}

std::string Vehicle::name() const { return kindTitle_ + " «" + nickname_ + "»"; }

std::optional<int> Vehicle::inventoryNumber() const { return inventoryNumber_; }

void Vehicle::assignInventoryNumber(int number) { assignOnce(inventoryNumber_, number); }

const std::string &Vehicle::kindTitle() const { return kindTitle_; }

const std::string &Vehicle::nickname() const { return nickname_; }

int Vehicle::simplicity() const { return simplicity_; }

double Vehicle::wearPercent() const { return wearPercent_; }

Scooter::Scooter(std::string nickname, int simplicity, double wearPercent) :
    Vehicle(std::string(kTitle), std::move(nickname), simplicity, wearPercent) {}

Bicycle::Bicycle(std::string nickname, int simplicity, double wearPercent) :
    Vehicle(std::string(kTitle), std::move(nickname), simplicity, wearPercent) {}

ElectricVehicle::ElectricVehicle(std::string kindTitle, std::string nickname, int simplicity, double wearPercent,
                                 double dailyEnergyKwh) :
    Vehicle(std::move(kindTitle), std::move(nickname), simplicity, wearPercent),
    dailyEnergyKwh_(checkedDailyEnergyKwh(dailyEnergyKwh)) {}

double ElectricVehicle::dailyEnergyKwh() const { return dailyEnergyKwh_; }

ElectricScooter::ElectricScooter(std::string nickname, int simplicity, double wearPercent, double dailyEnergyKwh) :
    ElectricVehicle(std::string(kTitle), std::move(nickname), simplicity, wearPercent, dailyEnergyKwh) {}

ElectricBicycle::ElectricBicycle(std::string nickname, int simplicity, double wearPercent, double dailyEnergyKwh) :
    ElectricVehicle(std::string(kTitle), std::move(nickname), simplicity, wearPercent, dailyEnergyKwh) {}

Item::Item(std::string kindTitle) :
    kindTitle_(requireText(std::move(kindTitle), "Вид предмета не может быть пустым")) {}

std::string Item::name() const { return kindTitle_; }

std::optional<int> Item::inventoryNumber() const { return inventoryNumber_; }

void Item::assignInventoryNumber(int number) { assignOnce(inventoryNumber_, number); }

const std::string &Item::kindTitle() const { return kindTitle_; }

Helmet::Helmet() : Item(std::string(kTitle)) {}

ElectricItem::ElectricItem(std::string kindTitle, double dailyEnergyKwh) :
    Item(std::move(kindTitle)), dailyEnergyKwh_(checkedDailyEnergyKwh(dailyEnergyKwh)) {}

double ElectricItem::dailyEnergyKwh() const { return dailyEnergyKwh_; }

DockStation::DockStation(double dailyEnergyKwh) : ElectricItem(std::string(kTitle), dailyEnergyKwh) {}

ChargingCabinet::ChargingCabinet(double dailyEnergyKwh) : ElectricItem(std::string(kTitle), dailyEnergyKwh) {}
