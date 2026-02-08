#pragma once

#include <jemgui/color.hpp>
#include <jemgui/painter.hpp>
#include <jemgui/theme.hpp>
#include <jemgui/types.hpp>

namespace jemgui::draw {

template <painter P>
void rounded_rect_fill(P& p, rect r, i16 radius, u16 color) {
  if (radius <= 0 || r.w() < 2 * radius || r.h() < 2 * radius) {
    p.fill_rect(r.x(), r.y(), static_cast<i16>(r.w()), static_cast<i16>(r.h()),
                color);
    return;
  }

  p.fill_rect(static_cast<i16>(r.x() + radius), r.y(),
              static_cast<i16>(r.w() - 2 * radius), static_cast<i16>(r.h()),
              color);

  p.fill_rect(r.x(), static_cast<i16>(r.y() + radius), static_cast<i16>(radius),
              static_cast<i16>(r.h() - 2 * radius), color);

  p.fill_rect(static_cast<i16>(r.right() - radius),
              static_cast<i16>(r.y() + radius), static_cast<i16>(radius),
              static_cast<i16>(r.h() - 2 * radius), color);

  i16 cx_tl = static_cast<i16>(r.x() + radius);
  i16 cy_tl = static_cast<i16>(r.y() + radius);
  i16 cx_tr = static_cast<i16>(r.right() - radius - 1);
  i16 cx_bl = static_cast<i16>(r.x() + radius);
  i16 cy_bl = static_cast<i16>(r.bottom() - radius - 1);
  i16 cx_br = static_cast<i16>(r.right() - radius - 1);

  i16 f = 1 - radius;
  i16 dd_f_x = 1;
  i16 dd_f_y = -2 * radius;
  i16 px = 0;
  i16 py = radius;

  while (px <= py) {
    p.hline(static_cast<i16>(cx_tl - px), static_cast<i16>(cy_tl - py),
            static_cast<i16>(cx_tr - cx_tl + 2 * px + 1), color);
    p.hline(static_cast<i16>(cx_tl - py), static_cast<i16>(cy_tl - px),
            static_cast<i16>(cx_tr - cx_tl + 2 * py + 1), color);
    p.hline(static_cast<i16>(cx_bl - px), static_cast<i16>(cy_bl + py),
            static_cast<i16>(cx_br - cx_bl + 2 * px + 1), color);
    p.hline(static_cast<i16>(cx_bl - py), static_cast<i16>(cy_bl + px),
            static_cast<i16>(cx_br - cx_bl + 2 * py + 1), color);

    if (f >= 0) {
      py--;
      dd_f_y += 2;
      f += dd_f_y;
    }
    px++;
    dd_f_x += 2;
    f += dd_f_x;
  }
}

template <painter P>
void rounded_rect_outline(P& p, rect r, i16 radius, u16 color) {
  if (radius <= 0 || r.w() < 2 * radius || r.h() < 2 * radius) {
    p.hline(r.x(), r.y(), static_cast<i16>(r.w()), color);
    p.hline(r.x(), static_cast<i16>(r.bottom() - 1), static_cast<i16>(r.w()),
            color);
    p.vline(r.x(), r.y(), static_cast<i16>(r.h()), color);
    p.vline(static_cast<i16>(r.right() - 1), r.y(), static_cast<i16>(r.h()),
            color);
    return;
  }

  p.hline(static_cast<i16>(r.x() + radius), r.y(),
          static_cast<i16>(r.w() - 2 * radius), color);
  p.hline(static_cast<i16>(r.x() + radius), static_cast<i16>(r.bottom() - 1),
          static_cast<i16>(r.w() - 2 * radius), color);
  p.vline(r.x(), static_cast<i16>(r.y() + radius),
          static_cast<i16>(r.h() - 2 * radius), color);
  p.vline(static_cast<i16>(r.right() - 1), static_cast<i16>(r.y() + radius),
          static_cast<i16>(r.h() - 2 * radius), color);

  i16 cx_tl = static_cast<i16>(r.x() + radius);
  i16 cy_tl = static_cast<i16>(r.y() + radius);
  i16 cx_tr = static_cast<i16>(r.right() - radius - 1);
  i16 cy_tr = static_cast<i16>(r.y() + radius);
  i16 cx_bl = static_cast<i16>(r.x() + radius);
  i16 cy_bl = static_cast<i16>(r.bottom() - radius - 1);
  i16 cx_br = static_cast<i16>(r.right() - radius - 1);
  i16 cy_br = static_cast<i16>(r.bottom() - radius - 1);

  i16 f = 1 - radius;
  i16 dd_f_x = 1;
  i16 dd_f_y = -2 * radius;
  i16 px = 0;
  i16 py = radius;

  while (px <= py) {
    p.pixel(static_cast<i16>(cx_tl - px), static_cast<i16>(cy_tl - py), color);
    p.pixel(static_cast<i16>(cx_tl - py), static_cast<i16>(cy_tl - px), color);
    p.pixel(static_cast<i16>(cx_tr + px), static_cast<i16>(cy_tr - py), color);
    p.pixel(static_cast<i16>(cx_tr + py), static_cast<i16>(cy_tr - px), color);
    p.pixel(static_cast<i16>(cx_bl - px), static_cast<i16>(cy_bl + py), color);
    p.pixel(static_cast<i16>(cx_bl - py), static_cast<i16>(cy_bl + px), color);
    p.pixel(static_cast<i16>(cx_br + px), static_cast<i16>(cy_br + py), color);
    p.pixel(static_cast<i16>(cx_br + py), static_cast<i16>(cy_br + px), color);

    if (f >= 0) {
      py--;
      dd_f_y += 2;
      f += dd_f_y;
    }
    px++;
    dd_f_x += 2;
    f += dd_f_x;
  }
}

inline i16 text_width(const char* text, u8 font_size) {
  i16 w = 0;
  while (*text) {
    if (*text == '\n') break;
    w = static_cast<i16>(w + 6 * font_size);
    text++;
  }
  return w;
}

inline i16 text_height(u8 font_size) { return static_cast<i16>(8 * font_size); }

template <painter P>
void text_centered(P& p, rect r, const char* text, u16 color, u8 font_size) {
  i16 tw = text_width(text, font_size);
  i16 th = text_height(font_size);
  i16 tx = static_cast<i16>(r.x() + (static_cast<i16>(r.w()) - tw) / 2);
  i16 ty = static_cast<i16>(r.y() + (static_cast<i16>(r.h()) - th) / 2);
  p.set_text_size(font_size);
  p.set_text_color(color);
  p.set_cursor(tx, ty);
  p.print(text);
}

template <painter P>
void text_left(P& p, rect r, const char* text, u16 color, u8 font_size,
               i16 pad_left = 0) {
  i16 th = text_height(font_size);
  i16 tx = static_cast<i16>(r.x() + pad_left);
  i16 ty = static_cast<i16>(r.y() + (static_cast<i16>(r.h()) - th) / 2);
  p.set_text_size(font_size);
  p.set_text_color(color);
  p.set_cursor(tx, ty);
  p.print(text);
}

}  // namespace jemgui::draw
