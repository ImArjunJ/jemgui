#pragma once

#include <jemgui/types.hpp>

namespace jemgui {

constexpr u16 rgb565(u8 r, u8 g, u8 b) {
  return static_cast<u16>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

constexpr u8 rgb565_r(u16 c) {
  return static_cast<u8>(((c >> 11) & 0x1F) * 255 / 31);
}

constexpr u8 rgb565_g(u16 c) {
  return static_cast<u8>(((c >> 5) & 0x3F) * 255 / 63);
}

constexpr u8 rgb565_b(u16 c) { return static_cast<u8>((c & 0x1F) * 255 / 31); }

constexpr u16 blend_rgb565(u16 fg, u16 bg, u8 alpha) {
  u32 fr = (fg >> 11) & 0x1F;
  u32 fg_ = (fg >> 5) & 0x3F;
  u32 fb = fg & 0x1F;
  u32 br = (bg >> 11) & 0x1F;
  u32 bg_ = (bg >> 5) & 0x3F;
  u32 bb = bg & 0x1F;
  u32 inv = 255 - alpha;
  u32 r = (fr * alpha + br * inv) / 255;
  u32 g = (fg_ * alpha + bg_ * inv) / 255;
  u32 b = (fb * alpha + bb * inv) / 255;
  return static_cast<u16>((r << 11) | (g << 5) | b);
}

constexpr u16 darken(u16 c, u8 amount) {
  u16 r = ((c >> 11) & 0x1F);
  u16 g = ((c >> 5) & 0x3F);
  u16 b = (c & 0x1F);
  r = r > amount / 8 ? r - amount / 8 : 0;
  g = g > amount / 4 ? g - amount / 4 : 0;
  b = b > amount / 8 ? b - amount / 8 : 0;
  return static_cast<u16>((r << 11) | (g << 5) | b);
}

constexpr u16 lighten(u16 c, u8 amount) {
  u16 r = ((c >> 11) & 0x1F);
  u16 g = ((c >> 5) & 0x3F);
  u16 b = (c & 0x1F);
  r = r + amount / 8 < 0x1F ? r + amount / 8 : 0x1F;
  g = g + amount / 4 < 0x3F ? g + amount / 4 : 0x3F;
  b = b + amount / 8 < 0x1F ? b + amount / 8 : 0x1F;
  return static_cast<u16>((r << 11) | (g << 5) | b);
}

namespace colors {

inline constexpr u16 black = 0x0000;
inline constexpr u16 white = 0xFFFF;
inline constexpr u16 red = 0xF800;
inline constexpr u16 green = 0x07E0;
inline constexpr u16 blue = 0x001F;
inline constexpr u16 cyan = 0x07FF;
inline constexpr u16 magenta = 0xF81F;
inline constexpr u16 yellow = 0xFFE0;
inline constexpr u16 orange = 0xFD20;
inline constexpr u16 purple = 0x8010;
inline constexpr u16 gray = 0x8410;
inline constexpr u16 dark_gray = 0x4208;
inline constexpr u16 light_gray = 0xC618;

}  // namespace colors

}  // namespace jemgui
