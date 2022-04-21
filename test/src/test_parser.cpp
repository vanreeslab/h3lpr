#include "gtest/gtest.h"
#include "macros.hpp"
#include "parser.hpp"

using namespace H3LPR;
using std::string;

class TestParser : public ::testing::Test {
    void SetUp() override {
        const testing::TestInfo* const test_info = testing::UnitTest::GetInstance()->current_test_info();
        m_log_noheader("::group:: Testing %s/%s", test_info->test_suite_name(), test_info->name());
    };
    void TearDown() override {
        m_log_noheader("::endgroup::");
    };
};

TEST_F(TestParser, help) {
    const int   argc   = 3;
    const char* msg[3] = {"./h3lpr", "--help", "--mandatory-param=0.1"};

    // create the parser
    Parser parser(argc, msg);

    // get a few option to populate the help
    bool is_flag   = parser.GetFlag("--flag", "the flag sets is_flag to true");
    EXPECT_EQ(is_flag, false);
    int  opt_param = parser.GetValue<int>("--optional-param", "an optional parameter", 3);
    EXPECT_EQ(opt_param, 3);
    double mandatory_param = parser.GetValue<double>("--mandatory-param", "a mandatory parameter");
    EXPECT_EQ(mandatory_param, 0.1);

    bool is_flag2 = parser.TestFlag("--flag2");

    parser.Finalize();
}

TEST_F(TestParser, flag) {
    const int   argc   = 2;
    const char* msg[2] = {"./h3lpr", "--flag"};

    // create the parser
    Parser parser(argc, msg);

    // get a few option to populate the help
    bool is_flag = parser.GetFlag("--flag", "the flag sets is_flag to true");
    EXPECT_EQ(is_flag, true);

    parser.Finalize();
}

TEST_F(TestParser, array) {
    const int   argc   = 4;
    const char* msg[4] = {"./h3lpr", "--array_1=2,1,0,3,4", "--array_2=3.1,4.1,5.9", "--help"};

    // create the parser
    Parser parser(argc, msg);

    // get a few option to populate the help
    auto array_1 = parser.GetValues<int, 5>("--array_1", "array of size 5");
    EXPECT_EQ(array_1[0], 2);
    EXPECT_EQ(array_1[1], 1);
    EXPECT_EQ(array_1[2], 0);
    EXPECT_EQ(array_1[3], 3);
    EXPECT_EQ(array_1[4], 4);

    auto array_2 = parser.GetValues<double, 3>("--array_2", "array of size 3");
    EXPECT_EQ(array_2[0], 3.1);
    EXPECT_EQ(array_2[1], 4.1);
    EXPECT_EQ(array_2[2], 5.9);

    auto array_3 = parser.GetValues<double, 2>("--array_3", "array of size 2", {1.7, 2.9});
    EXPECT_EQ(array_3[0], 1.7);
    EXPECT_EQ(array_3[1], 2.9);

    parser.Finalize();
}

TEST_F(TestParser, file) {
    const int   argc   = 3;
    const char* msg[3] = {"./h3lpr", "--config=config", "--help"};

    // create the parser
    Parser parser(argc,msg);

    // get a few option to populate the help
    bool is_flag = parser.GetFlag("--flag1", "the flag sets is_flag to true");
    EXPECT_EQ(is_flag, true);

    parser.Finalize();
}