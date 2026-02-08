#pragma once

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <jemgui/anim.hpp>
#include <jemgui/color.hpp>
#include <jemgui/draw.hpp>
#include <jemgui/hash.hpp>
#include <jemgui/input.hpp>
#include <jemgui/layout.hpp>
#include <jemgui/painter.hpp>
#include <jemgui/theme.hpp>
#include <jemgui/types.hpp>
#include <jemgui/widgets.hpp>

namespace jemgui {

template <painter P>
class ctx {
 public:
  explicit ctx(P& painter, const theme& t = themes::dark)
      : p_{painter}, theme_{t} {
    recalculate_scale();
  }

  void set_theme(const theme& t) { theme_ = t; }
  const theme& current_theme() const { return theme_; }

  void recalculate() {
    recalculate_scale();
    for (usize i = 0; i < max_scroll_panels; ++i) scroll_[i] = {};
  }

  void begin_frame(const input_state& input, i32 dt_ms = 0) {
    input_.update(input);
    anims_.tick(dt_ms);
    hot_ = 0;
    active_panel_.scroll_idx = -1;
    layout_.reset();
    container root{};
    root.bounds = rect{{0, 0}, {p_.width(), p_.height()}};
    root.cursor = vec2{s(theme_.padding), s(theme_.padding)};
    root.dir = direction::vertical;
    root.spacing = s(theme_.spacing);
    layout_.push(root);

    i16 pw = static_cast<i16>(p_.width());
    i16 ph = static_cast<i16>(p_.height());
    i16 edge = static_cast<i16>(s(theme_.padding) + s(theme_.corner_radius));
    p_.fill_rect(0, 0, pw, edge, theme_.bg);
    p_.fill_rect(0, static_cast<i16>(ph - edge), pw, edge, theme_.bg);
    p_.fill_rect(0, edge, edge, static_cast<i16>(ph - 2 * edge), theme_.bg);
    p_.fill_rect(static_cast<i16>(pw - edge), edge, edge,
                 static_cast<i16>(ph - 2 * edge), theme_.bg);
  }

  void end_frame() {
    if (!input_.down()) active_ = 0;
  }

  anim_pool& anims() { return anims_; }
  const anim_pool& anims() const { return anims_; }

  void row(i16 height = 0) {
    i16 h = height > 0 ? s(height) : s(theme_.widget_height);
    u16 w = layout_.available_w();
    rect r = layout_.allocate(w, static_cast<u16>(h));
    layout_.push({
        .bounds = r,
        .cursor = r.pos,
        .dir = direction::horizontal,
        .spacing = s(theme_.spacing),
    });
  }

  void col(i16 width = 0) {
    i16 w = width > 0 ? s(width) : static_cast<i16>(layout_.available_w());
    u16 h = layout_.available_h();
    rect r = layout_.allocate(static_cast<u16>(w), h);
    layout_.push({
        .bounds = r,
        .cursor = r.pos,
        .dir = direction::vertical,
        .spacing = s(theme_.spacing),
    });
  }

  void end() {
    if (layout_.depth > 1) layout_.pop();
  }

  void pad(i16 px) { layout_.advance(s(px)); }
  void spacer(i16 px) { layout_.advance(s(px)); }

  void indent(i16 px) {
    auto& c = layout_.top();
    c.cursor.x = static_cast<i16>(c.cursor.x + s(px));
  }

  void same_line(i16 spacing = -1) {
    auto& c = layout_.top();
    if (c.dir == direction::vertical && c.child_count > 0) {
      c.cursor.y =
          static_cast<i16>(c.cursor.y - s(theme_.widget_height) - c.spacing);
      i16 sp = spacing >= 0 ? s(spacing) : c.spacing;
      c.cursor.x = static_cast<i16>(c.cursor.x + sp);
      c.dir = direction::horizontal;
    }
  }

  void push_id(i16 index) { ids_.push(static_cast<id>(index)); }
  void push_id(const char* str) { ids_.push(hash_label(str)); }
  void pop_id() { ids_.pop(); }

  void label(const char* text) {
    u8 fs = font_size();
    i16 tw = draw::text_width(text, fs);
    i16 th = draw::text_height(fs);
    i16 wh = std::max(th, s(theme_.widget_height));
    rect r = layout_.allocate(static_cast<u16>(std::min<i16>(
                                  tw + s(theme_.padding) * 2,
                                  static_cast<i16>(layout_.available_w()))),
                              static_cast<u16>(wh));
    draw::text_left(p_, r, text, theme_.text, fs, s(theme_.padding));
  }

