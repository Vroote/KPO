#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "Fleet_reports.h"

namespace {

    class FleetReportsTest : public ::testing::Test {
    protected:
        Fleet fleet_;
        BeginnerPolicy policy_;
        FleetReports reports_{fleet_, policy_};
    };

} // namespace

TEST_F(FleetReportsTest, FR01_EmptyFleet) {
    const FleetSummary summary = reports_.summary();
    EXPECT_EQ(summary.vehicleCount, 0u);
    EXPECT_EQ(summary.itemCount, 0u);
    EXPECT_DOUBLE_EQ(summary.dailyEnergyKwh, 0.0);
    EXPECT_TRUE(reports_.beginnerFriendly().empty());
    EXPECT_TRUE(reports_.inventory().empty());
}

TEST_F(FleetReportsTest, FR02_SummaryCountsAndEnergy) {
    fleet_.addVehicle(std::make_unique<Scooter>("A", 3, 1));
    fleet_.addVehicle(std::make_unique<ElectricScooter>("B", 3, 1, 0.5));
    fleet_.addVehicle(std::make_unique<ElectricBicycle>("C", 3, 1, 1.25));
    fleet_.addItem(std::make_unique<Helmet>());
    fleet_.addItem(std::make_unique<DockStation>(1.5));
    fleet_.addItem(std::make_unique<ChargingCabinet>(2.0));

    const FleetSummary summary = reports_.summary();
    EXPECT_EQ(summary.vehicleCount, 3u);
    EXPECT_EQ(summary.itemCount, 3u);
    EXPECT_NEAR(summary.dailyEnergyKwh, 5.25, 1e-9);
}

TEST_F(FleetReportsTest, FR03_NonElectricHaveNoEnergy) {
    fleet_.addVehicle(std::make_unique<Scooter>("A", 3, 1));
    fleet_.addItem(std::make_unique<Helmet>());
    EXPECT_DOUBLE_EQ(reports_.summary().dailyEnergyKwh, 0.0);
}

TEST_F(FleetReportsTest, FR04_BeginnerFriendlyKeepsOrder) {
    fleet_.addVehicle(std::make_unique<Scooter>("Первый", 7, 1));
    fleet_.addVehicle(std::make_unique<Scooter>("Второй", 3, 1));
    fleet_.addVehicle(std::make_unique<Bicycle>("Третий", 6, 1));

    const std::vector<const Vehicle *> vehicles = reports_.beginnerFriendly();
    ASSERT_EQ(vehicles.size(), 2u);
    EXPECT_EQ(vehicles[0]->nickname(), "Первый");
    EXPECT_EQ(vehicles[1]->nickname(), "Третий");
}

TEST_F(FleetReportsTest, FR05_InventoryIsSortedByNumber) {
    fleet_.addItem(std::make_unique<DockStation>(1.5));
    fleet_.addVehicle(std::make_unique<Scooter>("Молния", 7, 10));
    fleet_.addVehicle(std::make_unique<ElectricScooter>("Искра", 3, 10, 0.5));
    fleet_.addItem(std::make_unique<Helmet>());
    fleet_.addVehicle(std::make_unique<ElectricBicycle>("Ток", 6, 10, 1.25));
    fleet_.addItem(std::make_unique<ChargingCabinet>(2.0));

    const std::vector<InventoryLine> lines = reports_.inventory();
    ASSERT_EQ(lines.size(), 6u);
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(lines[i].inventoryNumber, formatInventoryNumber(i + 1));
    }
    EXPECT_EQ(lines[0].name, "Док-станция");
    EXPECT_FALSE(lines[0].isVehicle);
    EXPECT_EQ(lines[1].name, "Самокат «Молния»");
    EXPECT_TRUE(lines[1].isVehicle);
    EXPECT_EQ(lines[3].name, "Шлем");
    EXPECT_FALSE(lines[3].isVehicle);
    EXPECT_EQ(lines[4].name, "Электровелосипед «Ток»");
    EXPECT_TRUE(lines[4].isVehicle);
}

TEST_F(FleetReportsTest, FR06_FirstLineNumber) {
    fleet_.addVehicle(std::make_unique<Scooter>("A", 3, 1));
    ASSERT_FALSE(reports_.inventory().empty());
    EXPECT_EQ(reports_.inventory()[0].inventoryNumber, "ВК-000001");
}

TEST_F(FleetReportsTest, FR07_ReportsSeeLaterChanges) {
    EXPECT_EQ(reports_.summary().vehicleCount, 0u);
    fleet_.addVehicle(std::make_unique<Scooter>("A", 7, 1));
    EXPECT_EQ(reports_.summary().vehicleCount, 1u);
    EXPECT_EQ(reports_.beginnerFriendly().size(), 1u);
}
