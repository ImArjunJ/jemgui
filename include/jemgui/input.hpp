#pragma once

#include <jemgui/types.hpp>

namespace jemgui {

struct input_state {
  vec2 touch_pos = {};
  bool touch_down = false;
};

struct input_cache {
  input_state current = {};
  input_state previous = {};

  void update(const input_state& next) {
    previous = current;
    current = next;
    if (!current.touch_down && previous.touch_down) {
      current.touch_pos = previous.touch_pos;
    }
  }

  bool pressed() const { return current.touch_down && !previous.touch_down; }
  bool released() const { return !current.touch_down && previous.touch_down; }
  bool held() const { return current.touch_down && previous.touch_down; }
  bool down() const { return current.touch_down; }

  vec2 pos() const { return current.touch_pos; }
  vec2 prev_pos() const { return previous.touch_pos; }

  bool pressed_in(rect r) const { return pressed() && r.contains(pos()); }
  bool released_in(rect r) const { return released() && r.contains(pos()); }
  bool held_in(rect r) const { return held() && r.contains(pos()); }
  bool down_in(rect r) const { return down() && r.contains(pos()); }
  bool hovering(rect r) const { return down() && r.contains(pos()); }
};

}  // namespace jemgui
