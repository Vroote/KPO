#include <gtest/gtest.h>

#include "ServiceCenter_service.h"
#include "equipment.h"

TEST(InspectionResult, IS01_DefaultIsRejectedWithoutReason) {
    const InspectionResult result;
    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.reason.empty());
}

TEST(ServiceCenter, SC01_Limits) {
    EXPECT_DOUBLE_EQ(ServiceCenter::kMaxWearPercent, 40.0);
    EXPECT_DOUBLE_EQ(ServiceCenter::kMaxDailyEnergyKwh, 2.0);
}

TEST(ServiceCenter, SC02_WearOnLimitIsAccepted) {
    const InspectionResult result = ServiceCenter().inspect(Scooter("A", 3, 40));
    EXPECT_TRUE(result.accepted);
    EXPECT_TRUE(result.reason.empty());
}

TEST(ServiceCenter, SC03_WearAboveLimitIsRejected) {
    const InspectionResult result = ServiceCenter().inspect(Bicycle("A", 3, 55));
    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.reason, "износ 55 % > 40 %");
}

TEST(ServiceCenter, SC04_FractionalWearInReason) {
    EXPECT_EQ(ServiceCenter().inspect(Scooter("A", 3, 40.25)).reason, "износ 40.25 % > 40 %");
}

TEST(ServiceCenter, SC05_EnergyOnLimitIsAccepted) {
    EXPECT_TRUE(ServiceCenter().inspect(ElectricScooter("A", 3, 10, 2)).accepted);
}

TEST(ServiceCenter, SC06_EnergyAboveLimitIsRejected) {
    const InspectionResult result = ServiceCenter().inspect(ElectricBicycle("A", 3, 10, 2.5));
    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.reason, "расход 2.5 > 2 кВт·ч/сут");
}

TEST(ServiceCenter, SC07_WearIsCheckedFirst) {
    EXPECT_EQ(ServiceCenter().inspect(ElectricScooter("A", 3, 55, 3)).reason, "износ 55 % > 40 %");
}

TEST(ServiceCenter, SC08_NonElectricIsAccepted) {
    const InspectionResult result = ServiceCenter().inspect(Bicycle("A", 3, 0));
    EXPECT_TRUE(result.accepted);
    EXPECT_TRUE(result.reason.empty());
}

TEST(ServiceCenter, SC09_WorksThroughInterface) {
    const ServiceCenter center;
    const IInspectionService &service = center;
    const InspectionResult result = service.inspect(ElectricBicycle("A", 3, 10, 2.5));
    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.reason, "расход 2.5 > 2 кВт·ч/сут");
}
