#include <gtest/gtest.h>

#include <Gosu/Timing.hpp>

class TimingTests : public testing::Test
{
};

TEST_F(TimingTests, sleep_and_milliseconds)
{
    const auto before = Gosu::milliseconds();
    Gosu::sleep(1234);
    const auto after = Gosu::milliseconds();
    ASSERT_GT(after, before);
    ASSERT_GT(after - before, 1000);
    ASSERT_LT(after - before, 1500);
}
