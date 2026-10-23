#include "app.h"

#include <algorithm>
#include <format>
#include <istream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace {

    constexpr std::size_t kIdWidth = 12;
    constexpr std::size_t kTitleWidth = 20;
    constexpr std::size_t kNumberWidth = 12;
    constexpr std::size_t kNameWidth = 32;

    struct InputEnded {};

    std::string trim(std::string_view text) {
        const auto first = text.find_first_not_of(" \t\r\n");
        if (first == std::string_view::npos) {
            return {};
        }
        const auto last = text.find_last_not_of(" \t\r\n");
        return std::string(text.substr(first, last - first + 1));
    }

    int parseInt(const std::string &text) {
        std::size_t parsed = 0;
        int value = 0;
        try {
            value = std::stoi(text, &parsed);
        } catch (const std::logic_error &) {
            throw std::invalid_argument("Ожидалось целое число: «" + text + "»");
        }
        if (parsed != text.size()) {
            throw std::invalid_argument("Ожидалось целое число: «" + text + "»");
        }
        return value;
    }

    double parseDouble(const std::string &text) {
        std::size_t parsed = 0;
        double value = 0.0;
        try {
            value = std::stod(text, &parsed);
        } catch (const std::logic_error &) {
            throw std::invalid_argument("Ожидалось число: «" + text + "»");
        }
        if (parsed != text.size()) {
            throw std::invalid_argument("Ожидалось число: «" + text + "»");
        }
        return value;
    }

    std::string formatKwh(double value) {
        std::string text = std::format("{:.2f}", value);
        text.erase(text.find_last_not_of('0') + 1);
        if (text.back() == '.') {
            text.pop_back();
        }
        return text;
    }

    std::size_t textWidth(std::string_view text) {
        return static_cast<std::size_t>(
                std::ranges::count_if(text, [](char c) { return (static_cast<unsigned char>(c) & 0xC0) != 0x80; }));
    }

    template<typename Type>
    void printTypes(std::ostream &out, const std::vector<Type> &types) {
        std::size_t idWidth = kIdWidth;
        std::size_t titleWidth = kTitleWidth;
        for (const Type &type: types) {
            idWidth = std::max(idWidth, textWidth(type.id) + 1);
            titleWidth = std::max(titleWidth, textWidth(type.title) + 1);
        }
        for (const Type &type: types) {
            out << padRight(type.id, idWidth)
                << (type.needEnergy ? padRight(type.title, titleWidth) + "нужен расход энергии" : type.title) << '\n';
        }
    }

} // namespace

std::string padRight(std::string_view text, std::size_t width) {
    const std::size_t length = textWidth(text);
    std::string result(text);
    if (length < width) {
        result.append(width - length, ' ');
    }
    return result;
}

ConsoleApp::ConsoleApp(const VehicleFactory &vehicleFactory, const ItemFactory &itemFactory, IntakeService &intake,
                       const FleetReports &reports, std::istream &in, std::ostream &out) :
    vehicleFactory_(vehicleFactory), itemFactory_(itemFactory), intake_(intake), reports_(reports), in_(in), out_(out) {
}

void ConsoleApp::run() {
    try {
        while (true) {
            printMenu();
            const std::string choice = readLine("> ");
            if (choice == "0") {
                return;
            }
            try {
                if (choice == "1") {
                    types();
                } else if (choice == "2") {
                    acceptVehicle();
                } else if (choice == "3") {
                    registerItem();
                } else if (choice == "4") {
                    summary();
                } else if (choice == "5") {
                    beginnerFriendly();
                } else if (choice == "6") {
                    inventory();
                } else {
                    out_ << "Неизвестный пункт меню: «" << choice << "»\n";
                }
            } catch (const std::invalid_argument &error) {
                out_ << "Ошибка: " << error.what() << '\n';
            }
        }
    } catch (const InputEnded &) {
    }
}

