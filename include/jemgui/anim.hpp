#pragma once

#include <jemgui/types.hpp>

namespace jemgui {

enum class ease : u8 {
  linear,
  in_quad,
  out_quad,
  in_out_quad,
  in_cubic,
  out_cubic,
  in_out_cubic,
  out_back,
  out_bounce,
};

inline i32 ease_apply(ease e, i32 t, i32 d) {
  if (d <= 0) return 256;
  if (t <= 0) return 0;
  if (t >= d) return 256;

  i32 f = (t << 8) / d;

  switch (e) {
    case ease::linear:
      return f;

    case ease::in_quad:
      return (f * f) >> 8;

    case ease::out_quad: {
      i32 inv = 256 - f;
      return 256 - ((inv * inv) >> 8);
    }

    case ease::in_out_quad:
      if (f < 128) {
        i32 h = f * 2;
        return (h * h) >> 9;
      } else {
        i32 h = (256 - f) * 2;
        return 256 - ((h * h) >> 9);
      }

    case ease::in_cubic:
      return (((f * f) >> 8) * f) >> 8;

    case ease::out_cubic: {
      i32 inv = 256 - f;
      return 256 - ((((inv * inv) >> 8) * inv) >> 8);
    }

    case ease::in_out_cubic:
      if (f < 128) {
        i32 h = f * 2;
        return (((h * h) >> 8) * h) >> 9;
      } else {
        i32 h = (256 - f) * 2;
        return 256 - ((((h * h) >> 8) * h) >> 9);
      }

    case ease::out_back: {
      i32 inv = 256 - f;
      i32 overshoot = 434;
      i32 base = 256 - ((inv * inv) >> 8);
      i32 extra = ((inv * inv) >> 8) * overshoot >> 8;
      i32 result = base + ((f * extra) >> 8);
      return result > 280 ? 280 : (result < 0 ? 0 : result);
    }

    case ease::out_bounce: {
      if (f < 92) {
        return (f * f * 756) >> 16;
      } else if (f < 184) {
        i32 t2 = f - 138;
        return ((t2 * t2 * 756) >> 16) + 192;
      } else if (f < 230) {
        i32 t2 = f - 207;
        return ((t2 * t2 * 756) >> 16) + 240;
      } else {
        i32 t2 = f - 243;
        return ((t2 * t2 * 756) >> 16) + 252;
      }
    }

    default:
      return f;
  }
}

struct anim_slot {
  id target = 0;
  i32 from = 0;
  i32 to = 0;
  i32 current = 0;
  i32 elapsed = 0;
  i32 duration = 0;
  ease curve = ease::linear;
  bool active = false;
};

struct anim_pool {
  static constexpr usize max_anims = 32;
  anim_slot slots[max_anims] = {};

  void tick(i32 dt_ms) {
    if (dt_ms <= 0) return;
    for (usize i = 0; i < max_anims; ++i) {
      auto& s = slots[i];
      if (!s.active) continue;
      s.elapsed += dt_ms;
      if (s.elapsed >= s.duration) {
        s.current = s.to;
        s.active = false;
      } else {
        i32 f = ease_apply(s.curve, s.elapsed, s.duration);
        s.current = s.from + ((s.to - s.from) * f >> 8);
      }
    }
  }

  i32 get(id target, i32 fallback) const {
    for (usize i = 0; i < max_anims; ++i) {
      if (slots[i].target == target) return slots[i].current;
    }
    return fallback;
  }

  bool running(id target) const {
    for (usize i = 0; i < max_anims; ++i) {
      if (slots[i].target == target && slots[i].active) return true;
    }
    return false;
  }

  void start(id target, i32 from, i32 to, i32 duration_ms,
             ease curve = ease::out_cubic) {
    for (usize i = 0; i < max_anims; ++i) {
      if (slots[i].target == target) {
        auto& s = slots[i];
        s.from = s.current;
        s.to = to;
        s.elapsed = 0;
        s.duration = duration_ms;
        s.curve = curve;
        s.active = true;
        return;
      }
    }
    for (usize i = 0; i < max_anims; ++i) {
      if (!slots[i].active && slots[i].target == 0) {
        auto& s = slots[i];
        s.target = target;
        s.from = from;
        s.to = to;
        s.current = from;
        s.elapsed = 0;
        s.duration = duration_ms;
        s.curve = curve;
        s.active = true;
        return;
      }
    }
    usize victim = 0;
    i32 best_score = -1;
    for (usize i = 0; i < max_anims; ++i) {
      if (!slots[i].active) {
        victim = i;
        best_score = 0x7FFFFFFF;
        break;
      }
      i32 remaining = slots[i].duration - slots[i].elapsed;
      if (remaining < best_score || best_score < 0) {
        best_score = remaining;
        victim = i;
      }
    }
    auto& s = slots[victim];
    s.target = target;
    s.from = from;
    s.to = to;
    s.current = from;
    s.elapsed = 0;
    s.duration = duration_ms;
    s.curve = curve;
    s.active = true;
  }

  void ensure(id target, i32 to, i32 duration_ms,
              ease curve = ease::out_cubic) {
    for (usize i = 0; i < max_anims; ++i) {
      if (slots[i].target == target) {
        if (slots[i].to == to && !slots[i].active) return;
        if (slots[i].to == to && slots[i].active) return;
        start(target, slots[i].current, to, duration_ms, curve);
        return;
      }
    }
    start(target, to, to, 0, curve);
  }
};

}  // namespace jemgui
