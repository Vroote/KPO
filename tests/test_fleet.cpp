#include <memory>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "Fleet.h"
#include "test_support.h"

TEST(Fleet, FL01_FormatInventoryNumber) {
    EXPECT_EQ(formatInventoryNumber(1), "ВК-000001");
    EXPECT_EQ(formatInventoryNumber(999999), "ВК-999999");
    EXPECT_EQ(formatInventoryNumber(1234567), "ВК-1234567");
}

TEST(Fleet, FL02_NewFleetIsEmpty) {
    const Fleet fleet;
    EXPECT_TRUE(fleet.vehicles().empty());
    EXPECT_TRUE(fleet.items().empty());
}

TEST(Fleet, FL03_SharedCounterForVehiclesAndItems) {
    Fleet fleet;
    EXPECT_EQ(fleet.addVehicle(std::make_unique<Scooter>("A", 3, 1)), "ВК-000001");
    EXPECT_EQ(fleet.addItem(std::make_unique<Helmet>()), "ВК-000002");
    EXPECT_EQ(fleet.addVehicle(std::make_unique<Bicycle>("B", 3, 1)), "ВК-000003");
}

TEST(Fleet, FL04_ObjectGetsTheReturnedNumber) {
    Fleet fleet;
    fleet.addItem(std::make_unique<Helmet>());
    fleet.addVehicle(std::make_unique<Scooter>("A", 3, 1));
    EXPECT_EQ(fleet.items()[0]->inventoryNumber(), 1);
    EXPECT_EQ(fleet.vehicles()[0]->inventoryNumber(), 2);
}

TEST(Fleet, FL05_NullptrIsRejected) {
    Fleet fleet;
    expectThrowWithMessage<std::invalid_argument>([&] { fleet.addVehicle(nullptr); },
                                                  "Нельзя добавить пустой транспорт");
    expectThrowWithMessage<std::invalid_argument>([&] { fleet.addItem(nullptr); }, "Нельзя добавить пустой предмет");
}

TEST(Fleet, FL06_NullptrDoesNotSpendNumber) {
    Fleet fleet;
    EXPECT_THROW(fleet.addVehicle(nullptr), std::invalid_argument);
    EXPECT_EQ(fleet.addVehicle(std::make_unique<Scooter>("A", 3, 1)), "ВК-000001");
}

TEST(Fleet, FL07_NumberedObjectIsRejectedWithoutGap) {
    Fleet fleet;
    fleet.addItem(std::make_unique<Helmet>());
    auto station = std::make_unique<DockStation>(1.0);
    station->assignInventoryNumber(99);
    expectThrowWithMessage<std::logic_error>([&] { fleet.addItem(std::move(station)); },
                                             "Инвентарный номер уже присвоен");
    EXPECT_EQ(fleet.items().size(), 1u);
    EXPECT_EQ(fleet.addVehicle(std::make_unique<Scooter>("A", 3, 1)), "ВК-000002");
}

TEST(Fleet, FL08_ListsKeepInsertionOrder) {
    Fleet fleet;
    fleet.addVehicle(std::make_unique<Scooter>("Первый", 3, 1));
    fleet.addItem(std::make_unique<DockStation>(1.0));
    fleet.addVehicle(std::make_unique<Bicycle>("Второй", 3, 1));
    fleet.addItem(std::make_unique<Helmet>());

    const std::vector<const Vehicle *> vehicles = fleet.vehicles();
    ASSERT_EQ(vehicles.size(), 2u);
    EXPECT_EQ(vehicles[0]->nickname(), "Первый");
    EXPECT_EQ(vehicles[1]->nickname(), "Второй");

    const std::vector<const Item *> items = fleet.items();
    ASSERT_EQ(items.size(), 2u);
    EXPECT_EQ(items[0]->name(), "Док-станция");
    EXPECT_EQ(items[1]->name(), "Шлем");
}

TEST(Fleet, FL09_PointersStayValidAfterGrowth) {
    Fleet fleet;
    fleet.addVehicle(std::make_unique<Scooter>("Первый", 3, 1));
    const Vehicle *first = fleet.vehicles()[0];
    for (int i = 0; i < 100; ++i) {
        fleet.addVehicle(std::make_unique<Bicycle>("B", 3, 1));
    }
    EXPECT_EQ(fleet.vehicles()[0], first);
    EXPECT_EQ(first->name(), "Самокат «Первый»");
}
