#pragma once

#include <algorithm>
#include <cstdint>

namespace jemgui {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using usize = std::size_t;

struct vec2 {
  i16 x = 0;
  i16 y = 0;

  constexpr bool operator==(const vec2&) const = default;

  constexpr vec2 operator+(vec2 rhs) const {
    return {static_cast<i16>(x + rhs.x), static_cast<i16>(y + rhs.y)};
  }

  constexpr vec2 operator-(vec2 rhs) const {
    return {static_cast<i16>(x - rhs.x), static_cast<i16>(y - rhs.y)};
  }
};

struct extent {
  u16 w = 0;
  u16 h = 0;

  constexpr bool operator==(const extent&) const = default;

  constexpr u32 area() const { return static_cast<u32>(w) * h; }
};

struct rect {
  vec2 pos;
  extent size;

  constexpr bool operator==(const rect&) const = default;

  constexpr i16 x() const { return pos.x; }
  constexpr i16 y() const { return pos.y; }
  constexpr u16 w() const { return size.w; }
  constexpr u16 h() const { return size.h; }

  constexpr i16 right() const { return static_cast<i16>(pos.x + size.w); }
  constexpr i16 bottom() const { return static_cast<i16>(pos.y + size.h); }

  constexpr i16 cx() const { return static_cast<i16>(pos.x + size.w / 2); }
  constexpr i16 cy() const { return static_cast<i16>(pos.y + size.h / 2); }

  constexpr bool contains(vec2 p) const {
    return p.x >= pos.x && p.x < right() && p.y >= pos.y && p.y < bottom();
  }

  constexpr rect shrink(i16 amount) const {
    return {
        {static_cast<i16>(pos.x + amount), static_cast<i16>(pos.y + amount)},
        {static_cast<u16>(size.w > 2 * amount ? size.w - 2 * amount : 0),
         static_cast<u16>(size.h > 2 * amount ? size.h - 2 * amount : 0)},
    };
  }
};

using id = u32;

inline constexpr i16 scale(i16 value, i16 scale_256) {
  return static_cast<i16>((static_cast<i32>(value) * scale_256) >> 8);
}

}  // namespace jemgui
