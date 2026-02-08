#pragma once

#include <concepts>
#include <jemgui/types.hpp>

namespace jemgui {

template <typename P>
concept painter = requires(P p, i16 x, i16 y, i16 w, i16 h, u16 color,
                           const char* text, u8 sz, rect clip) {
  { p.width() } -> std::convertible_to<u16>;
  { p.height() } -> std::convertible_to<u16>;
  { p.fill_rect(x, y, w, h, color) } -> std::same_as<void>;
  { p.hline(x, y, w, color) } -> std::same_as<void>;
  { p.vline(x, y, h, color) } -> std::same_as<void>;
  { p.pixel(x, y, color) } -> std::same_as<void>;
  { p.fill_circle(x, y, w, color) } -> std::same_as<void>;
  { p.set_cursor(x, y) } -> std::same_as<void>;
  { p.set_text_color(color) } -> std::same_as<void>;
  { p.set_text_size(sz) } -> std::same_as<void>;
  { p.print(text) } -> std::same_as<void>;
  { p.set_clip(clip) } -> std::same_as<void>;
  { p.clear_clip() } -> std::same_as<void>;
};

}  // namespace jemgui
