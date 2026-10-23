#include <memory>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "Di_container.h"
#include "app.h"

namespace {

    class RejectingInspection final : public IInspectionService {
    public:
        InspectionResult inspect(const Vehicle &) const override { return {false, "тестовый отказ"}; }
    };

    class AcceptingInspection final : public IInspectionService {
    public:
        InspectionResult inspect(const Vehicle &) const override { return {true, ""}; }
    };

} // namespace

TEST(DiContainer, DI01_FleetIsSingleton) {
    std::istringstream in;
    std::ostringstream out;
    auto injector = makeInjector(in, out);
    EXPECT_EQ(&injector.create<Fleet &>(), &injector.create<Fleet &>());
}

TEST(DiContainer, DI02_InspectionIsServiceCenter) {
    std::istringstream in;
    std::ostringstream out;
    auto injector = makeInjector(in, out);
    const IInspectionService &inspection = injector.create<const IInspectionService &>();
    EXPECT_NE(dynamic_cast<const ServiceCenter *>(&inspection), nullptr);
}

TEST(DiContainer, DI03_IntakeIsSingleton) {
    std::istringstream in;
    std::ostringstream out;
    auto injector = makeInjector(in, out);
    EXPECT_EQ(&injector.create<IntakeService &>(), &injector.create<IntakeService &>());
}

TEST(DiContainer, DI04_StreamsAreBoundByReference) {
    std::istringstream in;
    std::ostringstream out;
    auto injector = makeInjector(in, out);
    EXPECT_EQ(&injector.create<std::istream &>(), &in);
    EXPECT_EQ(&injector.create<std::ostream &>(), &out);
}

TEST(DiContainer, DI05_AppSharesFleetWithReports) {
    std::istringstream in("2\nscooter\nМолния\n7\n10\n4\n0\n");
    std::ostringstream out;
    auto injector = makeInjector(in, out);
    injector.create<ConsoleApp>().run();

    const std::string text = out.str();
    EXPECT_NE(text.find("Принят: ВК-000001 Самокат «Молния»"), std::string::npos) << text;
    EXPECT_NE(text.find("Транспорт: 1, вещей: 0"), std::string::npos) << text;
    ASSERT_EQ(injector.create<Fleet &>().vehicles().size(), 1u);
    EXPECT_EQ(injector.create<const FleetReports &>().summary().vehicleCount, 1u);
}

TEST(DiContainer, DI06_FakeInspectionRejectsThroughContainer) {
    namespace di = boost::di;
    std::istringstream in;
    std::ostringstream out;
    auto injector = di::make_injector(makeInjector(in, out),
                                      di::bind<IInspectionService>().to<RejectingInspection>()[di::override]);

    const IntakeResult result = injector.create<IntakeService &>().accept(std::make_unique<Scooter>("Молния", 7, 10));
    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.reason, "тестовый отказ");
    EXPECT_TRUE(injector.create<Fleet &>().vehicles().empty());
}

TEST(DiContainer, DI07_FakeInspectionAcceptsThroughApp) {
    namespace di = boost::di;
    std::istringstream in("2\nscooter\nСтарик\n7\n90\n0\n");
    std::ostringstream out;
    auto injector = di::make_injector(makeInjector(in, out),
                                      di::bind<IInspectionService>().to<AcceptingInspection>()[di::override]);
    injector.create<ConsoleApp>().run();

    const std::string text = out.str();
    EXPECT_NE(text.find("Принят: ВК-"), std::string::npos) << text;
    EXPECT_EQ(text.find("Отклонён"), std::string::npos) << text;
    ASSERT_FALSE(injector.create<Fleet &>().vehicles().empty());
    EXPECT_EQ(injector.create<Fleet &>().vehicles().back()->nickname(), "Старик");
}
