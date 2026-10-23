#include "Intake_service.h"

#include <stdexcept>
#include <utility>

IntakeService::IntakeService(const IInspectionService &inspection, Fleet &fleet) :
    inspection_(inspection), fleet_(fleet) {}

IntakeResult IntakeService::accept(std::unique_ptr<Vehicle> vehicle) {
    if (!vehicle) {
        throw std::invalid_argument("Нельзя принять пустой транспорт");
    }
    InspectionResult inspection = inspection_.inspect(*vehicle);
    if (!inspection.accepted) {
        return {false, "", std::move(inspection.reason)};
    }
    std::string number = fleet_.addVehicle(std::move(vehicle));
    return {true, std::move(number), std::move(inspection.reason)};
}

std::string IntakeService::registerItem(std::unique_ptr<Item> item) { return fleet_.addItem(std::move(item)); }
