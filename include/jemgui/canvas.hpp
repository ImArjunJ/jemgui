#pragma once

#include <cstdint>
#include <jemgui/color.hpp>
#include <jemgui/types.hpp>

namespace jemgui {

#if defined(__arm__) || defined(__thumb__)
#define JEMGUI_LARGE_BSS __attribute__((section(".sram1_bss")))
#else
#define JEMGUI_LARGE_BSS
#endif

#define JEMGUI_FRAMEBUF(name, max_w, max_h) \
  JEMGUI_LARGE_BSS static u16 name[(max_w) * (max_h)]

template <typename D>
class canvas {
 public:
  canvas(D& display, u16* buf)
      : display_{display},
        buf_{buf},
        w_{display.width()},
        h_{display.height()} {
    fill_screen(0x0000);
  }

  u16 width() const { return w_; }
  u16 height() const { return h_; }

  void reinit() {
    w_ = display_.width();
    h_ = display_.height();
    has_clip_ = false;
    dirty_y0_ = 0;
    dirty_y1_ = -1;
    fill_screen(0x0000);
  }

  void set_clip(rect r) {
    clip_ = r;
    has_clip_ = true;
  }

  void clear_clip() { has_clip_ = false; }

  void fill_screen(u16 color) {
    u32 word = (static_cast<u32>(color) << 16) | color;
    u32* dst = reinterpret_cast<u32*>(buf_);
    u32 count = (static_cast<u32>(w_) * h_) >> 1;
    while (count >= 8) {
      dst[0] = word;
      dst[1] = word;
      dst[2] = word;
      dst[3] = word;
      dst[4] = word;
      dst[5] = word;
      dst[6] = word;
      dst[7] = word;
      dst += 8;
      count -= 8;
    }
    while (count--) *dst++ = word;
    dirty_y0_ = 0;
    dirty_y1_ = static_cast<i16>(h_ - 1);
  }

  void pixel(i16 x, i16 y, u16 color) {
    if (static_cast<u16>(x) < w_ && static_cast<u16>(y) < h_) {
      if (has_clip_ && !clip_.contains({x, y})) return;
      buf_[y * w_ + x] = color;
      mark_dirty(y, y);
    }
  }

  void hline(i16 x, i16 y, i16 length, u16 color) {
    if (static_cast<u16>(y) >= h_ || length <= 0) return;
    i16 x0 = x < 0 ? static_cast<i16>(0) : x;
    i16 x1 = static_cast<i16>(x + length - 1);
    if (x1 >= w_) x1 = static_cast<i16>(w_ - 1);
    if (has_clip_) {
      if (y < clip_.y() || y >= clip_.bottom()) return;
      if (x0 < clip_.x()) x0 = clip_.x();
      if (x1 >= clip_.right()) x1 = static_cast<i16>(clip_.right() - 1);
    }
    if (x0 > x1) return;
    mark_dirty(y, y);
    u16* p = buf_ + y * w_ + x0;
    i16 n = static_cast<i16>(x1 - x0 + 1);
    if (n >= 4) {
      if (reinterpret_cast<uintptr_t>(p) & 2) {
        *p++ = color;
        n--;
      }
      u32 word = (static_cast<u32>(color) << 16) | color;
      u32* wp = reinterpret_cast<u32*>(p);
      i16 pairs = static_cast<i16>(n >> 1);
      while (pairs >= 4) {
        wp[0] = word;
        wp[1] = word;
        wp[2] = word;
        wp[3] = word;
        wp += 4;
        pairs -= 4;
      }
      while (pairs--) *wp++ = word;
      p = reinterpret_cast<u16*>(wp);
      if (n & 1) *p = color;
    } else {
      while (n--) *p++ = color;
    }
  }

