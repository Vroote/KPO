#include <cstddef>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "Beginner_policy.h"
#include "ServiceCenter_service.h"
#include "app.h"

namespace {

    const std::string kMenuExit = "0 — выход";

    bool contains(const std::string &text, const std::string &part) { return text.find(part) != std::string::npos; }

    std::size_t countOf(const std::string &text, const std::string &part) {
        std::size_t count = 0;
        for (auto position = text.find(part); position != std::string::npos; position = text.find(part, position + 1)) {
            ++count;
        }
        return count;
    }

    class ConsoleAppTest : public ::testing::Test {
    protected:
        std::string run(const std::string &input) {
            std::istringstream in(input);
            std::ostringstream out;
            ConsoleApp app(vehicleFactory_, itemFactory_, intake_, reports_, in, out);
            app.run();
            return out.str();
        }

        VehicleFactory vehicleFactory_;
        ItemFactory itemFactory_;
        ServiceCenter serviceCenter_;
        Fleet fleet_;
        BeginnerPolicy policy_;
        IntakeService intake_{serviceCenter_, fleet_};
        FleetReports reports_{fleet_, policy_};
    };

} // namespace

TEST(PadRight, PR01_PadsCyrillicBySymbols) { EXPECT_EQ(padRight("ВК", 5), "ВК   "); }

TEST(PadRight, PR02_PadsQuotesBySymbols) { EXPECT_EQ(padRight("«Ток»", 6), "«Ток» "); }

TEST(PadRight, PR03_LongTextIsNotTruncated) { EXPECT_EQ(padRight("abcdef", 3), "abcdef"); }

TEST(PadRight, PR04_ZeroWidth) {
    EXPECT_EQ(padRight("", 0), "");
    EXPECT_EQ(padRight("ab", 0), "ab");
}

TEST_F(ConsoleAppTest, AP01_ExitImmediately) { EXPECT_EQ(countOf(run("0\n"), kMenuExit), 1u); }

TEST_F(ConsoleAppTest, AP02_EmptyInputExitsQuietly) {
    const std::string out = run("");
    EXPECT_EQ(countOf(out, kMenuExit), 1u);
    EXPECT_FALSE(contains(out, "Ошибка"));
}

TEST_F(ConsoleAppTest, AP03_UnknownMenuItem) {
    const std::string out = run("7\n");
    EXPECT_TRUE(contains(out, "Неизвестный пункт меню: «7»")) << out;
    EXPECT_EQ(countOf(out, kMenuExit), 2u);
}

TEST_F(ConsoleAppTest, AP04_VehicleTypes) {
    const std::string out = run("1\n");
    EXPECT_TRUE(contains(out, "scooter     Самокат\n")) << out;
    EXPECT_TRUE(contains(out, "bicycle     Велосипед\n")) << out;
    EXPECT_TRUE(contains(out, "e-scooter   Электросамокат      нужен расход энергии\n")) << out;
    EXPECT_TRUE(contains(out, "e-bicycle   Электровелосипед    нужен расход энергии\n")) << out;
}

TEST_F(ConsoleAppTest, AP05_AcceptVehicle) {
    const std::string out = run("2\nscooter\nМолния\n7\n10\n");
    EXPECT_TRUE(contains(out, "Принят: ВК-000001 Самокат «Молния»")) << out;
    EXPECT_FALSE(contains(out, "Расход, кВт·ч/сут: ")) << out;
    EXPECT_EQ(fleet_.vehicles().size(), 1u);
}

TEST_F(ConsoleAppTest, AP06_ElectricVehicleAsksForEnergy) {
    const std::string out = run("2\ne-scooter\nИскра\n3\n10\n1.5\n");
    EXPECT_TRUE(contains(out, "Расход, кВт·ч/сут: ")) << out;
    EXPECT_TRUE(contains(out, "Принят: ВК-000001 Электросамокат «Искра»")) << out;
}

