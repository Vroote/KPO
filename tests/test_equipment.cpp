#include <limits>
#include <optional>
#include <stdexcept>

#include <gtest/gtest.h>

#include "equipment.h"
#include "test_support.h"

namespace {

    constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
    constexpr double kInfinity = std::numeric_limits<double>::infinity();

    const std::string kEmptyNickname = "Кличка не может быть пустой";
    const std::string kBadSimplicity = "Простота должна быть от 1 до 10";
    const std::string kNaNWear = "Износ не может быть NaN";
    const std::string kBadWear = "Износ должен быть от 0 до 100 процентов";
    const std::string kInfiniteEnergy = "Суточное потребление энергии должно быть конечным числом";
    const std::string kNegativeEnergy = "Суточное потребление энергии не может быть отрицательным";
    const std::string kAlreadyAssigned = "Инвентарный номер уже присвоен";
    const std::string kNotPositive = "Инвентарный номер должен быть положительным";

    bool isEnergyConsumer(const IInventoriable &object) {
        return dynamic_cast<const IEnergyConsumer *>(&object) != nullptr;
    }

} // namespace

TEST(Equipment, EQ01_ScooterKeepsConstructorValues) {
    const Scooter scooter("Молния", 3, 12.5);
    EXPECT_EQ(scooter.name(), "Самокат «Молния»");
    EXPECT_EQ(scooter.nickname(), "Молния");
    EXPECT_EQ(scooter.simplicity(), 3);
    EXPECT_DOUBLE_EQ(scooter.wearPercent(), 12.5);
}

TEST(Equipment, EQ02_KindTitles) {
    EXPECT_EQ(Scooter::kTitle, "Самокат");
    EXPECT_EQ(Bicycle::kTitle, "Велосипед");
    EXPECT_EQ(ElectricScooter::kTitle, "Электросамокат");
    EXPECT_EQ(ElectricBicycle::kTitle, "Электровелосипед");
    EXPECT_EQ(Helmet::kTitle, "Шлем");
    EXPECT_EQ(DockStation::kTitle, "Док-станция");
    EXPECT_EQ(ChargingCabinet::kTitle, "Зарядный шкаф");
}

TEST(Equipment, EQ03_KindTitleMatchesKTitle) {
    EXPECT_EQ(Scooter("A", 1, 0).kindTitle(), Scooter::kTitle);
    EXPECT_EQ(Bicycle("A", 1, 0).kindTitle(), Bicycle::kTitle);
    EXPECT_EQ(ElectricScooter("A", 1, 0, 0).kindTitle(), ElectricScooter::kTitle);
    EXPECT_EQ(ElectricBicycle("A", 1, 0, 0).kindTitle(), ElectricBicycle::kTitle);
    EXPECT_EQ(Helmet().kindTitle(), Helmet::kTitle);
    EXPECT_EQ(DockStation(0).kindTitle(), DockStation::kTitle);
    EXPECT_EQ(ChargingCabinet(0).kindTitle(), ChargingCabinet::kTitle);
}

TEST(Equipment, EQ04_EmptyNicknameIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("", 3, 1); }, kEmptyNickname);
    expectThrowWithMessage<std::invalid_argument>([] { Bicycle("   ", 3, 1); }, kEmptyNickname);
    expectThrowWithMessage<std::invalid_argument>([] { ElectricScooter("\t\r\n", 3, 1, 1); }, kEmptyNickname);
}

TEST(Equipment, EQ05_SimplicityOutOfRangeIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("A", 0, 1); }, kBadSimplicity);
    expectThrowWithMessage<std::invalid_argument>([] { Bicycle("A", 11, 1); }, kBadSimplicity);
}

TEST(Equipment, EQ06_SimplicityBoundsAreAccepted) {
    EXPECT_EQ(Scooter("A", 1, 1).simplicity(), 1);
    EXPECT_EQ(Scooter("A", 10, 1).simplicity(), 10);
}

TEST(Equipment, EQ07_WearOutOfRangeIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("A", 3, -0.1); }, kBadWear);
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("A", 3, 100.1); }, kBadWear);
}

TEST(Equipment, EQ08_WearBoundsAreAccepted) {
    EXPECT_DOUBLE_EQ(Scooter("A", 3, 0).wearPercent(), 0.0);
    EXPECT_DOUBLE_EQ(Scooter("A", 3, 100).wearPercent(), 100.0);
}

TEST(Equipment, EQ09_NaNWearIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("A", 3, kNaN); }, kNaNWear);
}

TEST(Equipment, EQ10_InfiniteWearIsOutOfRange) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("A", 3, kInfinity); }, kBadWear);
}

TEST(Equipment, EQ11_NegativeEnergyIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { ElectricScooter("A", 3, 1, -1); }, kNegativeEnergy);
    expectThrowWithMessage<std::invalid_argument>([] { ElectricBicycle("A", 3, 1, -1); }, kNegativeEnergy);
}

