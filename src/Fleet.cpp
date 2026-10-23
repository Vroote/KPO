#include "Fleet.h"

#include <format>
#include <stdexcept>
#include <utility>

namespace {

    template<typename T>
    std::string addNumbered(std::vector<std::unique_ptr<T>> &storage, std::unique_ptr<T> object, int &lastNumber,
                            const char *nullMessage) {
        if (!object) {
            throw std::invalid_argument(nullMessage);
        }
        const int number = lastNumber + 1;
        object->assignInventoryNumber(number);
        storage.push_back(std::move(object));
        lastNumber = number;
        return formatInventoryNumber(number);
    }

    template<typename T>
    std::vector<const T *> view(const std::vector<std::unique_ptr<T>> &storage) {
        std::vector<const T *> result;
        result.reserve(storage.size());
        for (const auto &object: storage) {
            result.push_back(object.get());
        }
        return result;
    }

} // namespace

std::string formatInventoryNumber(int number) { return std::format("ВК-{:06}", number); }

std::string Fleet::addVehicle(std::unique_ptr<Vehicle> vehicle) {
    return addNumbered(vehicles_, std::move(vehicle), lastNumber_, "Нельзя добавить пустой транспорт");
}

std::string Fleet::addItem(std::unique_ptr<Item> item) {
    return addNumbered(items_, std::move(item), lastNumber_, "Нельзя добавить пустой предмет");
}

std::vector<const Vehicle *> Fleet::vehicles() const { return view(vehicles_); }

std::vector<const Item *> Fleet::items() const { return view(items_); }
