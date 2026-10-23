#pragma once

#include <memory>
#include <string>
#include <vector>

#include "equipment.h"

std::string formatInventoryNumber(int number);

class Fleet {
public:
    std::string addVehicle(std::unique_ptr<Vehicle> vehicle);
    std::string addItem(std::unique_ptr<Item> item);

    std::vector<const Vehicle *> vehicles() const;
    std::vector<const Item *> items() const;

private:
    std::vector<std::unique_ptr<Vehicle>> vehicles_;
    std::vector<std::unique_ptr<Item>> items_;
    int lastNumber_ = 0;
};
