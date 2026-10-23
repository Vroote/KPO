#pragma once

#include <optional>
#include <string>
#include <string_view>

class IInventoriable {
public:
    virtual ~IInventoriable() = default;

    virtual std::string name() const = 0;
    virtual std::optional<int> inventoryNumber() const = 0;
};

class IEnergyConsumer {
public:
    virtual ~IEnergyConsumer() = default;

    virtual double dailyEnergyKwh() const = 0;
};

class Vehicle : public IInventoriable {
public:
    std::string name() const override;
    std::optional<int> inventoryNumber() const override;
    void assignInventoryNumber(int number);

    const std::string &kindTitle() const;
    const std::string &nickname() const;
    int simplicity() const;
    double wearPercent() const;

protected:
    Vehicle(std::string kindTitle, std::string nickname, int simplicity, double wearPercent);

private:
    std::string kindTitle_;
    std::string nickname_;
    int simplicity_;
    double wearPercent_;
    std::optional<int> inventoryNumber_;
};

class Scooter final : public Vehicle {
public:
    static constexpr std::string_view kTitle = "Самокат";

    Scooter(std::string nickname, int simplicity, double wearPercent);
};

class Bicycle final : public Vehicle {
public:
    static constexpr std::string_view kTitle = "Велосипед";

    Bicycle(std::string nickname, int simplicity, double wearPercent);
};

class ElectricVehicle : public Vehicle, public IEnergyConsumer {
public:
    double dailyEnergyKwh() const override;

protected:
    ElectricVehicle(std::string kindTitle, std::string nickname, int simplicity, double wearPercent,
                    double dailyEnergyKwh);

private:
    double dailyEnergyKwh_;
};

class ElectricScooter final : public ElectricVehicle {
public:
    static constexpr std::string_view kTitle = "Электросамокат";

    ElectricScooter(std::string nickname, int simplicity, double wearPercent, double dailyEnergyKwh);
};

class ElectricBicycle final : public ElectricVehicle {
public:
    static constexpr std::string_view kTitle = "Электровелосипед";

    ElectricBicycle(std::string nickname, int simplicity, double wearPercent, double dailyEnergyKwh);
};

class Item : public IInventoriable {
public:
    std::string name() const override;
    std::optional<int> inventoryNumber() const override;
    void assignInventoryNumber(int number);

    const std::string &kindTitle() const;

protected:
    explicit Item(std::string kindTitle);

private:
    std::string kindTitle_;
    std::optional<int> inventoryNumber_;
};

class Helmet final : public Item {
public:
    static constexpr std::string_view kTitle = "Шлем";

    Helmet();
};

class ElectricItem : public Item, public IEnergyConsumer {
public:
    double dailyEnergyKwh() const override;

protected:
    ElectricItem(std::string kindTitle, double dailyEnergyKwh);

private:
    double dailyEnergyKwh_;
};

class DockStation final : public ElectricItem {
public:
    static constexpr std::string_view kTitle = "Док-станция";

    explicit DockStation(double dailyEnergyKwh);
};

class ChargingCabinet final : public ElectricItem {
public:
    static constexpr std::string_view kTitle = "Зарядный шкаф";

    explicit ChargingCabinet(double dailyEnergyKwh);
};
