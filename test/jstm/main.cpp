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
  i16 page = 0;

  bool toggle_a = false;
  bool toggle_b = true;
  bool check_a = true;
  bool check_b = false;
  i16 radio_val = 0;
  i16 slider_val = 50;
  i16 slider_b = 25;
  float progress_val = 0.0f;
  bool progress_fwd = true;

  i16 speed = 35;
  i16 distance = 125;
  i16 ride_time = 42;
  i16 battery = 78;

  i16 selected_file = 0;
  i16 copies = 1;
  bool duplex = false;
  bool high_quality = true;

  bool led_g = false;
  bool led_b = false;
  bool led_r = false;

  u32 frame = 0;
  u32 last_tick = millis();

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

    {
      auto& p = ui.painter();
      i16 nav_h =
          jemgui::scale(static_cast<i16>(ui.current_theme().padding * 2 +
                                         ui.current_theme().widget_height),
                        ui.scale_value());
      p.fill_rect(0, 0, static_cast<i16>(p.width()), nav_h,
                  ui.current_theme().bg);
    }

    ui.row();
    auto tab = [&](const char* text, i16 idx) {
      u16 c = (page == idx) ? ui.current_theme().accent
                            : ui.current_theme().surface_alt;
      if (ui.button_colored(text, c)) page = idx;
    };
    tab("wdg", 0);
    tab("dash", 1);
    tab("home", 2);
    tab("ctrl", 3);
    ui.end();

    ui.push_id(page);
    ui.panel_begin(nullptr);

    if (page == 0) {
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

      ui.toggle("enable", toggle_a);
      ui.toggle("animate", toggle_b);

      ui.separator();

      ui.checkbox("option a", check_a);
      ui.checkbox("option b", check_b);

      ui.separator();

      ui.radio("choice 1", radio_val, 0);
      ui.radio("choice 2", radio_val, 1);

      ui.separator();

      ui.slider("value", slider_val, 0, 100);
      ui.slider("speed", slider_b, 0, 50);

      ui.separator();

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

      ui.row(16);
      ui.badge("ok", ui.current_theme().success);
      ui.badge("warn", ui.current_theme().warning);
      ui.badge("err", ui.current_theme().danger);
      ui.badge("info", ui.current_theme().accent);
      ui.end();

      ui.separator();

      ui.row();
      ui.toggle("g", led_g);
      ui.toggle("b", led_b);
      ui.toggle("r", led_r);
      ui.end();
    }

    else if (page == 1) {
      ui.header("DASHBOARD");
      ui.spacer(4);

      {
        i16 sv = ui.scale_value();
        i16 outer = jemgui::scale(28, sv);
        i16 inner = jemgui::scale(20, sv);
        i16 gap = jemgui::scale(3, sv);
        i16 unit_h = draw::text_height(1);

        i16 area_top = ui.cursor().y;
        i16 area_cx = static_cast<i16>(ui.cursor().x + ui.available().w / 2);

        ui.spacer(76);

        i16 gcy = static_cast<i16>(area_top + outer + jemgui::scale(2, sv));
        i16 gcx = area_cx;
        float gf = static_cast<float>(speed) / 60.0f;
        ui.gauge(gcx, gcy, outer, inner, gf, ui.current_theme().accent,
                 ui.current_theme().surface_alt);

        char vbuf[8];
        snprintf(vbuf, sizeof(vbuf), "%d", speed);
        u8 vfs = static_cast<u8>(jemgui::scale(2, sv));
        if (vfs < 1) vfs = 1;
        i16 vtw = draw::text_width(vbuf, vfs);
        i16 vth = draw::text_height(vfs);
        jemgui::rect vr = {
            {static_cast<i16>(gcx - vtw / 2), static_cast<i16>(gcy - vth / 2)},
            {static_cast<u16>(vtw), static_cast<u16>(vth)}};
        draw::text_centered(ui.painter(), vr, vbuf, ui.current_theme().text,
                            vfs);

        i16 uty = static_cast<i16>(gcy + outer + gap);
        i16 utw = draw::text_width("km/h", 1);
        jemgui::rect ur = {{static_cast<i16>(gcx - utw / 2), uty},
                           {static_cast<u16>(utw), static_cast<u16>(unit_h)}};
        draw::text_centered(ui.painter(), ur, "km/h",
                            ui.current_theme().text_dim, 1);
      }

      ui.spacer(4);

      {
        i16 cw = 94;
        i16 ch = 36;
        char dbuf[16];
        char tbuf[16];
        snprintf(dbuf, sizeof(dbuf), "%d.%d", distance / 10, distance % 10);
        snprintf(tbuf, sizeof(tbuf), "%dm", ride_time);

        ui.row(ch);
        ui.col(cw);
        ui.stat_card("SPEED", "35", ui.current_theme().accent);
        ui.end();
        ui.col(cw);
        ui.stat_card("DIST", dbuf, ui.current_theme().success);
        ui.end();
        ui.col(cw);
        ui.stat_card("TIME", tbuf, ui.current_theme().warning);
        ui.end();
        ui.end();
      }

      ui.spacer(4);

      ui.progress("bat", static_cast<float>(battery) / 100.0f);

      ui.spacer(4);

      ui.slider("spd", speed, 0, 60);
    }

    else if (page == 2) {
      u16 aw = ui.available().w;
      i16 sv = ui.scale_value();
      i16 sp = jemgui::scale(ui.current_theme().spacing, sv);
      u16 tw = static_cast<u16>((aw - sp) / 2);
      i16 th = jemgui::scale(78, sv);

      ui.row(78);
      if (ui.tile("COPY", jemgui::rgb565(120, 80, 200), tw, th)) led_g = !led_g;
      if (ui.tile("SCAN", jemgui::rgb565(60, 140, 220), tw, th)) led_b = !led_b;
      ui.end();

      ui.row(78);
      if (ui.tile("PRINT", jemgui::rgb565(60, 180, 100), tw, th))
        led_r = !led_r;
      if (ui.tile("SETUP", jemgui::rgb565(220, 100, 60), tw, th)) page = 3;
      ui.end();

      ui.spacer(8);
      ui.label("tap tiles to toggle LEDs");
    }

    else if (page == 3) {
      ui.header("PRINT SETTINGS", jemgui::rgb565(60, 100, 200));
      ui.spacer(4);

      ui.label("select file");
      static const char* files[] = {"report.pdf", "photo.jpg", "design.png",
                                    "notes.txt"};
      for (i16 i = 0; i < 4; ++i) {
        if (ui.list_item(files[i], selected_file == i)) {
          selected_file = i;
        }
      }

      ui.separator();

      ui.spinner("copies", copies, 1, 99);
      ui.toggle("duplex", duplex);
      ui.toggle("high quality", high_quality);

      ui.spacer(4);

      ui.button_fill_colored("START PRINT", ui.current_theme().success);
    }

    ui.panel_end();
    ui.pop_id();

    ui.end_frame();
    fb.flush();

    led_g ? led_green.high() : led_green.low();
    led_b ? led_blue.high() : led_blue.low();
    led_r ? led_red.high() : led_red.low();

    ++frame;
  }
}
