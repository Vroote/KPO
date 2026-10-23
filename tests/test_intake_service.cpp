#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "Intake_service.h"
#include "ServiceCenter_service.h"
#include "test_support.h"

namespace {

    class FakeInspection final : public IInspectionService {
    public:
        explicit FakeInspection(InspectionResult result) : result_(std::move(result)) {}

        InspectionResult inspect(const Vehicle &) const override {
            ++calls_;
            return result_;
        }

        int calls() const { return calls_; }

    private:
        InspectionResult result_;
        mutable int calls_ = 0;
    };

    std::unique_ptr<Vehicle> makeScooter() { return std::make_unique<Scooter>("Молния", 3, 10); }

} // namespace

TEST(IntakeService, IN01_AcceptedVehicleGoesToFleet) {
    FakeInspection inspection({true, ""});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    const IntakeResult result = intake.accept(makeScooter());
    EXPECT_TRUE(result.accepted);
    EXPECT_EQ(result.inventoryNumber, "ВК-000001");
    EXPECT_TRUE(result.reason.empty());
    ASSERT_EQ(fleet.vehicles().size(), 1u);
    EXPECT_EQ(fleet.vehicles()[0]->name(), "Самокат «Молния»");
}

TEST(IntakeService, IN02_RejectedVehicleIsNotAdded) {
    FakeInspection inspection({false, "X"});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    const IntakeResult result = intake.accept(makeScooter());
    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.inventoryNumber.empty());
    EXPECT_EQ(result.reason, "X");
    EXPECT_TRUE(fleet.vehicles().empty());
}

TEST(IntakeService, IN03_RejectionDoesNotSpendNumber) {
    FakeInspection reject({false, "X"});
    FakeInspection accept({true, ""});
    Fleet fleet;
    IntakeService(reject, fleet).accept(makeScooter());
    EXPECT_EQ(IntakeService(accept, fleet).accept(makeScooter()).inventoryNumber, "ВК-000001");
}

TEST(IntakeService, IN04_InspectIsCalledOnce) {
    FakeInspection inspection({true, ""});
    Fleet fleet;
    IntakeService(inspection, fleet).accept(makeScooter());
    EXPECT_EQ(inspection.calls(), 1);
}

TEST(IntakeService, IN05_NullptrIsRejectedBeforeInspect) {
    FakeInspection inspection({true, ""});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    expectThrowWithMessage<std::invalid_argument>([&] { intake.accept(nullptr); }, "Нельзя принять пустой транспорт");
    EXPECT_EQ(inspection.calls(), 0);
    EXPECT_TRUE(fleet.vehicles().empty());
}

TEST(IntakeService, IN06_NullptrDoesNotSpendNumber) {
    FakeInspection inspection({true, ""});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    EXPECT_THROW(intake.accept(nullptr), std::invalid_argument);
    EXPECT_EQ(intake.registerItem(std::make_unique<Helmet>()), "ВК-000001");
}

TEST(IntakeService, IN07_RegisterItemSkipsInspection) {
    FakeInspection inspection({false, "X"});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    EXPECT_EQ(intake.registerItem(std::make_unique<Helmet>()), "ВК-000001");
    EXPECT_EQ(inspection.calls(), 0);
    EXPECT_EQ(fleet.items().size(), 1u);
}

TEST(IntakeService, IN08_RegisterNullptrItem) {
    FakeInspection inspection({true, ""});
    Fleet fleet;
    IntakeService intake(inspection, fleet);
    expectThrowWithMessage<std::invalid_argument>([&] { intake.registerItem(nullptr); },
                                                  "Нельзя добавить пустой предмет");
}

TEST(IntakeService, IN09_ReasonOfAcceptedIsPassedThrough) {
    FakeInspection inspection({true, "Y"});
    Fleet fleet;
    const IntakeResult result = IntakeService(inspection, fleet).accept(makeScooter());
    EXPECT_TRUE(result.accepted);
    EXPECT_EQ(result.reason, "Y");
}

TEST(IntakeService, IN10_WorksWithServiceCenter) {
    const ServiceCenter center;
    Fleet fleet;
    IntakeService intake(center, fleet);
    EXPECT_EQ(intake.accept(std::make_unique<Bicycle>("Старик", 3, 55)).reason, "износ 55 % > 40 %");
    EXPECT_EQ(intake.accept(std::make_unique<ElectricScooter>("Ток", 3, 10, 2.5)).reason, "расход 2.5 > 2 кВт·ч/сут");
    EXPECT_TRUE(fleet.vehicles().empty());
}

TEST(IntakeService, IN11_DefaultResult) {
    const IntakeResult result;
    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.inventoryNumber.empty());
    EXPECT_TRUE(result.reason.empty());
}
