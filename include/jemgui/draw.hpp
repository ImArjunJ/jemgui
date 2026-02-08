#pragma once

#include <jemgui/color.hpp>
#include <jemgui/painter.hpp>
#include <jemgui/types.hpp>
#include <jemgui/widgets.hpp>

namespace jemgui::draw {

template <painter P>
void gradient_v(P& p, rect r, u16 top_color, u16 bot_color) {
  if (r.h() == 0) return;
  i32 tr = (top_color >> 11) & 0x1F;
  i32 tg = (top_color >> 5) & 0x3F;
  i32 tb = top_color & 0x1F;
  i32 br = (bot_color >> 11) & 0x1F;
  i32 bg = (bot_color >> 5) & 0x3F;
  i32 bb = bot_color & 0x1F;
  i16 h = static_cast<i16>(r.h());
  i32 hm1 = h > 1 ? h - 1 : 1;
  for (i16 y = 0; y < h; ++y) {
    i32 cr = tr + (br - tr) * y / hm1;
    i32 cg = tg + (bg - tg) * y / hm1;
    i32 cb = tb + (bb - tb) * y / hm1;
    u16 c = static_cast<u16>(((cr & 0x1F) << 11) | ((cg & 0x3F) << 5) |
                             (cb & 0x1F));
    p.hline(r.x(), static_cast<i16>(r.y() + y), static_cast<i16>(r.w()), c);
  }
}

template <painter P>
void gradient_h(P& p, rect r, u16 left_color, u16 right_color) {
  if (r.w() == 0) return;
  i32 lr = (left_color >> 11) & 0x1F;
  i32 lg = (left_color >> 5) & 0x3F;
  i32 lb = left_color & 0x1F;
  i32 rr = (right_color >> 11) & 0x1F;
  i32 rg = (right_color >> 5) & 0x3F;
  i32 rb = right_color & 0x1F;
  i16 w = static_cast<i16>(r.w());
  i32 wm1 = w > 1 ? w - 1 : 1;
  for (i16 x = 0; x < w; ++x) {
    i32 cr = lr + (rr - lr) * x / wm1;
    i32 cg = lg + (rg - lg) * x / wm1;
    i32 cb = lb + (rb - lb) * x / wm1;
    u16 c = static_cast<u16>(((cr & 0x1F) << 11) | ((cg & 0x3F) << 5) |
                             (cb & 0x1F));
    p.vline(static_cast<i16>(r.x() + x), r.y(), static_cast<i16>(r.h()), c);
  }
}

template <painter P>
void shadow(P& p, rect r, i16 radius, u16 bg_color, u8 layers = 3,
            i16 offset_x = 2, i16 offset_y = 2) {
  for (i16 i = layers; i > 0; --i) {
    u8 alpha = static_cast<u8>(40 / i);
    u16 sc = blend_rgb565(0x0000, bg_color, alpha);
    rect sr = {
        {static_cast<i16>(r.x() + offset_x + i),
         static_cast<i16>(r.y() + offset_y + i)},
        {static_cast<u16>(r.w() + i * 2), static_cast<u16>(r.h() + i * 2)},
    };
    rounded_rect_fill(p, sr, static_cast<i16>(radius + i), sc);
  }
}

template <painter P>
void rounded_rect_gradient_v(P& p, rect r, i16 radius, u16 top_color,
                             u16 bot_color) {
  if (r.h() == 0 || r.w() == 0) return;

  i32 tr = (top_color >> 11) & 0x1F;
  i32 tg = (top_color >> 5) & 0x3F;
  i32 tb = top_color & 0x1F;
  i32 br = (bot_color >> 11) & 0x1F;
  i32 bg = (bot_color >> 5) & 0x3F;
  i32 bb = bot_color & 0x1F;
  i16 h = static_cast<i16>(r.h());
  i32 hm1 = h > 1 ? h - 1 : 1;

  if (radius <= 0 || r.w() < 2 * radius || r.h() < 2 * radius) {
    gradient_v(p, r, top_color, bot_color);
    return;
  }

  auto color_at = [&](i16 y) -> u16 {
    i32 cr = tr + (br - tr) * static_cast<i32>(y) / hm1;
    i32 cg = tg + (bg - tg) * static_cast<i32>(y) / hm1;
    i32 cb = tb + (bb - tb) * static_cast<i32>(y) / hm1;
    return static_cast<u16>(((cr & 0x1F) << 11) | ((cg & 0x3F) << 5) |
                            (cb & 0x1F));
  };

  for (i16 y = radius; y < h - radius; ++y) {
    u16 c = color_at(y);
    p.hline(r.x(), static_cast<i16>(r.y() + y), static_cast<i16>(r.w()), c);
  }

  i16 cx_l = static_cast<i16>(r.x() + radius);
  i16 cx_r = static_cast<i16>(r.right() - radius - 1);

  i16 f = 1 - radius;
  i16 dd_f_x = 1;
  i16 dd_f_y = -2 * radius;
  i16 px = 0;
  i16 py = radius;

  while (px <= py) {
    i16 y_top_py = static_cast<i16>(radius - py);
    i16 y_top_px = static_cast<i16>(radius - px);
    i16 y_bot_py = static_cast<i16>(h - 1 - radius + py);
    i16 y_bot_px = static_cast<i16>(h - 1 - radius + px);

    {
      u16 c = color_at(y_top_py);
      i16 x0 = static_cast<i16>(cx_l - px);
      i16 w2 = static_cast<i16>(cx_r - cx_l + 2 * px + 1);
      p.hline(x0, static_cast<i16>(r.y() + y_top_py), w2, c);
    }
    {
      u16 c = color_at(y_top_px);
      i16 x0 = static_cast<i16>(cx_l - py);
      i16 w2 = static_cast<i16>(cx_r - cx_l + 2 * py + 1);
      p.hline(x0, static_cast<i16>(r.y() + y_top_px), w2, c);
    }
    {
      u16 c = color_at(y_bot_py);
      i16 x0 = static_cast<i16>(cx_l - px);
      i16 w2 = static_cast<i16>(cx_r - cx_l + 2 * px + 1);
      p.hline(x0, static_cast<i16>(r.y() + y_bot_py), w2, c);
    }
    {
      u16 c = color_at(y_bot_px);
      i16 x0 = static_cast<i16>(cx_l - py);
      i16 w2 = static_cast<i16>(cx_r - cx_l + 2 * py + 1);
      p.hline(x0, static_cast<i16>(r.y() + y_bot_px), w2, c);
    }

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
void circle_outline(P& p, i16 x0, i16 y0, i16 r, u16 color) {
  i16 f = 1 - r;
  i16 ddx = 1;
  i16 ddy = -2 * r;
  i16 px = 0;
  i16 py = r;
  p.pixel(x0, static_cast<i16>(y0 + r), color);
  p.pixel(x0, static_cast<i16>(y0 - r), color);
  p.pixel(static_cast<i16>(x0 + r), y0, color);
  p.pixel(static_cast<i16>(x0 - r), y0, color);
  while (px < py) {
    if (f >= 0) {
      py--;
      ddy += 2;
      f += ddy;
    }
    px++;
    ddx += 2;
    f += ddx;
    p.pixel(static_cast<i16>(x0 + px), static_cast<i16>(y0 + py), color);
    p.pixel(static_cast<i16>(x0 - px), static_cast<i16>(y0 + py), color);
    p.pixel(static_cast<i16>(x0 + px), static_cast<i16>(y0 - py), color);
    p.pixel(static_cast<i16>(x0 - px), static_cast<i16>(y0 - py), color);
    p.pixel(static_cast<i16>(x0 + py), static_cast<i16>(y0 + px), color);
    p.pixel(static_cast<i16>(x0 - py), static_cast<i16>(y0 + px), color);
    p.pixel(static_cast<i16>(x0 + py), static_cast<i16>(y0 - px), color);
    p.pixel(static_cast<i16>(x0 - py), static_cast<i16>(y0 - px), color);
  }
}

template <painter P>
void arc_fill(P& p, i16 cx, i16 cy, i16 outer_r, i16 inner_r, i16 start_deg,
              i16 end_deg, u16 color) {
  auto sin_approx = [](i16 deg) -> i32 {
    deg = static_cast<i16>(((deg % 360) + 360) % 360);
    bool neg = deg >= 180;
    if (neg) deg = static_cast<i16>(deg - 180);
    if (deg > 90) deg = static_cast<i16>(180 - deg);
    i32 x = static_cast<i32>(deg) * 286 / 90;
    i32 s = x * (512 - ((x * x) >> 8)) >> 8;
    return neg ? -s : s;
  };

  auto cos_approx = [&](i16 deg) -> i32 {
    return sin_approx(static_cast<i16>(deg + 90));
  };

  for (i16 y = static_cast<i16>(-outer_r); y <= outer_r; ++y) {
    for (i16 x = static_cast<i16>(-outer_r); x <= outer_r; ++x) {
      i32 d2 = static_cast<i32>(x) * x + static_cast<i32>(y) * y;
      i32 or2 = static_cast<i32>(outer_r) * outer_r;
      i32 ir2 = static_cast<i32>(inner_r) * inner_r;
      if (d2 > or2 || d2 < ir2) continue;

      i16 deg = 0;
      if (x == 0 && y == 0)
        deg = 0;
      else {
        i32 ax = x < 0 ? -x : x;
        i32 ay = y < 0 ? -y : y;
        i32 angle = (ax > ay) ? (ay * 45 / ax) : (90 - ax * 45 / ay);
        if (x < 0 && y <= 0)
          deg = static_cast<i16>(180 - angle);
        else if (x < 0 && y > 0)
          deg = static_cast<i16>(180 + angle);
        else if (x >= 0 && y > 0)
          deg = static_cast<i16>(360 - angle);
        else
          deg = static_cast<i16>(angle);
      }

      bool in_range;
      if (start_deg <= end_deg)
        in_range = deg >= start_deg && deg <= end_deg;
      else
        in_range = deg >= start_deg || deg <= end_deg;

      if (in_range) {
        p.pixel(static_cast<i16>(cx + x), static_cast<i16>(cy + y), color);
      }
    }
  }
}

}  // namespace jemgui::draw
