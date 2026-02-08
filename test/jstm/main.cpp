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

  bool green_on = false;
  bool blue_on = false;
  bool red_on = false;
  bool blink_enable = false;
  i16 blink_rate = 5;
  i16 brightness_pct = 80;

  u32 frame = 0;
  u32 blink_counter = 0;
  bool blink_phase = false;

  while (true) {
    input_state input;
    if (touch.touched()) {
      auto tp = touch.read();
      if (tp.x != drivers::xpt2046::INVALID_COORD) {
        input.touch_pos = {tp.x, tp.y};
        input.touch_down = true;
      }
    }

    ui.begin_frame(input);

    ui.panel_begin("led controller");

    ui.row();
    ui.toggle("green", green_on);
    ui.toggle("blue", blue_on);
    ui.toggle("red", red_on);
    ui.end();

    ui.toggle("blink all", blink_enable);
    ui.slider("blink rate", blink_rate, 1, 20);
    ui.slider("brightness", brightness_pct, 0, 100);

    ui.separator();

    ui.row();
    if (ui.button("all on")) {
      green_on = true;
      blue_on = true;
      red_on = true;
    }
    if (ui.button("all off")) {
      green_on = false;
      blue_on = false;
      red_on = false;
    }
    if (ui.button("rotate")) {
      rotation = static_cast<u8>((rotation + 1) & 3);
      display.set_rotation(rotation);
      touch.set_screen_size(display.width(), display.height());
      touch.set_rotation(rotation);
      fb.reinit();
      ui.recalculate();
    }
    ui.end();

    ui.separator();

    ui.label_fmt("frame: %lu  rotation: %u", frame, rotation);

    ui.row();
    ui.progress("brightness", static_cast<float>(brightness_pct) / 100.0f);
    ui.end();

    if (input.touch_down) {
      ui.label_fmt("touch: %d, %d", input.touch_pos.x, input.touch_pos.y);
    } else {
      ui.label("touch: --");
    }

    ui.panel_end();

    ui.end_frame();
    fb.flush();

    if (blink_enable) {
      blink_counter++;
      u32 half_period = static_cast<u32>(60 / blink_rate) / 2;
      if (half_period < 1) half_period = 1;
      if (blink_counter >= half_period) {
        blink_counter = 0;
        blink_phase = !blink_phase;
      }
    } else {
      blink_phase = true;
      blink_counter = 0;
    }

    bool phase = blink_phase;
    (green_on && phase) ? led_green.high() : led_green.low();
    (blue_on && phase) ? led_blue.high() : led_blue.low();
    (red_on && phase) ? led_red.high() : led_red.low();

    frame++;
  }
}