TEST(Equipment, EQ12_NonFiniteEnergyIsRejected) {
    expectThrowWithMessage<std::invalid_argument>([] { ElectricScooter("A", 3, 1, kNaN); }, kInfiniteEnergy);
    expectThrowWithMessage<std::invalid_argument>([] { ElectricScooter("A", 3, 1, kInfinity); }, kInfiniteEnergy);
    expectThrowWithMessage<std::invalid_argument>([] { ElectricBicycle("A", 3, 1, -kInfinity); }, kInfiniteEnergy);
}

TEST(Equipment, EQ13_ZeroEnergyIsAccepted) { EXPECT_DOUBLE_EQ(ElectricScooter("A", 3, 1, 0).dailyEnergyKwh(), 0.0); }

TEST(Equipment, EQ14_SimplicityIsCheckedBeforeEnergy) {
    expectThrowWithMessage<std::invalid_argument>([] { ElectricScooter("A", 0, 1, -1); }, kBadSimplicity);
}

TEST(Equipment, EQ15_NicknameIsCheckedBeforeSimplicity) {
    expectThrowWithMessage<std::invalid_argument>([] { Scooter("", 0, 1); }, kEmptyNickname);
}

TEST(Equipment, EQ16_OnlyElectricKindsConsumeEnergy) {
    EXPECT_FALSE(isEnergyConsumer(Scooter("A", 1, 0)));
    EXPECT_FALSE(isEnergyConsumer(Bicycle("A", 1, 0)));
    EXPECT_TRUE(isEnergyConsumer(ElectricScooter("A", 1, 0, 0)));
    EXPECT_TRUE(isEnergyConsumer(ElectricBicycle("A", 1, 0, 0)));
    EXPECT_FALSE(isEnergyConsumer(Helmet()));
    EXPECT_TRUE(isEnergyConsumer(DockStation(0)));
    EXPECT_TRUE(isEnergyConsumer(ChargingCabinet(0)));
}

TEST(Equipment, EQ17_ItemNameIsKindTitle) {
    EXPECT_EQ(Helmet().name(), "Шлем");
    EXPECT_EQ(DockStation(1).name(), "Док-станция");
    EXPECT_EQ(ChargingCabinet(1).name(), "Зарядный шкаф");
}

TEST(Equipment, EQ18_ElectricItemsValidateEnergy) {
    expectThrowWithMessage<std::invalid_argument>([] { DockStation(-1); }, kNegativeEnergy);
    expectThrowWithMessage<std::invalid_argument>([] { ChargingCabinet{kNaN}; }, kInfiniteEnergy);
}

TEST(Equipment, EQ19_NewObjectHasNoInventoryNumber) {
    EXPECT_EQ(Scooter("A", 1, 0).inventoryNumber(), std::nullopt);
    EXPECT_EQ(Helmet().inventoryNumber(), std::nullopt);
}

TEST(Equipment, EQ20_AssignInventoryNumber) {
    Scooter scooter("A", 1, 0);
    Helmet helmet;
    scooter.assignInventoryNumber(7);
    helmet.assignInventoryNumber(8);
    EXPECT_EQ(scooter.inventoryNumber(), 7);
    EXPECT_EQ(helmet.inventoryNumber(), 8);
}

TEST(Equipment, EQ21_SecondAssignmentIsLogicError) {
    Scooter scooter("A", 1, 0);
    Helmet helmet;
    scooter.assignInventoryNumber(7);
    helmet.assignInventoryNumber(7);
    expectThrowWithMessage<std::logic_error>([&] { scooter.assignInventoryNumber(8); }, kAlreadyAssigned);
    expectThrowWithMessage<std::logic_error>([&] { helmet.assignInventoryNumber(8); }, kAlreadyAssigned);
    EXPECT_EQ(scooter.inventoryNumber(), 7);
    EXPECT_EQ(helmet.inventoryNumber(), 7);
}

TEST(Equipment, EQ22_NonPositiveNumberIsRejected) {
    Scooter scooter("A", 1, 0);
    expectThrowWithMessage<std::invalid_argument>([&] { scooter.assignInventoryNumber(0); }, kNotPositive);
    expectThrowWithMessage<std::invalid_argument>([&] { scooter.assignInventoryNumber(-1); }, kNotPositive);
    EXPECT_EQ(scooter.inventoryNumber(), std::nullopt);
    scooter.assignInventoryNumber(5);
    EXPECT_EQ(scooter.inventoryNumber(), 5);
}

TEST(Equipment, EQ23_AlreadyAssignedIsCheckedFirst) {
    Helmet helmet;
    helmet.assignInventoryNumber(3);
    expectThrowWithMessage<std::logic_error>([&] { helmet.assignInventoryNumber(0); }, kAlreadyAssigned);
    EXPECT_EQ(helmet.inventoryNumber(), 3);
}
