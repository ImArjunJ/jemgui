#pragma once

#include <jemgui/color.hpp>
#include <jemgui/types.hpp>

namespace jemgui {

struct theme {
  u16 bg;
  u16 fg;
  u16 accent;
  u16 accent_hover;
  u16 accent_press;
  u16 surface;
  u16 surface_alt;
  u16 border;
  u16 text;
  u16 text_dim;
  u16 success;
  u16 warning;
  u16 danger;

  i16 padding;
  i16 spacing;
  i16 corner_radius;
  i16 widget_height;
  i16 border_width;
  u8 font_size;
};

namespace themes {

inline constexpr theme dark = {
    .bg = rgb565(18, 18, 24),
    .fg = rgb565(30, 30, 42),
    .accent = rgb565(90, 120, 255),
    .accent_hover = rgb565(110, 140, 255),
    .accent_press = rgb565(70, 100, 220),
    .surface = rgb565(28, 28, 38),
    .surface_alt = rgb565(38, 38, 50),
    .border = rgb565(55, 55, 70),
    .text = rgb565(230, 230, 240),
    .text_dim = rgb565(130, 130, 150),
    .success = rgb565(80, 200, 120),
    .warning = rgb565(255, 180, 50),
    .danger = rgb565(240, 70, 70),
    .padding = 6,
    .spacing = 4,
    .corner_radius = 4,
    .widget_height = 24,
    .border_width = 1,
    .font_size = 1,
};

inline constexpr theme light = {
    .bg = rgb565(240, 240, 245),
    .fg = rgb565(255, 255, 255),
    .accent = rgb565(60, 100, 240),
    .accent_hover = rgb565(80, 120, 255),
    .accent_press = rgb565(40, 80, 200),
    .surface = rgb565(250, 250, 252),
    .surface_alt = rgb565(235, 235, 240),
    .border = rgb565(200, 200, 210),
    .text = rgb565(30, 30, 40),
    .text_dim = rgb565(120, 120, 140),
    .success = rgb565(50, 180, 100),
    .warning = rgb565(230, 160, 30),
    .danger = rgb565(220, 50, 50),
    .padding = 6,
    .spacing = 4,
    .corner_radius = 4,
    .widget_height = 24,
    .border_width = 1,
    .font_size = 1,
};

inline constexpr theme mono = {
    .bg = 0x0000,
    .fg = 0xFFFF,
    .accent = 0xFFFF,
    .accent_hover = 0xFFFF,
    .accent_press = 0xFFFF,
    .surface = 0x0000,
    .surface_alt = 0x0000,
    .border = 0xFFFF,
    .text = 0xFFFF,
    .text_dim = 0xFFFF,
    .success = 0xFFFF,
    .warning = 0xFFFF,
    .danger = 0xFFFF,
    .padding = 2,
    .spacing = 2,
    .corner_radius = 0,
    .widget_height = 12,
    .border_width = 1,
    .font_size = 1,
};

}  // namespace themes

}  // namespace jemgui