  void label_colored(const char* text, u16 color) {
    u8 fs = font_size();
    i16 tw = draw::text_width(text, fs);
    i16 th = draw::text_height(fs);
    i16 wh = std::max(th, s(theme_.widget_height));
    rect r = layout_.allocate(static_cast<u16>(std::min<i16>(
                                  tw + s(theme_.padding) * 2,
                                  static_cast<i16>(layout_.available_w()))),
                              static_cast<u16>(wh));
    draw::text_left(p_, r, text, color, fs, s(theme_.padding));
  }

  void label_fmt(const char* fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    label(buf);
  }

  bool button(const char* text) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 tw = draw::text_width(text, fs);
    i16 h = s(theme_.widget_height);
    i16 w = static_cast<i16>(tw + s(theme_.padding) * 4);
    if (layout_.top().dir == direction::horizontal)
      w = std::min(w, static_cast<i16>(layout_.available_w()));
    rect r = layout_.allocate(static_cast<u16>(w), static_cast<u16>(h));

    bool hovered = input_.down_in(r);
    bool pressed = false;
    if (hovered) {
      hot_ = wid;
      if (input_.pressed_in(r)) active_ = wid;
    }
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }

    u16 bg_color = theme_.accent;
    if (active_ == wid && hovered)
      bg_color = theme_.accent_press;
    else if (hot_ == wid)
      bg_color = theme_.accent_hover;

