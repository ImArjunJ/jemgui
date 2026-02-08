#include <jemgui/jemgui.hpp>
#include <jstm/core.hpp>
#include <jstm/drivers/ili9341.hpp>
#include <jstm/drivers/xpt2046.hpp>
#include <jstm/hal/fmc_lcd.hpp>
#include <jstm/hal/gpio.hpp>
#include <jstm/hal/spi_bus.hpp>
#include <jstm/time.hpp>

using namespace jstm;
using namespace jemgui;

JEMGUI_FRAMEBUF(framebuf, 320, 240);

int main() {
  hal::system_init();
  delay_ms(100);

  u8 rotation = 1;

  auto lcd_addr = hal::fmc_lcd_init();

  hal::output_pin rst(GPIOB, GPIO_PIN_11, true);
  hal::output_pin bl(GPIOF, GPIO_PIN_6, false);

  drivers::ili9341 display({
      .cmd_addr = lcd_addr.cmd,
      .data_addr = lcd_addr.data,
      .rst_port = GPIOB,
      .rst_pin = GPIO_PIN_11,
      .bl_port = GPIOF,
      .bl_pin = GPIO_PIN_6,
  });

  auto dr = display.init();
  if (!dr) {
    log::error("display init failed: %s", dr.error().message);
    while (true) delay_ms(1000);
  }
  display.set_rotation(rotation);

  hal::spi_bus touch_spi(SPI1,
                         {
                             .baudrate_prescaler = SPI_BAUDRATEPRESCALER_128,
                             .cpol = SPI_POLARITY_LOW,
                             .cpha = SPI_PHASE_1EDGE,
                             .sck_port = GPIOA,
                             .sck_pin = GPIO_PIN_5,
                             .mosi_port = GPIOA,
                             .mosi_pin = GPIO_PIN_7,
                             .miso_port = GPIOA,
                             .miso_pin = GPIO_PIN_6,
                             .af = GPIO_AF5_SPI1,
                         });

  hal::output_pin touch_cs(GPIOC, GPIO_PIN_4, true);
  hal::input_pin touch_irq(GPIOC, GPIO_PIN_5, hal::pull::up);

  drivers::xpt2046 touch(touch_spi, touch_cs, &touch_irq);

  auto tr = touch.init();
  if (!tr) {
    log::error("touch init failed: %s", tr.error().message);
    while (true) delay_ms(1000);
  }
  touch.set_screen_size(display.width(), display.height());
  touch.set_rotation(rotation);

  hal::output_pin led_green(GPIOB, GPIO_PIN_0, false);
  hal::output_pin led_blue(GPIOB, GPIO_PIN_7, false);
  hal::output_pin led_red(GPIOB, GPIO_PIN_14, false);

  canvas<drivers::ili9341> fb(display, framebuf);
  ctx ui(fb);

  bool use_dark = true;
  bool toggle_a = false;
  bool toggle_b = true;
  bool check_a = true;
  bool check_b = false;
  bool check_c = true;
  i16 radio_val = 0;
  i16 slider_val = 50;
  i16 slider_b = 25;
  i16 brightness = 80;
  bool led_g = false;
  bool led_b = false;
  bool led_r = false;
  bool blink_en = false;

  u32 frame = 0;
  u32 last_tick = millis();
  u32 blink_ctr = 0;
  bool blink_phase = false;

  float progress_val = 0.0f;
  bool progress_fwd = true;

  while (true) {
    u32 now = millis();
    i32 dt = static_cast<i32>(now - last_tick);
    if (dt > 100) dt = 16;
    last_tick = now;

    input_state input;
    if (touch.touched()) {
      auto tp = touch.read();
      if (tp.x != drivers::xpt2046::INVALID_COORD) {
        input.touch_pos = {tp.x, tp.y};
        input.touch_down = true;
      }
    }

    ui.begin_frame(input, dt);

    ui.panel_begin("jemgui demo");

    ui.row();
    if (ui.button(use_dark ? "light" : "dark")) {
      use_dark = !use_dark;
      ui.set_theme(use_dark ? themes::dark : themes::light);
    }
    if (ui.button("rotate")) {
      rotation = static_cast<u8>((rotation + 1) & 3);
      display.set_rotation(rotation);
      touch.set_screen_size(display.width(), display.height());
      touch.set_rotation(rotation);
      fb.reinit();
      ui.recalculate();
    }
    ui.label_fmt("f:%lu", frame);
    ui.end();

    ui.separator();

    ui.label("buttons");
    ui.row();
    if (ui.button("click me")) {
      led_g = !led_g;
    }
    ui.button_colored("success", ui.current_theme().success);
    ui.button_colored("danger", ui.current_theme().danger);
    ui.end();

    ui.separator();

    ui.label("toggles");
    ui.toggle("enable", toggle_a);
    ui.toggle("animate", toggle_b);

    ui.separator();

    ui.label("checkboxes");
    ui.checkbox("option a", check_a);
    ui.checkbox("option b", check_b);
    ui.checkbox("option c", check_c);

    ui.separator();

    ui.label("radio");
    ui.radio("choice 1", radio_val, 0);
    ui.radio("choice 2", radio_val, 1);
    ui.radio("choice 3", radio_val, 2);

    ui.separator();

    ui.label("sliders");
    ui.slider("value", slider_val, 0, 100);
    ui.slider("speed", slider_b, 0, 50);

    ui.separator();

    ui.label("progress");
    ui.progress("load", progress_val);

    if (toggle_b) {
      float step = 0.005f * static_cast<float>(dt);
      if (progress_fwd) {
        progress_val += step;
        if (progress_val >= 1.0f) {
          progress_val = 1.0f;
          progress_fwd = false;
        }
      } else {
        progress_val -= step;
        if (progress_val <= 0.0f) {
          progress_val = 0.0f;
          progress_fwd = true;
        }
      }
    }

    ui.separator();

    ui.label("gauge");
    {
      i16 gauge_h = 60;
      ui.spacer(gauge_h);

      auto avail = ui.available();
      i16 gcx = static_cast<i16>(ui.cursor().x + avail.w / 2);
      i16 gcy = static_cast<i16>(ui.cursor().y - gauge_h / 2 + 4);
      i16 outer = 26;
      i16 inner = 18;
      float gf = static_cast<float>(slider_val) / 100.0f;
      ui.gauge(gcx, gcy, outer, inner, gf, ui.current_theme().accent,
               ui.current_theme().surface_alt);

      char vbuf[8];
      snprintf(vbuf, sizeof(vbuf), "%d", slider_val);
      u8 fs = 1;
      i16 vtw = draw::text_width(vbuf, fs);
      i16 vth = draw::text_height(fs);
      jemgui::rect vr = {
          {static_cast<i16>(gcx - vtw / 2), static_cast<i16>(gcy - vth / 2)},
          {static_cast<u16>(vtw), static_cast<u16>(vth)}};
      draw::text_centered(ui.painter(), vr, vbuf, ui.current_theme().text, fs);
    }

    ui.separator();

    ui.label("badges");
    ui.row(16);
    ui.badge("ok", ui.current_theme().success);
    ui.badge("warn", ui.current_theme().warning);
    ui.badge("err", ui.current_theme().danger);
    ui.badge("info", ui.current_theme().accent);
    ui.end();

    ui.separator();

    ui.label("led control");
    ui.row();
    ui.toggle("g", led_g);
    ui.toggle("b", led_b);
    ui.toggle("r", led_r);
    ui.end();
    ui.toggle("blink", blink_en);
    ui.slider("bright", brightness, 0, 100);

    ui.separator();

    if (input.touch_down) {
      ui.label_fmt("touch: %d, %d", input.touch_pos.x, input.touch_pos.y);
    } else {
      ui.label("touch: --");
    }
    ui.label_fmt("rot: %u  scale: %d", rotation, ui.scale_value());

    ui.panel_end();

    ui.end_frame();
    fb.flush();

    if (blink_en) {
      blink_ctr++;
      if (blink_ctr >= 15) {
        blink_ctr = 0;
        blink_phase = !blink_phase;
      }
    } else {
      blink_phase = true;
      blink_ctr = 0;
    }

    bool phase = blink_phase;
    (led_g && phase) ? led_green.high() : led_green.low();
    (led_b && phase) ? led_blue.high() : led_blue.low();
    (led_r && phase) ? led_red.high() : led_red.low();

    ++frame;
  }
}
