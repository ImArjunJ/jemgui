#pragma once

#include <jemgui/types.hpp>

namespace jemgui {

enum class direction : u8 { vertical, horizontal };

struct container {
  rect bounds = {};
  vec2 cursor = {};
  direction dir = direction::vertical;
  i16 spacing = 0;
  i16 child_count = 0;
};

struct layout_stack {
  static constexpr usize max_depth = 8;

  container entries[max_depth] = {};
  usize depth = 0;

  void reset() {
    for (usize i = 0; i < max_depth; ++i) {
      entries[i] = container{};
    }
    depth = 0;
  }

  void push(container c) {
    if (depth < max_depth) {
      entries[depth++] = c;
    }
  }

  void pop() {
    if (depth > 0) {
      --depth;
    }
  }

  container& top() { return entries[depth - 1]; }
  const container& top() const { return entries[depth - 1]; }
  bool empty() const { return depth == 0; }

  rect allocate(u16 w, u16 h) {
    auto& c = top();
    rect r = {{c.cursor.x, c.cursor.y}, {w, h}};

    if (c.dir == direction::horizontal) {
      c.cursor.x = static_cast<i16>(c.cursor.x + w + c.spacing);
    } else {
      c.cursor.y = static_cast<i16>(c.cursor.y + h + c.spacing);
    }

    c.child_count++;
    return r;
  }

  u16 available_w() const {
    auto& c = top();
    i16 remaining = static_cast<i16>(c.bounds.right() - c.cursor.x);
    return remaining > 0 ? static_cast<u16>(remaining) : 0;
  }

  u16 available_h() const {
    auto& c = top();
    i16 remaining = static_cast<i16>(c.bounds.bottom() - c.cursor.y);
    return remaining > 0 ? static_cast<u16>(remaining) : 0;
  }

  void advance(i16 px) {
    auto& c = top();
    if (c.dir == direction::horizontal) {
      c.cursor.x = static_cast<i16>(c.cursor.x + px);
    } else {
      c.cursor.y = static_cast<i16>(c.cursor.y + px);
    }
  }
};

}  // namespace jemgui
