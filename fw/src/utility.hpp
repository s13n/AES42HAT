/** @file
 * various utilities.
 */
#pragma once

#include <array>
#include <utility>

// Helper to generate repeated values
template <typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> make_array_impl(T value, std::index_sequence<Is...>) {
    return {((void)Is, value)...}; // Repeat `value` using comma operator
}

// Main function: generates std::array with N copies of `value`
template <std::size_t N, typename T>
constexpr std::array<T, N> make_array(T value) {
    return make_array_impl<T>(value, std::make_index_sequence<N>{});
}