  void vline(i16 x, i16 y, i16 vh, u16 color) {
    if (static_cast<u16>(x) >= w_ || vh <= 0) return;
    i16 y0 = y < 0 ? static_cast<i16>(0) : y;
    i16 y1 = static_cast<i16>(y + vh - 1);
    if (y1 >= h_) y1 = static_cast<i16>(h_ - 1);
    if (has_clip_) {
      if (x < clip_.x() || x >= clip_.right()) return;
      if (y0 < clip_.y()) y0 = clip_.y();
      if (y1 >= clip_.bottom()) y1 = static_cast<i16>(clip_.bottom() - 1);
    }
    if (y0 > y1) return;
    mark_dirty(y0, y1);
    u16* p = buf_ + y0 * w_ + x;
    i16 n = static_cast<i16>(y1 - y0 + 1);
    while (n--) {
      *p = color;
      p += w_;
    }
  }

  void fill_rect(i16 x, i16 y, i16 w, i16 h, u16 color) {
    i16 y0 = y < 0 ? static_cast<i16>(0) : y;
    i16 y1 = static_cast<i16>(y + h - 1);
    if (y1 >= h_) y1 = static_cast<i16>(h_ - 1);
    if (y0 > y1) return;
    for (i16 j = y0; j <= y1; ++j) hline(x, j, w, color);
  }

  void fill_circle(i16 x0, i16 y0, i16 r, u16 color) {
    hline(static_cast<i16>(x0 - r), y0, static_cast<i16>(2 * r + 1), color);
    i16 f = static_cast<i16>(1 - r);
    i16 ddx = 1;
    i16 ddy = static_cast<i16>(-2 * r);
    i16 px = 0;
    i16 py = r;
    while (px < py) {
      if (f >= 0) {
        py--;
        ddy = static_cast<i16>(ddy + 2);
        f = static_cast<i16>(f + ddy);
      }
      px++;
      ddx = static_cast<i16>(ddx + 2);
      f = static_cast<i16>(f + ddx);
      hline(static_cast<i16>(x0 - px), static_cast<i16>(y0 + py),
            static_cast<i16>(2 * px + 1), color);
      hline(static_cast<i16>(x0 - px), static_cast<i16>(y0 - py),
            static_cast<i16>(2 * px + 1), color);
      hline(static_cast<i16>(x0 - py), static_cast<i16>(y0 + px),
            static_cast<i16>(2 * py + 1), color);
      hline(static_cast<i16>(x0 - py), static_cast<i16>(y0 - px),
            static_cast<i16>(2 * py + 1), color);
    }
  }

  void set_cursor(i16 x, i16 y) {
    cx_ = x;
    cy_ = y;
  }

  void set_text_color(u16 c) { tc_ = c; }

  void set_text_size(u8 s) { ts_ = s; }

  void print(const char* str) {
    while (*str) {
      if (*str == '\n') {
        cx_ = 0;
        cy_ = static_cast<i16>(cy_ + 8 * ts_);
      } else {
        draw_glyph(cx_, cy_, *str, tc_, ts_);
        cx_ = static_cast<i16>(cx_ + 6 * ts_);
      }
      str++;
    }
  }

  void flush() {
    if (dirty_y0_ <= dirty_y1_) {
      u16 band_h = static_cast<u16>(dirty_y1_ - dirty_y0_ + 1);
      display_.blit(0, static_cast<u16>(dirty_y0_), w_, band_h,
                    buf_ + dirty_y0_ * w_);
      dirty_y0_ = static_cast<i16>(h_);
      dirty_y1_ = -1;
    }
  }

 private:
  bool font_bit(char c, u8 col, u8 row) const {
    if (c < 32 || c > 126 || col >= 5 || row >= 8) return false;
    return (font_[(c - 32) * 5 + col] >> row) & 1;
  }

