#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <string_view>

#include "Equipment_factory.h"
#include "Fleet_reports.h"
#include "Intake_service.h"

std::string padRight(std::string_view text, std::size_t width);

class ConsoleApp {
public:
    ConsoleApp(const VehicleFactory &vehicleFactory, const ItemFactory &itemFactory, IntakeService &intake,
               const FleetReports &reports, std::istream &in, std::ostream &out);

    void run();

private:
    void printMenu();
    void types();
    void acceptVehicle();
    void registerItem();
    void summary();
    void beginnerFriendly();
    void inventory();

    std::string readLine(std::string_view prompt);
    int readInt(std::string_view prompt);
    double readDouble(std::string_view prompt);

    const VehicleFactory &vehicleFactory_;
    const ItemFactory &itemFactory_;
    IntakeService &intake_;
    const FleetReports &reports_;
    std::istream &in_;
    std::ostream &out_;
};
