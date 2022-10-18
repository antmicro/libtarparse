#pragma once

namespace tarparse::internal {
    template <typename T>
    constexpr T const& min(T const& v1, T const& v2)
    {
        return v1 <= v2 ? v1 : v2;
    }

    template <typename T>
    constexpr T const& max(T const& v1, T const& v2)
    {
        return v1 >= v2 ? v1 : v2;
    }
}