  void draw_glyph(i16 x, i16 y, char c, u16 fg, u8 sz) {
    if (c < 32 || c > 126) return;
    if (sz == 1) {
      for (u8 i = 0; i < 5; i++) {
        u8 line = font_[(c - 32) * 5 + i];
        for (u8 j = 0; j < 8; j++) {
          if (line & 0x01)
            pixel(static_cast<i16>(x + i), static_cast<i16>(y + j), fg);
          line >>= 1;
        }
      }
      return;
    }

    for (u8 i = 0; i < 5; i++) {
      u8 line = font_[(c - 32) * 5 + i];
      for (u8 j = 0; j < 8; j++) {
        if (line & 0x01) {
          fill_rect(static_cast<i16>(x + i * sz), static_cast<i16>(y + j * sz),
                    sz, sz, fg);
        }
        line >>= 1;
      }
    }

    if (sz < 2) return;
    for (u8 i = 0; i < 5; i++) {
      for (u8 j = 0; j < 8; j++) {
        bool me = font_bit(c, i, j);
        if (me) continue;

        bool left = font_bit(c, static_cast<u8>(i - 1), j);
        bool right = font_bit(c, static_cast<u8>(i + 1), j);
        bool up = font_bit(c, i, static_cast<u8>(j - 1));
        bool down = font_bit(c, i, static_cast<u8>(j + 1));

        if (!left && !right && !up && !down) continue;

        i16 bx = static_cast<i16>(x + i * sz);
        i16 by = static_cast<i16>(y + j * sz);

        auto read_bg = [&](i16 px, i16 py) -> u16 {
          if (static_cast<u16>(px) < w_ && static_cast<u16>(py) < h_)
            return buf_[py * w_ + px];
          return 0;
        };

        if (left && !right) {
          u16 bg = read_bg(bx, by);
          u16 blend = blend_rgb565(fg, bg, 80);
          pixel(bx, by, blend);
        }
        if (right && !left) {
          i16 ex = static_cast<i16>(bx + sz - 1);
          u16 bg = read_bg(ex, by);
          u16 blend = blend_rgb565(fg, bg, 80);
          pixel(ex, by, blend);
        }
        if (up && !down) {
          u16 bg = read_bg(bx, by);
          u16 blend = blend_rgb565(fg, bg, 80);
          pixel(bx, by, blend);
        }
        if (down && !up) {
          i16 ey = static_cast<i16>(by + sz - 1);
          u16 bg = read_bg(bx, ey);
          u16 blend = blend_rgb565(fg, bg, 80);
          pixel(bx, ey, blend);
        }
      }
    }
  }

