//
// Copyright (c) 2022 Antmicro
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace tarparse::internal {
    enum class ParseMode {
        Decimal,
        Octal
    };

    /*
     *  Parses an UNSIGNED numeric string to a size_t value. Leading zeroes are allowed.
     *  The mode specfies the base of the numeric representation - decimal or octal.
     *
     *  Returns false if:
     *      - an empty or null string is passed, or
     *      - an invalid character is encounter, or
     *      - the parsed value is too big to fit inside a size_t
     *  Otherwise, returns true and sets the parsed value.
     */
    constexpr bool parse_unsigned(char const* string, size_t& value, ParseMode mode = ParseMode::Decimal)
    {
        if(!string || *string == '\0') {
            return false;
        }

        const size_t multiplier = (mode == ParseMode::Decimal) ? 10u : 8u;
        auto valid_chars = (mode == ParseMode::Decimal)
            ? [](char ch) { return ch >= '0' && ch <= '9'; }
            : [](char ch) { return ch >= '0' && ch <= '7'; };

        size_t current = 0;
        while(*string != '\0') {
            const char ch = *string;
            if(!valid_chars(ch)) {
                return false;
            }
            const size_t digit = (static_cast<size_t>(ch) - static_cast<size_t>('0'));

            const size_t result = current * multiplier;
            if(result < current) {
                //  Overflow
                return false;
            }

            const size_t addition_result = result + digit;
            if(addition_result < result) {
                //  Overflow
                return false;
            }

            current = addition_result;
            string++;
        }
        value = current;
        return true;
    }
}
