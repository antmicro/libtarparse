//
// Copyright (c) 2022 Antmicro
// SPDX-License-Identifier: Apache-2.0
//
#include "catch.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <tarparse/internal/numparse.hpp>

TEST_CASE("Numeric parser")
{
    SECTION("parse_unsigned")
    {
        SECTION("simple parse")
        {
            size_t val = 0;
            REQUIRE(tarparse::internal::parse_unsigned("12345", val));
            REQUIRE(val == 12345);
        }

        SECTION("parse with leading zeroes")
        {
            size_t val = 0;
            REQUIRE(tarparse::internal::parse_unsigned("0000012345", val));
            REQUIRE(val == 12345);
        }

        SECTION("reject invalid chars")
        {
            size_t val = 0;
            REQUIRE(!tarparse::internal::parse_unsigned("-12345", val));
        }

        SECTION("reject empty string")
        {
            size_t val = 0;
            REQUIRE(!tarparse::internal::parse_unsigned("", val));
        }

        SECTION("limits")
        {
            SECTION("handles size_t::max")
            {
                const size_t sizet_max = std::numeric_limits<size_t>::max();
                const auto str = std::to_string(sizet_max);

                size_t val = 0;
                REQUIRE(tarparse::internal::parse_unsigned(str.c_str(), val));
                REQUIRE(val == sizet_max);
            }

            SECTION("rejects overflow")
            {
                const size_t sizet_max = std::numeric_limits<size_t>::max();
                //  Probably guarantees overflow?
                const std::string str = std::string { "9" } + std::to_string(sizet_max);

                size_t val = 0;
                REQUIRE(!tarparse::internal::parse_unsigned(str.c_str(), val));
            }
        }

        SECTION("octal")
        {
            SECTION("simple octal parse")
            {
                size_t val = 0;
                REQUIRE(tarparse::internal::parse_unsigned("37", val, tarparse::internal::ParseMode::Octal));
                REQUIRE(val == 31);
            }
        }
    }
}