void ConsoleApp::printMenu() {
    out_ << '\n'
         << "1 — виды транспорта\n"
         << "2 — принять транспорт\n"
         << "3 — поставить вещь на баланс\n"
         << "4 — сводка\n"
         << "5 — для новичков\n"
         << "6 — инвентарь\n"
         << "0 — выход\n";
}

void ConsoleApp::types() { printTypes(out_, vehicleFactory_.types()); }

void ConsoleApp::acceptVehicle() {
    const std::vector<VehicleType> types = vehicleFactory_.types();
    printTypes(out_, types);
    const std::string kind = readLine("Вид (id): ");
    const auto type = std::ranges::find(types, kind, &VehicleType::id);
    if (type == types.end()) {
        throw std::invalid_argument("Неизвестный вид транспорта: " + kind);
    }

    VehicleParams params;
    params.name = readLine("Кличка: ");
    params.simplicity = readInt("Простота (1–10): ");
    params.wear = readDouble("Износ, %: ");
    if (type->needEnergy) {
        params.energy = readDouble("Расход, кВт·ч/сут: ");
    }

    std::unique_ptr<Vehicle> vehicle = vehicleFactory_.create(kind, params);
    const std::string name = vehicle->name();
    const IntakeResult result = intake_.accept(std::move(vehicle));
    if (result.accepted) {
        out_ << "Принят: " << result.inventoryNumber << ' ' << name << '\n';
    } else {
        out_ << "Отклонён: " << result.reason << '\n';
    }
}

void ConsoleApp::registerItem() {
    const std::vector<ItemType> types = itemFactory_.types();
    printTypes(out_, types);
    const std::string kind = readLine("Вид (id): ");
    const auto type = std::ranges::find(types, kind, &ItemType::id);
    if (type == types.end()) {
        throw std::invalid_argument("Неизвестный вид вещи: " + kind);
    }

    ItemParams params;
    if (type->needEnergy) {
        params.energy = readDouble("Расход, кВт·ч/сут: ");
    }

    std::unique_ptr<Item> item = itemFactory_.create(kind, params);
    const std::string name = item->name();
    const std::string number = intake_.registerItem(std::move(item));
    out_ << "Поставлен на баланс: " << number << ' ' << name << '\n';
}

void ConsoleApp::summary() {
    const FleetSummary summary = reports_.summary();
    out_ << "Транспорт: " << summary.vehicleCount << ", вещей: " << summary.itemCount
         << ", суточный расход: " << formatKwh(summary.dailyEnergyKwh) << " кВт·ч.\n";
}

void ConsoleApp::beginnerFriendly() {
    const std::vector<const Vehicle *> vehicles = reports_.beginnerFriendly();
    if (vehicles.empty()) {
        out_ << "Подходящего транспорта нет.\n";
        return;
    }
    for (const Vehicle *vehicle: vehicles) {
        out_ << padRight(formatInventoryNumber(vehicle->inventoryNumber().value()), kNumberWidth)
             << padRight(vehicle->name(), kNameWidth) << "простота " << vehicle->simplicity() << '\n';
    }
}

void ConsoleApp::inventory() {
    const std::vector<InventoryLine> lines = reports_.inventory();
    if (lines.empty()) {
        out_ << "Инвентарь пуст.\n";
        return;
    }
    for (const InventoryLine &line: lines) {
        out_ << padRight(line.inventoryNumber, kNumberWidth) << padRight(line.name, kNameWidth)
             << (line.isVehicle ? "транспорт" : "вещь") << '\n';
    }
}

std::string ConsoleApp::readLine(std::string_view prompt) {
    out_ << prompt << std::flush;
    std::string line;
    if (!std::getline(in_, line)) {
        throw InputEnded{};
    }
    return trim(line);
}

int ConsoleApp::readInt(std::string_view prompt) { return parseInt(readLine(prompt)); }

double ConsoleApp::readDouble(std::string_view prompt) { return parseDouble(readLine(prompt)); }
