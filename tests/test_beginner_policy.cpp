#include <gtest/gtest.h>

#include "Beginner_policy.h"
#include "equipment.h"

TEST(BeginnerPolicy, BP01_MinSimplicity) { EXPECT_EQ(BeginnerPolicy::kMinSimplicity, 6); }

TEST(BeginnerPolicy, BP02_BelowLimitIsNotSuitable) {
    const BeginnerPolicy policy;
    EXPECT_FALSE(policy.isSuitable(Scooter("A", 1, 0)));
    EXPECT_FALSE(policy.isSuitable(Scooter("A", 5, 0)));
}

TEST(BeginnerPolicy, BP03_LimitIsSuitable) { EXPECT_TRUE(BeginnerPolicy().isSuitable(Bicycle("A", 6, 0))); }

TEST(BeginnerPolicy, BP04_ElectricMaxSimplicityIsSuitable) {
    EXPECT_TRUE(BeginnerPolicy().isSuitable(ElectricBicycle("A", 10, 0, 1)));
}