TEST_F(ConsoleAppTest, AP07_RejectedByWear) {
    const std::string out = run("2\nbicycle\nСтарик\n3\n55\n");
    EXPECT_TRUE(contains(out, "Отклонён: износ 55 % > 40 %")) << out;
    EXPECT_TRUE(fleet_.vehicles().empty());
}

TEST_F(ConsoleAppTest, AP08_RejectedByEnergy) {
    EXPECT_TRUE(contains(run("2\ne-bicycle\nТок\n6\n20\n2.5\n"), "Отклонён: расход 2.5 > 2 кВт·ч/сут"));
}

TEST_F(ConsoleAppTest, AP09_UnknownVehicleKindStopsQuestions) {
    const std::string out = run("2\ntank\n");
    EXPECT_TRUE(contains(out, "Ошибка: Неизвестный вид транспорта: tank")) << out;
    EXPECT_FALSE(contains(out, "Кличка: ")) << out;
}

TEST_F(ConsoleAppTest, AP10_SimplicityMustBeInteger) {
    const std::string out = run("2\nscooter\nX\nабв\n2\nscooter\nX\n5.5\n2\nscooter\nX\n7abc\n");
    EXPECT_TRUE(contains(out, "Ошибка: Ожидалось целое число: «абв»")) << out;
    EXPECT_TRUE(contains(out, "Ошибка: Ожидалось целое число: «5.5»")) << out;
    EXPECT_TRUE(contains(out, "Ошибка: Ожидалось целое число: «7abc»")) << out;
}

TEST_F(ConsoleAppTest, AP11_BlankNickname) {
    EXPECT_TRUE(contains(run("2\nscooter\n   \n5\n5\n"), "Ошибка: Кличка не может быть пустой"));
}

TEST_F(ConsoleAppTest, AP12_NaNWear) {
    EXPECT_TRUE(contains(run("2\nscooter\nX\n5\nnan\n"), "Ошибка: Износ не может быть NaN"));
}

TEST_F(ConsoleAppTest, AP13_InputIsTrimmed) {
    EXPECT_TRUE(contains(run(" 2 \n scooter \n Молния \n7\n10\n"), "Принят: ВК-000001 Самокат «Молния»"));
}

TEST_F(ConsoleAppTest, AP14_RegisterHelmetWithoutEnergy) {
    const std::string out = run("3\nhelmet\n");
    EXPECT_TRUE(contains(out, "Поставлен на баланс: ВК-000001 Шлем")) << out;
    EXPECT_FALSE(contains(out, "Расход, кВт·ч/сут: ")) << out;
}

TEST_F(ConsoleAppTest, AP15_RegisterDockStation) {
    EXPECT_TRUE(contains(run("3\ndock-station\n2\n"), "Поставлен на баланс: ВК-000001 Док-станция"));
}

TEST_F(ConsoleAppTest, AP16_UnknownItemKind) {
    EXPECT_TRUE(contains(run("3\nlock\n"), "Ошибка: Неизвестный вид вещи: lock"));
}

TEST_F(ConsoleAppTest, AP17_NegativeItemEnergy) {
    EXPECT_TRUE(
            contains(run("3\ndock-station\n-1\n"), "Ошибка: Суточное потребление энергии не может быть отрицательным"));
}

TEST_F(ConsoleAppTest, AP18_ItemTypeColumnsDoNotStick) {
    const std::string out = run("3\n");
    EXPECT_TRUE(contains(out, "helmet           Шлем\n")) << out;
    EXPECT_TRUE(contains(out, "dock-station     Док-станция         нужен расход энергии\n")) << out;
    EXPECT_TRUE(contains(out, "charging-cabinet Зарядный шкаф       нужен расход энергии\n")) << out;
}

TEST_F(ConsoleAppTest, AP19_SummaryEnergy) {
    const std::string out = run("2\ne-scooter\nИскра\n3\n10\n1.5\n"
                                "3\ndock-station\n2\n"
                                "3\ncharging-cabinet\n2.3\n"
                                "4\n");
    EXPECT_TRUE(contains(out, "Транспорт: 1, вещей: 2, суточный расход: 5.8 кВт·ч.")) << out;
}