  // clang-format off
  static constexpr u8 font_[] = {
    0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x5F,0x00,0x00,
    0x00,0x07,0x00,0x07,0x00, 0x14,0x7F,0x14,0x7F,0x14,
    0x24,0x2A,0x7F,0x2A,0x12, 0x23,0x13,0x08,0x64,0x62,
    0x36,0x49,0x55,0x22,0x50, 0x00,0x05,0x03,0x00,0x00,
    0x00,0x1C,0x22,0x41,0x00, 0x00,0x41,0x22,0x1C,0x00,
    0x08,0x2A,0x1C,0x2A,0x08, 0x08,0x08,0x3E,0x08,0x08,
    0x00,0x50,0x30,0x00,0x00, 0x08,0x08,0x08,0x08,0x08,
    0x00,0x60,0x60,0x00,0x00, 0x20,0x10,0x08,0x04,0x02,
    0x3E,0x51,0x49,0x45,0x3E, 0x00,0x42,0x7F,0x40,0x00,
    0x42,0x61,0x51,0x49,0x46, 0x21,0x41,0x45,0x4B,0x31,
    0x18,0x14,0x12,0x7F,0x10, 0x27,0x45,0x45,0x45,0x39,
    0x3C,0x4A,0x49,0x49,0x30, 0x01,0x71,0x09,0x05,0x03,
    0x36,0x49,0x49,0x49,0x36, 0x06,0x49,0x49,0x29,0x1E,
    0x00,0x36,0x36,0x00,0x00, 0x00,0x56,0x36,0x00,0x00,
    0x00,0x08,0x14,0x22,0x41, 0x14,0x14,0x14,0x14,0x14,
    0x41,0x22,0x14,0x08,0x00, 0x02,0x01,0x51,0x09,0x06,
    0x32,0x49,0x79,0x41,0x3E, 0x7E,0x11,0x11,0x11,0x7E,
    0x7F,0x49,0x49,0x49,0x36, 0x3E,0x41,0x41,0x41,0x22,
    0x7F,0x41,0x41,0x22,0x1C, 0x7F,0x49,0x49,0x49,0x41,
    0x7F,0x09,0x09,0x01,0x01, 0x3E,0x41,0x41,0x51,0x32,
    0x7F,0x08,0x08,0x08,0x7F, 0x00,0x41,0x7F,0x41,0x00,
    0x20,0x40,0x41,0x3F,0x01, 0x7F,0x08,0x14,0x22,0x41,
    0x7F,0x40,0x40,0x40,0x40, 0x7F,0x02,0x04,0x02,0x7F,
    0x7F,0x04,0x08,0x10,0x7F, 0x3E,0x41,0x41,0x41,0x3E,
    0x7F,0x09,0x09,0x09,0x06, 0x3E,0x41,0x51,0x21,0x5E,
    0x7F,0x09,0x19,0x29,0x46, 0x46,0x49,0x49,0x49,0x31,
    0x01,0x01,0x7F,0x01,0x01, 0x3F,0x40,0x40,0x40,0x3F,
    0x1F,0x20,0x40,0x20,0x1F, 0x7F,0x20,0x18,0x20,0x7F,
    0x63,0x14,0x08,0x14,0x63, 0x03,0x04,0x78,0x04,0x03,
    0x61,0x51,0x49,0x45,0x43, 0x00,0x00,0x7F,0x41,0x41,
    0x02,0x04,0x08,0x10,0x20, 0x41,0x41,0x7F,0x00,0x00,
    0x04,0x02,0x01,0x02,0x04, 0x40,0x40,0x40,0x40,0x40,
    0x00,0x01,0x02,0x04,0x00, 0x20,0x54,0x54,0x54,0x78,
    0x7F,0x48,0x44,0x44,0x38, 0x38,0x44,0x44,0x44,0x20,
    0x38,0x44,0x44,0x48,0x7F, 0x38,0x54,0x54,0x54,0x18,
    0x08,0x7E,0x09,0x01,0x02, 0x08,0x14,0x54,0x54,0x3C,
    0x7F,0x08,0x04,0x04,0x78, 0x00,0x44,0x7D,0x40,0x00,
    0x20,0x40,0x44,0x3D,0x00, 0x00,0x7F,0x10,0x28,0x44,
    0x00,0x41,0x7F,0x40,0x00, 0x7C,0x04,0x18,0x04,0x78,
    0x7C,0x08,0x04,0x04,0x78, 0x38,0x44,0x44,0x44,0x38,
    0x7C,0x14,0x14,0x14,0x08, 0x08,0x14,0x14,0x18,0x7C,
    0x7C,0x08,0x04,0x04,0x08, 0x48,0x54,0x54,0x54,0x20,
    0x04,0x3F,0x44,0x40,0x20, 0x3C,0x40,0x40,0x20,0x7C,
    0x1C,0x20,0x40,0x20,0x1C, 0x3C,0x40,0x30,0x40,0x3C,
    0x44,0x28,0x10,0x28,0x44, 0x0C,0x50,0x50,0x50,0x3C,
    0x44,0x64,0x54,0x4C,0x44, 0x00,0x08,0x36,0x41,0x00,
    0x00,0x00,0x7F,0x00,0x00, 0x00,0x41,0x36,0x08,0x00,
    0x08,0x08,0x2A,0x1C,0x08,
  };
  // clang-format on

  rect clip_ = {};
  bool has_clip_ = false;

  void mark_dirty(i16 y0, i16 y1) {
    if (y0 < dirty_y0_) dirty_y0_ = y0;
    if (y1 > dirty_y1_) dirty_y1_ = y1;
  }

  D& display_;
  u16* buf_;
  u16 w_;
  u16 h_;
  i16 dirty_y0_ = 0;
  i16 dirty_y1_ = -1;
  i16 cx_ = 0;
  i16 cy_ = 0;
  u16 tc_ = 0xFFFF;
  u8 ts_ = 1;
};

}  // namespace jemgui
