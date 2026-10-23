#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Equipment_factory.h"
#include "test_support.h"

namespace {

    std::unique_ptr<Vehicle> makeBicycle(const VehicleParams &params) {
        return std::make_unique<Bicycle>(params.name, params.simplicity, params.wear);
    }

    std::unique_ptr<Item> makeHelmet(const ItemParams &) { return std::make_unique<Helmet>(); }

    template<typename Type>
    void expectType(const Type &type, const std::string &id, const std::string &title, bool needEnergy) {
        EXPECT_EQ(type.id, id);
        EXPECT_EQ(type.title, title);
        EXPECT_EQ(type.needEnergy, needEnergy);
    }

} // namespace

TEST(VehicleFactory, VF01_BuiltInTypes) {
    const std::vector<VehicleType> types = VehicleFactory().types();
    ASSERT_EQ(types.size(), 4u);
    expectType(types[0], "scooter", "Самокат", false);
    expectType(types[1], "bicycle", "Велосипед", false);
    expectType(types[2], "e-scooter", "Электросамокат", true);
    expectType(types[3], "e-bicycle", "Электровелосипед", true);
}

TEST(VehicleFactory, VF02_Contains) {
    const VehicleFactory factory;
    for (const char *id: {"scooter", "bicycle", "e-scooter", "e-bicycle"}) {
        EXPECT_TRUE(factory.contains(id)) << id;
    }
    EXPECT_FALSE(factory.contains("tank"));
    EXPECT_FALSE(factory.contains(""));
}

TEST(VehicleFactory, VF03_CreateScooter) {
    const std::unique_ptr<Vehicle> vehicle = VehicleFactory().create("scooter", {"Молния", 3, 10, 0});
    ASSERT_NE(dynamic_cast<const Scooter *>(vehicle.get()), nullptr);
    EXPECT_EQ(vehicle->name(), "Самокат «Молния»");
}

TEST(VehicleFactory, VF04_CreateElectricBicycle) {
    const std::unique_ptr<Vehicle> vehicle = VehicleFactory().create("e-bicycle", {"Ток", 5, 20, 1.5});
    const auto *bicycle = dynamic_cast<const ElectricBicycle *>(vehicle.get());
    ASSERT_NE(bicycle, nullptr);
    EXPECT_DOUBLE_EQ(bicycle->dailyEnergyKwh(), 1.5);
}

TEST(VehicleFactory, VF05_EnergyIsIgnoredForNonElectric) {
    EXPECT_NO_THROW(VehicleFactory().create("scooter", {"A", 3, 1, -5}));
}

TEST(VehicleFactory, VF06_UnknownKind) {
    const VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.create("tank", {"A", 3, 1, 0}); },
                                                  "Неизвестный вид транспорта: tank");
}

TEST(VehicleFactory, VF07_ConstructorErrorPassesThrough) {
    const VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.create("scooter", {"x", 0, 1, 0}); },
                                                  "Простота должна быть от 1 до 10");
}

TEST(VehicleFactory, VF08_ElectricConstructorErrorPassesThrough) {
    const VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.create("e-scooter", {"x", 3, 1, -1}); },
                                                  "Суточное потребление энергии не может быть отрицательным");
}

TEST(VehicleFactory, VF09_EmptyIdIsRejected) {
    VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.registerType({"", "Пусто", false}, makeBicycle); },
                                                  "Идентификатор вида транспорта не может быть пустым");
    EXPECT_EQ(factory.types().size(), 4u);
}

TEST(VehicleFactory, VF10_EmptyCreatorIsRejected) {
    VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.registerType({"tandem", "Тандем", false}, nullptr); },
                                                  "Создатель для вида «tandem» не задан");
    EXPECT_FALSE(factory.contains("tandem"));
    EXPECT_EQ(factory.types().size(), 4u);
}

TEST(VehicleFactory, VF11_DuplicateIdIsRejected) {
    VehicleFactory factory;
    expectThrowWithMessage<std::invalid_argument>(
            [&] { factory.registerType({"scooter", "Дубль", false}, makeBicycle); },
            "Вид транспорта «scooter» уже зарегистрирован");
    EXPECT_EQ(factory.types().size(), 4u);
    EXPECT_EQ(factory.types()[0].title, "Самокат");
}

TEST(VehicleFactory, VF12_RegisterCustomType) {
    VehicleFactory factory;
    factory.registerType({"tandem", "Тандем", false}, makeBicycle);
    EXPECT_TRUE(factory.contains("tandem"));
    ASSERT_EQ(factory.types().size(), 5u);
    expectType(factory.types().back(), "tandem", "Тандем", false);
    EXPECT_EQ(factory.create("tandem", {"Двойка", 2, 0, 0})->name(), "Велосипед «Двойка»");
}

TEST(ItemFactory, IF01_BuiltInTypes) {
    const std::vector<ItemType> types = ItemFactory().types();
    ASSERT_EQ(types.size(), 3u);
    expectType(types[0], "helmet", "Шлем", false);
    expectType(types[1], "dock-station", "Док-станция", true);
    expectType(types[2], "charging-cabinet", "Зарядный шкаф", true);
}

TEST(ItemFactory, IF02_CreateHelmet) {
    const std::unique_ptr<Item> item = ItemFactory().create("helmet", {});
    EXPECT_NE(dynamic_cast<const Helmet *>(item.get()), nullptr);
    EXPECT_EQ(dynamic_cast<const IEnergyConsumer *>(item.get()), nullptr);
}

TEST(ItemFactory, IF03_CreateDockStation) {
    const std::unique_ptr<Item> item = ItemFactory().create("dock-station", {1.5});
    const auto *station = dynamic_cast<const DockStation *>(item.get());
    ASSERT_NE(station, nullptr);
    EXPECT_DOUBLE_EQ(station->dailyEnergyKwh(), 1.5);
}

TEST(ItemFactory, IF04_CreateChargingCabinet) {
    const std::unique_ptr<Item> item = ItemFactory().create("charging-cabinet", {2.3});
    EXPECT_NE(dynamic_cast<const ChargingCabinet *>(item.get()), nullptr);
}

TEST(ItemFactory, IF05_EnergyIsIgnoredForHelmet) { EXPECT_NO_THROW(ItemFactory().create("helmet", {-5})); }

TEST(ItemFactory, IF06_ConstructorErrorPassesThrough) {
    const ItemFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.create("dock-station", {-1}); },
                                                  "Суточное потребление энергии не может быть отрицательным");
}

TEST(ItemFactory, IF07_UnknownKind) {
    const ItemFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.create("tank", {}); }, "Неизвестный вид вещи: tank");
}

TEST(ItemFactory, IF08_RegisterTypeErrors) {
    ItemFactory factory;
    expectThrowWithMessage<std::invalid_argument>([&] { factory.registerType({"", "Пусто", false}, makeHelmet); },
                                                  "Идентификатор вида вещи не может быть пустым");
    expectThrowWithMessage<std::invalid_argument>([&] { factory.registerType({"lock", "Замок", false}, nullptr); },
                                                  "Создатель для вида «lock» не задан");
    expectThrowWithMessage<std::invalid_argument>([&] { factory.registerType({"helmet", "Дубль", false}, makeHelmet); },
                                                  "Вид вещи «helmet» уже зарегистрирован");
    EXPECT_EQ(factory.types().size(), 3u);
}