    u16 top_c = lighten(bg_color, 60);
    u16 bot_c = darken(bg_color, 40);
    draw::rounded_rect_gradient_v(p_, r, s(theme_.corner_radius), top_c, bot_c);
    draw::text_centered(p_, r, text, theme_.text, fs);
    return pressed;
  }

  bool button_colored(const char* text, u16 color) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 tw = draw::text_width(text, fs);
    i16 h = s(theme_.widget_height);
    i16 w = static_cast<i16>(tw + s(theme_.padding) * 4);
    if (layout_.top().dir == direction::horizontal)
      w = std::min(w, static_cast<i16>(layout_.available_w()));
    rect r = layout_.allocate(static_cast<u16>(w), static_cast<u16>(h));

    bool hovered = input_.down_in(r);
    bool pressed = false;
    if (hovered) {
      hot_ = wid;
      if (input_.pressed_in(r)) active_ = wid;
    }
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }

    u16 bg = color;
    if (active_ == wid && hovered)
      bg = darken(color, 80);
    else if (hot_ == wid)
      bg = lighten(color, 50);

    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius), bg);
    draw::text_centered(p_, r, text, theme_.text, fs);
    return pressed;
  }

  bool toggle(const char* text, bool& value) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    i16 track_w = s(28);
    i16 track_h = s(14);
    i16 tw = draw::text_width(text, fs);
    i16 total_w = static_cast<i16>(tw + track_w + s(theme_.padding) * 3);
    rect r = layout_.allocate(static_cast<u16>(total_w), static_cast<u16>(h));

    bool toggled = false;
    if (input_.pressed_in(r)) active_ = wid;
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) {
        value = !value;
        toggled = true;
      }
      active_ = 0;
    }
    if (input_.down_in(r)) hot_ = wid;

    draw::text_left(p_, r, text, theme_.text, fs, s(theme_.padding));

    i16 track_x = static_cast<i16>(r.x() + tw + s(theme_.padding) * 2);
    i16 track_y = static_cast<i16>(r.y() + (h - track_h) / 2);
    rect track_r = {{track_x, track_y},
                    {static_cast<u16>(track_w), static_cast<u16>(track_h)}};
    i16 track_radius = static_cast<i16>(track_h / 2);

    id anim_id = mix_id(wid, 0xA1);
    anims_.ensure(anim_id, value ? 256 : 0, 180, ease::out_cubic);
    i32 frac = anims_.get(anim_id, value ? 256 : 0);

    u16 track_color = blend_rgb565(theme_.accent, theme_.surface_alt,
                                   static_cast<u8>(frac > 255 ? 255 : frac));
    draw::rounded_rect_fill(p_, track_r, track_radius, track_color);
    draw::rounded_rect_outline(p_, track_r, track_radius, theme_.border);

    i16 off_x = static_cast<i16>(track_x + track_h / 2);
    i16 on_x = static_cast<i16>(track_x + track_w - track_h / 2);
    i16 knob_x = static_cast<i16>(off_x + ((on_x - off_x) * frac >> 8));
    i16 knob_y = static_cast<i16>(track_y + track_h / 2);
    i16 knob_r = static_cast<i16>(track_h / 2 - 2);
    p_.fill_circle(knob_x, knob_y, knob_r, theme_.text);

    return toggled;
  }

  bool checkbox(const char* text, bool& value) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    i16 box_sz = s(14);
    i16 tw = draw::text_width(text, fs);
    i16 total_w = static_cast<i16>(box_sz + tw + s(theme_.padding) * 3);
    rect r = layout_.allocate(static_cast<u16>(total_w), static_cast<u16>(h));

    bool toggled = false;
    if (input_.pressed_in(r)) active_ = wid;
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) {
        value = !value;
        toggled = true;
      }
      active_ = 0;
    }
    if (input_.down_in(r)) hot_ = wid;

    i16 bx = static_cast<i16>(r.x() + s(theme_.padding));
    i16 by = static_cast<i16>(r.y() + (h - box_sz) / 2);
    rect box_r = {{bx, by},
                  {static_cast<u16>(box_sz), static_cast<u16>(box_sz)}};

    id anim_id = mix_id(wid, 0xCB);
    anims_.ensure(anim_id, value ? 256 : 0, 150, ease::out_cubic);
    i32 frac = anims_.get(anim_id, value ? 256 : 0);

    u16 box_bg = blend_rgb565(theme_.accent, theme_.surface_alt,
                              static_cast<u8>(frac > 255 ? 255 : frac));
    draw::rounded_rect_fill(p_, box_r, s(2), box_bg);
    draw::rounded_rect_outline(p_, box_r, s(2), theme_.border);

    if (frac > 32) {
      i16 cx = static_cast<i16>(bx + box_sz / 2);
      i16 cy = static_cast<i16>(by + box_sz / 2);
      i16 mark = static_cast<i16>((box_sz / 4) * frac / 256);
      if (mark < 1) mark = 1;
      i16 x1 = static_cast<i16>(cx - mark);
      i16 x2 = static_cast<i16>(cx + mark);
      i16 y1 = static_cast<i16>(cy - mark);
      i16 y2 = static_cast<i16>(cy + mark);
      p_.hline(x1, cy, static_cast<i16>(x2 - x1 + 1), theme_.text);
      p_.vline(cx, y1, static_cast<i16>(y2 - y1 + 1), theme_.text);
    }

    rect text_r = {{static_cast<i16>(bx + box_sz + s(theme_.padding)), r.y()},
                   {static_cast<u16>(tw), static_cast<u16>(h)}};
    draw::text_left(p_, text_r, text, theme_.text, fs, 0);
    return toggled;
  }

  bool radio(const char* text, i16& current_val, i16 this_val) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    i16 circle_r = s(6);
    i16 tw = draw::text_width(text, fs);
    i16 total_w = static_cast<i16>(circle_r * 2 + tw + s(theme_.padding) * 3);
    rect r = layout_.allocate(static_cast<u16>(total_w), static_cast<u16>(h));

    bool changed = false;
    if (input_.pressed_in(r)) active_ = wid;
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos()) && current_val != this_val) {
        current_val = this_val;
        changed = true;
      }
      active_ = 0;
    }
    if (input_.down_in(r)) hot_ = wid;

    bool selected = current_val == this_val;
    i16 cx = static_cast<i16>(r.x() + s(theme_.padding) + circle_r);
    i16 cy = static_cast<i16>(r.y() + h / 2);

    draw::circle_outline(p_, cx, cy, circle_r, theme_.border);

    id anim_id = mix_id(wid, 0xD1);
    anims_.ensure(anim_id, selected ? 256 : 0, 150, ease::out_cubic);
    i32 frac = anims_.get(anim_id, selected ? 256 : 0);

    if (frac > 32) {
      i16 inner_r = static_cast<i16>((circle_r - 3) * frac / 256);
      if (inner_r < 1) inner_r = 1;
      p_.fill_circle(cx, cy, inner_r, theme_.accent);
    }

    rect text_r = {{static_cast<i16>(cx + circle_r + s(theme_.padding)), r.y()},
                   {static_cast<u16>(tw), static_cast<u16>(h)}};
    draw::text_left(p_, text_r, text, theme_.text, fs, 0);
    return changed;
  }

  bool slider(const char* text, i16& value, i16 min_val, i16 max_val) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 full_w = layout_.available_w();
    rect r = layout_.allocate(full_w, static_cast<u16>(h));

    i16 tw = draw::text_width(text, fs);
    i16 label_w = static_cast<i16>(tw + s(theme_.padding) * 2);
    i16 track_x = static_cast<i16>(r.x() + label_w);
    i16 track_w =
        static_cast<i16>(static_cast<i16>(r.w()) - label_w - s(theme_.padding));
    i16 track_h = s(6);
    i16 track_y = static_cast<i16>(r.y() + (h - track_h) / 2);
    rect track_r = {{track_x, track_y},
                    {static_cast<u16>(track_w), static_cast<u16>(track_h)}};

    bool changed = false;
    if (input_.pressed_in(r)) active_ = wid;
    if (active_ == wid) {
      if (input_.down()) {
        i32 rel = input_.pos().x - track_x;
        rel = std::clamp<i32>(rel, 0, track_w);
        i16 new_val =
            static_cast<i16>(min_val + (rel * (max_val - min_val)) / track_w);
        if (new_val != value) {
          value = new_val;
          changed = true;
        }
      }
      if (!input_.down()) active_ = 0;
    }
    if (input_.down_in(r)) hot_ = wid;

    draw::text_left(p_, r, text, theme_.text, fs, s(theme_.padding));
    draw::rounded_rect_fill(p_, track_r, s(3), theme_.surface_alt);

    i32 range = max_val - min_val;
    i32 target_fill =
        range > 0 ? static_cast<i32>(value - min_val) * track_w / range : 0;
    target_fill = std::clamp<i32>(target_fill, 0, track_w);

    id fill_anim = mix_id(wid, 0xF1);
    anims_.ensure(fill_anim, static_cast<i32>(target_fill), 120,
                  ease::out_cubic);
    i32 fill_w =
        std::clamp<i32>(anims_.get(fill_anim, target_fill), 0, track_w);

    if (fill_w > 0) {
      rect fill_r = {{track_x, track_y},
                     {static_cast<u16>(fill_w), static_cast<u16>(track_h)}};
      draw::rounded_rect_fill(p_, fill_r, s(3), theme_.accent);
    }

    i16 thumb_x = static_cast<i16>(track_x + fill_w);
    i16 thumb_y = static_cast<i16>(r.y() + h / 2);
    i16 thumb_r = s(5);

    id halo_anim = mix_id(wid, 0xF2);
    anims_.ensure(halo_anim, (active_ == wid) ? 256 : 0, 150, ease::out_quad);
    i32 halo_frac = anims_.get(halo_anim, 0);
    if (halo_frac > 16) {
      i16 halo_r = static_cast<i16>(thumb_r + s(3) * halo_frac / 256);
      u8 halo_a = static_cast<u8>(80 * halo_frac / 256);
      u16 halo_c = blend_rgb565(theme_.accent, theme_.bg, halo_a);
      p_.fill_circle(thumb_x, thumb_y, halo_r, halo_c);
    }

    u16 thumb_c = (active_ == wid) ? theme_.accent_press : theme_.accent;
    p_.fill_circle(thumb_x, thumb_y, thumb_r, thumb_c);
    return changed;
  }

  void progress(const char* text, float fraction) {
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 full_w = layout_.available_w();
    rect r = layout_.allocate(full_w, static_cast<u16>(h));

    i16 tw = draw::text_width(text, fs);
    i16 label_w = static_cast<i16>(tw + s(theme_.padding) * 2);
    i16 bar_x = static_cast<i16>(r.x() + label_w);
    i16 bar_w =
        static_cast<i16>(static_cast<i16>(r.w()) - label_w - s(theme_.padding));
    i16 bar_h = s(8);
    i16 bar_y = static_cast<i16>(r.y() + (h - bar_h) / 2);
    rect bar_r = {{bar_x, bar_y},
                  {static_cast<u16>(bar_w), static_cast<u16>(bar_h)}};

    draw::text_left(p_, r, text, theme_.text, fs, s(theme_.padding));
    draw::rounded_rect_fill(p_, bar_r, s(3), theme_.surface_alt);

    float f = std::clamp(fraction, 0.0f, 1.0f);
    i16 fill_w = static_cast<i16>(static_cast<float>(bar_w) * f);
    if (fill_w > 0) {
      rect fill_r = {{bar_x, bar_y},
                     {static_cast<u16>(fill_w), static_cast<u16>(bar_h)}};
      draw::rounded_rect_fill(p_, fill_r, s(3), theme_.accent);
    }
  }

  void gauge(i16 cx, i16 cy, i16 outer_r, i16 inner_r, float fraction,
             u16 fill_color, u16 track_color) {
    i16 start_deg = 135;
    i16 end_deg = 45;
    draw::arc_fill(p_, cx, cy, outer_r, inner_r, start_deg, end_deg,
                   track_color);

    float f = std::clamp(fraction, 0.0f, 1.0f);
    i16 sweep = 270;
    i16 fill_end = static_cast<i16>(
        start_deg + static_cast<i16>(static_cast<float>(sweep) * f));
    if (fill_end >= 360) fill_end = static_cast<i16>(fill_end - 360);
    if (f > 0.001f)
      draw::arc_fill(p_, cx, cy, outer_r, inner_r, start_deg, fill_end,
                     fill_color);
  }

  void badge(const char* text, u16 color) {
    u8 fs = font_size();
    i16 tw = draw::text_width(text, fs);
    i16 th = draw::text_height(fs);
    i16 pad_h = s(2);
    i16 pad_w = s(6);
    i16 bw = static_cast<i16>(tw + pad_w * 2);
    i16 bh = static_cast<i16>(th + pad_h * 2);
    rect r = layout_.allocate(static_cast<u16>(bw), static_cast<u16>(bh));
    draw::rounded_rect_fill(p_, r, static_cast<i16>(bh / 2), color);
    draw::text_centered(p_, r, text, theme_.text, fs);
  }

  void header(const char* text, u16 bg_color = 0) {
    u8 fs = font_size();
    i16 h = static_cast<i16>(s(theme_.widget_height) + s(4));
    u16 full_w = layout_.available_w();
    rect r = layout_.allocate(full_w, static_cast<u16>(h));
    u16 bg = bg_color != 0 ? bg_color : theme_.accent;
    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius), bg);
    draw::text_centered(p_, r, text, theme_.text, fs);
  }

  bool tile(const char* text, u16 color, u16 tile_w, u16 tile_h) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    rect r = layout_.allocate(tile_w, tile_h);

    bool hovered = input_.down_in(r);
    bool pressed = false;
    if (hovered) {
      hot_ = wid;
      if (input_.pressed_in(r)) active_ = wid;
    }
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }

    u16 bg = color;
    if (active_ == wid && hovered)
      bg = darken(color, 60);
    else if (hot_ == wid)
      bg = lighten(color, 40);

    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius + 2), bg);
    draw::text_centered(p_, r, text, theme_.text, fs);
    return pressed;
  }

  void stat_card(const char* label, const char* value, u16 accent_color) {
    u8 fs = font_size();
    u8 vfs = static_cast<u8>(fs < 2 ? 2 : fs);
    i16 lh = draw::text_height(fs);
    i16 vh = draw::text_height(vfs);
    i16 pad = s(theme_.padding);
    i16 h = static_cast<i16>(lh + vh + pad * 2 + s(2));
    u16 w = layout_.available_w();
    rect r = layout_.allocate(w, static_cast<u16>(h));

    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius), theme_.surface_alt);

    i16 bar_w = s(3);
    p_.fill_rect(r.x(), static_cast<i16>(r.y() + s(2)), bar_w,
                 static_cast<i16>(r.h() - s(4)), accent_color);

    i16 lx = static_cast<i16>(r.x() + bar_w + pad);
    u16 tw = static_cast<u16>(r.w() - bar_w - pad * 2);

    rect label_r = {{lx, static_cast<i16>(r.y() + pad)},
                    {tw, static_cast<u16>(lh)}};
    draw::text_left(p_, label_r, label, theme_.text_dim, fs, 0);

    rect val_r = {{lx, static_cast<i16>(r.y() + pad + lh + s(2))},
                  {tw, static_cast<u16>(vh)}};
    draw::text_left(p_, val_r, value, accent_color, vfs, 0);
  }

  bool list_item(const char* text, bool selected) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 full_w = layout_.available_w();
    rect r = layout_.allocate(full_w, static_cast<u16>(h));

    bool pressed = false;
    if (input_.pressed_in(r)) active_ = wid;
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }
    if (input_.down_in(r)) hot_ = wid;

    u16 bg = selected ? theme_.accent
                      : (hot_ == wid ? theme_.surface_alt : theme_.surface);
    draw::rounded_rect_fill(p_, r, s(2), bg);
    draw::text_left(p_, r, text, theme_.text, fs, s(theme_.padding));
    return pressed;
  }

  bool spinner(const char* label, i16& value, i16 min_val, i16 max_val,
               i16 step = 1) {
    id wid = ids_.make(label);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 full_w = layout_.available_w();
    rect r = layout_.allocate(full_w, static_cast<u16>(h));

    i16 tw = draw::text_width(label, fs);
    i16 label_w = static_cast<i16>(tw + s(theme_.padding) * 2);
    i16 btn_w = h;

    rect minus_r = {{static_cast<i16>(r.x() + label_w), r.y()},
                    {static_cast<u16>(btn_w), static_cast<u16>(h)}};
    rect plus_r = {{static_cast<i16>(r.right() - btn_w), r.y()},
                   {static_cast<u16>(btn_w), static_cast<u16>(h)}};
    rect val_r = {
        {static_cast<i16>(minus_r.right()), r.y()},
        {static_cast<u16>(plus_r.x() - minus_r.right()), static_cast<u16>(h)}};

    bool changed = false;

    id minus_id = mix_id(wid, 0xE0);
    if (input_.pressed_in(minus_r)) active_ = minus_id;
    if (active_ == minus_id && input_.released()) {
      if (minus_r.contains(input_.pos()) && value > min_val) {
        value = static_cast<i16>(value - step);
        if (value < min_val) value = min_val;
        changed = true;
      }
      active_ = 0;
    }

    id plus_id = mix_id(wid, 0xE1);
    if (input_.pressed_in(plus_r)) active_ = plus_id;
    if (active_ == plus_id && input_.released()) {
      if (plus_r.contains(input_.pos()) && value < max_val) {
        value = static_cast<i16>(value + step);
        if (value > max_val) value = max_val;
        changed = true;
      }
      active_ = 0;
    }

    draw::text_left(p_, r, label, theme_.text, fs, s(theme_.padding));

    u16 minus_bg =
        (active_ == minus_id) ? theme_.accent_press : theme_.surface_alt;
    draw::rounded_rect_fill(p_, minus_r, s(2), minus_bg);
    draw::text_centered(p_, minus_r, "-", theme_.text, fs);

    u16 plus_bg =
        (active_ == plus_id) ? theme_.accent_press : theme_.surface_alt;
    draw::rounded_rect_fill(p_, plus_r, s(2), plus_bg);
    draw::text_centered(p_, plus_r, "+", theme_.text, fs);

    draw::rounded_rect_fill(p_, val_r, 0, theme_.surface);
    draw::rounded_rect_outline(p_, val_r, 0, theme_.border);
    char vbuf[16];
    snprintf(vbuf, sizeof(vbuf), "%d", value);
    draw::text_centered(p_, val_r, vbuf, theme_.text, fs);

    return changed;
  }

  bool button_fill(const char* text) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 w = layout_.available_w();
    rect r = layout_.allocate(w, static_cast<u16>(h));

    bool hovered = input_.down_in(r);
    bool pressed = false;
    if (hovered) {
      hot_ = wid;
      if (input_.pressed_in(r)) active_ = wid;
    }
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }

    u16 bg_color = theme_.accent;
    if (active_ == wid && hovered)
      bg_color = theme_.accent_press;
    else if (hot_ == wid)
      bg_color = theme_.accent_hover;

    u16 top_c = lighten(bg_color, 60);
    u16 bot_c = darken(bg_color, 40);
    draw::rounded_rect_gradient_v(p_, r, s(theme_.corner_radius), top_c, bot_c);
    draw::text_centered(p_, r, text, theme_.text, fs);
    return pressed;
  }

  bool button_fill_colored(const char* text, u16 color) {
    id wid = ids_.make(text);
    u8 fs = font_size();
    i16 h = s(theme_.widget_height);
    u16 w = layout_.available_w();
    rect r = layout_.allocate(w, static_cast<u16>(h));

    bool hovered = input_.down_in(r);
    bool pressed = false;
    if (hovered) {
      hot_ = wid;
      if (input_.pressed_in(r)) active_ = wid;
    }
    if (active_ == wid && input_.released()) {
      if (r.contains(input_.pos())) pressed = true;
      active_ = 0;
    }

    u16 bg = color;
    if (active_ == wid && hovered)
      bg = darken(color, 80);
    else if (hot_ == wid)
      bg = lighten(color, 50);

    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius), bg);
    draw::text_centered(p_, r, text, theme_.text, fs);
    return pressed;
  }

  void separator() {
    i16 sp = s(theme_.spacing);
    layout_.advance(sp);
    u16 w = layout_.available_w();
    rect r = layout_.allocate(w, 1);
    p_.hline(r.x(), r.y(), static_cast<i16>(r.w()), theme_.border);
    layout_.advance(sp);
  }

  void panel_begin(const char* title = nullptr, bool with_shadow = false) {
    i16 pad_val = s(theme_.padding);
    u16 w = layout_.available_w();
    u16 h = layout_.available_h();
    rect r = layout_.allocate(w, h);

    if (with_shadow) {
      draw::shadow(p_, r, s(theme_.corner_radius), theme_.bg, 3, s(2), s(2));
    }

    draw::rounded_rect_fill(p_, r, s(theme_.corner_radius), theme_.surface);
    draw::rounded_rect_outline(p_, r, s(theme_.corner_radius), theme_.border);

    rect inner = r.shrink(pad_val);

    if (title) {
      u8 fs = font_size();
      i16 th = draw::text_height(fs);
      i16 title_h = static_cast<i16>(th + pad_val);
      rect title_r = {inner.pos, {inner.w(), static_cast<u16>(title_h)}};
      draw::text_left(p_, title_r, title, theme_.text, fs, 0);
      p_.hline(inner.x(), static_cast<i16>(inner.y() + title_h),
               static_cast<i16>(inner.w()), theme_.border);
      inner.pos.y = static_cast<i16>(inner.y() + title_h + pad_val);
      inner.size.h = static_cast<u16>(
          inner.h() > title_h + pad_val ? inner.h() - title_h - pad_val : 0);
    }

    id pid = title ? ids_.make(title) : ids_.make("__panel__");
    i16 si = find_scroll(pid);

    active_panel_.scroll_idx = si;
    active_panel_.clip = inner;
    active_panel_.content_start_y = inner.y();

    rect content_bounds = inner;
    if (si >= 0 && scroll_[si].content_h > static_cast<i16>(inner.h())) {
      i16 bar_space = static_cast<i16>(s(4) + s(2));
      content_bounds.size.w = static_cast<u16>(
          inner.w() > bar_space ? inner.w() - bar_space : inner.w());
    }
    content_bounds.size.h = 4096;

    vec2 cursor = content_bounds.pos;
    if (si >= 0) cursor.y = static_cast<i16>(cursor.y - scroll_[si].offset);

    p_.set_clip(inner);
    layout_.push({
        .bounds = content_bounds,
        .cursor = cursor,
        .dir = direction::vertical,
        .spacing = s(theme_.spacing),
    });
  }

  void panel_end() {
    i16 si = active_panel_.scroll_idx;
    if (si >= 0) {
      auto& se = scroll_[si];
      auto& c = layout_.top();
      i16 content_h = static_cast<i16>(
          c.cursor.y - (active_panel_.content_start_y - se.offset));
      se.content_h = content_h;

      i16 visible_h = static_cast<i16>(active_panel_.clip.h());
      i16 max_scroll = content_h > visible_h
                           ? static_cast<i16>(content_h - visible_h)
                           : static_cast<i16>(0);

      if (active_ == 0 && input_.held()) {
        if (active_panel_.clip.contains(input_.pos()) ||
            active_panel_.clip.contains(input_.prev_pos())) {
          i16 dy = static_cast<i16>(input_.pos().y - input_.prev_pos().y);
          se.offset = static_cast<i16>(se.offset - dy);
          se.velocity = static_cast<i16>(-dy * 4);
        }
      } else if (!input_.down() && se.velocity != 0) {
        se.offset = static_cast<i16>(se.offset + se.velocity / 4);
        if (se.velocity > 0)
          se.velocity = static_cast<i16>(se.velocity - 1);
        else if (se.velocity < 0)
          se.velocity = static_cast<i16>(se.velocity + 1);
      }

      if (se.offset < 0) se.offset = 0;
      if (se.offset > max_scroll) se.offset = max_scroll;

      if (max_scroll > 0) {
        i16 bar_w = s(3);
        i16 bar_x = static_cast<i16>(active_panel_.clip.right() - bar_w);
        i16 bar_h = static_cast<i16>(active_panel_.clip.h());
        i16 thumb_h =
            static_cast<i16>(static_cast<i32>(bar_h) * visible_h / content_h);
        if (thumb_h < s(8)) thumb_h = s(8);
        i16 thumb_y = static_cast<i16>(active_panel_.clip.y() +
                                       static_cast<i32>(se.offset) *
                                           (bar_h - thumb_h) / max_scroll);
        p_.fill_rect(bar_x, active_panel_.clip.y(), bar_w, bar_h,
                     theme_.surface_alt);
        draw::rounded_rect_fill(
            p_,
            rect{{bar_x, thumb_y},
                 {static_cast<u16>(bar_w), static_cast<u16>(thumb_h)}},
            static_cast<i16>(bar_w / 2), theme_.border);
      }

      p_.clear_clip();
      active_panel_.scroll_idx = -1;
    }
    end();
  }

  void icon(const u16* bitmap, u16 w, u16 h) {
    rect r = layout_.allocate(w, h);
    for (i16 row = 0; row < static_cast<i16>(h); ++row) {
      for (i16 col = 0; col < static_cast<i16>(w); ++col) {
        u16 px = bitmap[row * w + col];
        if (px != 0)
          p_.pixel(static_cast<i16>(r.x() + col), static_cast<i16>(r.y() + row),
                   px);
      }
    }
  }

  bool is_hot(id widget) const { return hot_ == widget; }
  bool is_active(id widget) const { return active_ == widget; }
  vec2 cursor() const { return layout_.top().cursor; }
  extent available() const {
    return {layout_.available_w(), layout_.available_h()};
  }
  i16 scale_value() const { return scale_; }
  const input_cache& input() const { return input_; }
  P& painter() { return p_; }

 private:
  void recalculate_scale() {
    i32 pw = p_.width();
    i32 ph = p_.height();
    i32 sx = (pw << 8) / 320;
    i32 sy = (ph << 8) / 240;
    scale_ = static_cast<i16>(sx < sy ? sx : sy);
    if (scale_ < 64) scale_ = 64;
  }

  i16 s(i16 value) const { return scale(value, scale_); }

  u8 font_size() const {
    i16 scaled = s(static_cast<i16>(theme_.font_size));
    return scaled < 1 ? 1 : static_cast<u8>(scaled);
  }

  static constexpr usize max_scroll_panels = 4;

  struct scroll_entry {
    id pid = 0;
    i16 offset = 0;
    i16 content_h = 0;
    i16 velocity = 0;
  };

  struct panel_info {
    i16 scroll_idx = -1;
    rect clip = {};
    i16 content_start_y = 0;
  };

  i16 find_scroll(id pid) {
    for (usize i = 0; i < max_scroll_panels; ++i) {
      if (scroll_[i].pid == pid) return static_cast<i16>(i);
    }
    for (usize i = 0; i < max_scroll_panels; ++i) {
      if (scroll_[i].pid == 0) {
        scroll_[i].pid = pid;
        return static_cast<i16>(i);
      }
    }
    return -1;
  }

  P& p_;
  theme theme_;
  input_cache input_;
  layout_stack layout_;
  id_stack ids_;
  anim_pool anims_;
  id hot_ = 0;
  id active_ = 0;
  i16 scale_ = 256;
  scroll_entry scroll_[max_scroll_panels] = {};
  panel_info active_panel_ = {};
};

}  // namespace jemgui
