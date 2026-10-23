#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "equipment.h"

struct VehicleParams {
    std::string name;
    int simplicity = 0;
    double wear = 0.0;
    double energy = 0.0;
};

struct VehicleType {
    std::string id;
    std::string title;
    bool needEnergy = false;
};

class VehicleFactory {
public:
    using Creator = std::function<std::unique_ptr<Vehicle>(const VehicleParams &)>;

    VehicleFactory();

    std::unique_ptr<Vehicle> create(std::string_view kind, const VehicleParams &params) const;
    std::vector<VehicleType> types() const;
    bool contains(std::string_view kind) const;
    void registerType(VehicleType type, Creator creator);

private:
    struct Entry {
        VehicleType type;
        Creator creator;
    };

    const Entry *find(std::string_view kind) const;

    std::vector<Entry> entries_;
};

struct ItemParams {
    double energy = 0.0;
};

struct ItemType {
    std::string id;
    std::string title;
    bool needEnergy = false;
};

class ItemFactory {
public:
    using Creator = std::function<std::unique_ptr<Item>(const ItemParams &)>;

    ItemFactory();

    std::unique_ptr<Item> create(std::string_view kind, const ItemParams &params) const;
    std::vector<ItemType> types() const;
    bool contains(std::string_view kind) const;
    void registerType(ItemType type, Creator creator);

private:
    struct Entry {
        ItemType type;
        Creator creator;
    };

    const Entry *find(std::string_view kind) const;

    std::vector<Entry> entries_;
};
