#include <gtest/gtest.h>
#include "../TimeParser.h"

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {

    // Test with correct time strings
    char time_test[] = "141205";
    ASSERT_EQ(time_parse(time_test), 725);

    char time_test3[] = "235959";
    ASSERT_EQ(time_parse(time_test3), 3599);


    // tests with incorrect time strings
    char time_test2[] = "000000";
    ASSERT_EQ(time_parse(time_test2), -1);

    char time_test4[] = "120000";
    ASSERT_EQ(time_parse(time_test4), -1);

    char time_test5[] = "1234567";
    ASSERT_EQ(time_parse(time_test5), -1);

    char time_test6[] = "12a205";
    ASSERT_EQ(time_parse(time_test6), -1);

    char time_test7[] = "250000";
    ASSERT_EQ(time_parse(time_test7), -1);
   
    char time_test8[] = "126000";
    ASSERT_EQ(time_parse(time_test8), -1);
 
    char time_test9[] = "120060";
    ASSERT_EQ(time_parse(time_test9), -1);

    char time_test10[] = "1205";
    ASSERT_EQ(time_parse(time_test10), -1);
}

TEST(TimeParserTest, TestCaseCorrectUART) {

    // Test with correct uart strings
    char input1[] = "g2000";
    UartParser result1 = uart_parse(input1);

    ASSERT_EQ(result1.color, 'g');
    ASSERT_EQ(result1.time, 2000);

    char input2[] = "r1000";
    UartParser result2 = uart_parse(input2);

    ASSERT_EQ(result2.color, 'r');
    ASSERT_EQ(result2.time, 1000);


    // tests with incorrect uart strings
    char input3[] = "r100";
    UartParser result3 = uart_parse(input3);

    ASSERT_EQ(result3.color, -4);
    ASSERT_EQ(result3.time, -1);

    char input4[] = "r10000";
    UartParser result4 = uart_parse(input4);

    ASSERT_EQ(result4.color, -4);
    ASSERT_EQ(result4.time, -1);

    char input5[] = "r100a";
    UartParser result5 = uart_parse(input5);

    ASSERT_EQ(result5.color, -4);
    ASSERT_EQ(result5.time, -1);
}


// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
