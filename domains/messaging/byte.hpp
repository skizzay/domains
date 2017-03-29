#pragma once

#include <domains/utils/type_traits.hpp>

namespace domains {
enum class byte : unsigned char {};

template <class T>
constexpr std::enable_if_t<is_integral_v<T>, T> to_integer(byte b) noexcept {
   return static_cast<T>(static_cast<unsigned char>(b));
}

template <class T>
constexpr std::enable_if_t<is_integral_v<T>, byte &> operator<<=(byte &b, T const shift) noexcept {
   return b = byte(static_cast<unsigned char>(b) << shift);
}

template <class T>
constexpr std::enable_if_t<is_integral_v<T>, byte &> operator>>=(byte &b, T const shift) noexcept {
   return b = byte(static_cast<unsigned char>(b) >> shift);
}

constexpr byte &operator|=(byte &l, byte const r) noexcept {
   return l = byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
}

constexpr byte &operator&=(byte &l, byte const r) noexcept {
   return l = byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
}

constexpr byte &operator^=(byte &l, byte const r) noexcept {
   return l = byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
}

template <class T>
constexpr std::enable_if_t<is_integral_v<T>, byte> operator<<(byte const b,
                                                              T const shift) noexcept {
   return byte(static_cast<unsigned char>(b) << shift);
}

template <class T>
constexpr std::enable_if_t<is_integral_v<T>, byte> operator>>(byte const b,
                                                              T const shift) noexcept {
   return byte(static_cast<unsigned char>(b) >> shift);
}

constexpr byte operator|=(byte const l, byte const r) noexcept {
   return byte(static_cast<unsigned char>(l) | static_cast<unsigned char>(r));
}

constexpr byte operator&=(byte const l, byte const r) noexcept {
   return byte(static_cast<unsigned char>(l) & static_cast<unsigned char>(r));
}

constexpr byte operator^=(byte const l, byte const r) noexcept {
   return byte(static_cast<unsigned char>(l) ^ static_cast<unsigned char>(r));
}
}
