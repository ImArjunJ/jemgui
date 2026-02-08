#pragma once

#include <jemgui/types.hpp>

namespace jemgui {

constexpr id fnv1a(const char* str) {
  id hash = 2166136261u;
  while (*str) {
    hash ^= static_cast<u8>(*str++);
    hash *= 16777619u;
  }
  return hash;
}

constexpr id mix_id(id base, id extra) {
  base ^= extra + 0x9e3779b9u + (base << 6) + (base >> 2);
  return base;
}

constexpr id hash_label(const char* label) { return fnv1a(label); }

struct id_stack {
  static constexpr usize max_depth = 8;

  id entries[max_depth] = {};
  usize depth = 0;

  void push(id val) {
    if (depth < max_depth) {
      entries[depth++] = val;
    }
  }

  void pop() {
    if (depth > 0) {
      --depth;
    }
  }

  id current() const {
    id combined = 0;
    for (usize i = 0; i < depth; ++i) {
      combined = mix_id(combined, entries[i]);
    }
    return combined;
  }

  id make(const char* label) const {
    return mix_id(current(), hash_label(label));
  }

  id make(id raw) const { return mix_id(current(), raw); }
};

}  // namespace jemgui
