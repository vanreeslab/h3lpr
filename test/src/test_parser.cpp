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
    int  opt_param = parser.GetValue<int>("--optional-param", "an optional parameter", 3);
    EXPECT_EQ(opt_param, 3);
    double mandatory_param = parser.GetValue<double>("--mandatory-param", "a mandatory parameter");
    EXPECT_EQ(mandatory_param, 0.1);

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