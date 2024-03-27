#include <gtest/gtest.h>
#include <string>

extern "C" {
    #include "unix/http.h"
}

TEST(http, string_to_header) {
    const char str_1[] = "Connection: close";
    const char str_2[] = " Connection : close ";

    http_header expected;
    http_header_set(&expected, "Connection", "close");

    http_header actual;

    http_str_to_header(&actual, str_1, strlen(str_1));
    EXPECT_STREQ(expected.key, actual.key);
    EXPECT_STREQ(expected.value, actual.value);

    http_str_to_header(&actual, str_2, strlen(str_2));
    EXPECT_STREQ(expected.key, actual.key);
    EXPECT_STREQ(expected.value, actual.value);
}

TEST(http, header_to_string) {
    http_header header;
    http_header_set(&header, "Connection", "close");

    const char expected[] = "Connection: close";

    static const int BUF_SIZE = 64;
    char actual[BUF_SIZE];

    http_header_to_str(actual, header, BUF_SIZE);
    EXPECT_STREQ(expected, actual);
}

TEST(http, parse_headers) {
    static const int HEADER_COUNT = 3;
    const char header_data[] = "Connection: close\r\nLast-modified: 20240902\r\nHost: www.internet.com";

    http_header headers[HEADER_COUNT];
    http_str_to_headers(headers, HEADER_COUNT, header_data, strlen(header_data));

    EXPECT_STREQ(headers[0].key,   "Connection");
    EXPECT_STREQ(headers[0].value, "close");
    EXPECT_STREQ(headers[1].key,   "Last-modified");
    EXPECT_STREQ(headers[1].value, "20240902");
    EXPECT_STREQ(headers[2].key,   "Host");
    EXPECT_STREQ(headers[2].value, "www.internet.com");
}