TEST_F(ConsoleAppTest, AP20_EmptyReports) {
    const std::string out = run("4\n5\n6\n");
    EXPECT_TRUE(contains(out, "Транспорт: 0, вещей: 0, суточный расход: 0 кВт·ч.")) << out;
    EXPECT_TRUE(contains(out, "Подходящего транспорта нет.")) << out;
    EXPECT_TRUE(contains(out, "Инвентарь пуст.")) << out;
}

TEST_F(ConsoleAppTest, AP21_InventoryLines) {
    const std::string out = run("2\nscooter\nМолния\n7\n10\n"
                                "3\nhelmet\n"
                                "2\nbicycle\nВетер\n3\n10\n"
                                "6\n");
    const std::string first = padRight("ВК-000001", 12) + padRight("Самокат «Молния»", 32) + "транспорт\n";
    const std::string second = padRight("ВК-000002", 12) + padRight("Шлем", 32) + "вещь\n";
    const std::string third = padRight("ВК-000003", 12) + padRight("Велосипед «Ветер»", 32) + "транспорт\n";
    ASSERT_TRUE(contains(out, first)) << out;
    ASSERT_TRUE(contains(out, second)) << out;
    ASSERT_TRUE(contains(out, third)) << out;
    EXPECT_LT(out.find(first), out.find(second));
    EXPECT_LT(out.find(second), out.find(third));
}

TEST_F(ConsoleAppTest, AP22_EofInsideAcceptVehicle) {
    const std::string out = run("2\nscooter\nМолния\n");
    EXPECT_FALSE(contains(out, "Ошибка")) << out;
    EXPECT_FALSE(contains(out, "Принят")) << out;
    EXPECT_TRUE(fleet_.vehicles().empty());
}

TEST_F(ConsoleAppTest, AP23_EofInsideRegisterItem) {
    const std::string out = run("3\ndock-station\n");
    EXPECT_FALSE(contains(out, "Ошибка")) << out;
    EXPECT_FALSE(contains(out, "Поставлен")) << out;
    EXPECT_TRUE(fleet_.items().empty());
}

TEST_F(ConsoleAppTest, AP24_LastLineWithoutNewline) {
    EXPECT_TRUE(contains(run("4"), "Транспорт: 0, вещей: 0, суточный расход: 0 кВт·ч."));
}

TEST_F(ConsoleAppTest, AP25_LoopContinuesAfterError) {
    const std::string out = run("2\ntank\n4\n");
    const auto error = out.find("Ошибка: Неизвестный вид транспорта: tank");
    const auto summary = out.find("Транспорт: 0, вещей: 0");
    ASSERT_NE(error, std::string::npos) << out;
    ASSERT_NE(summary, std::string::npos) << out;
    EXPECT_LT(error, summary);
}

TEST_F(ConsoleAppTest, AP26_BeginnerFriendlyLines) {
    const std::string out = run("2\nscooter\nМолния\n7\n10\n"
                                "2\nbicycle\nВетер\n3\n10\n"
                                "5\n");
    EXPECT_TRUE(contains(out, padRight("ВК-000001", 12) + padRight("Самокат «Молния»", 32) + "простота 7\n")) << out;
    EXPECT_FALSE(contains(out, "Велосипед «Ветер»                простота")) << out;
}

TEST_F(ConsoleAppTest, AP27_AcceptVehicleShowsTypesFirst) {
    const std::string out = run("2\n");
    const auto table = out.find("e-bicycle   Электровелосипед    нужен расход энергии\n");
    const auto prompt = out.find("Вид (id): ");
    ASSERT_NE(table, std::string::npos) << out;
    ASSERT_NE(prompt, std::string::npos) << out;
    EXPECT_LT(table, prompt);
}
