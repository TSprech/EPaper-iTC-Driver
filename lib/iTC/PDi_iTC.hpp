#ifndef PDI_ITC_HPP
#define PDI_ITC_HPP

#include <stdint.h>
#include <algorithm>
#include <array>
#include <type_traits>
// Include seperator
#include <SPI.h>

class ITC {
  //* ════════════════════════════════════════════════════════════════════════════════════════════════════
 public:
  //* ────────────────────────────────────────────────────────────────────────────────────────────────────
  // The values assigned to each of these pixel color values is important, a common expression used thoughout this library
  // is "color & 0b1" and "(color & 0b10 >> 1)". The first using 0b1 is used to see if the pixel should be black, hence why
  // the Colors number for black is 1. The second one using 0b10 checks if the pixel should be a color (red) and then shifts
  // the result right once to standardize its position like the black pixel code. Note that white is 0 as if neither of the
  // bits are set, both operations result in 0 which the display interprets as white. Long story short: don't change these.
  enum class Colors : uint8_t {
    white = 0x0,
    black = 0x1,
    red = 0x2
  };

  enum class Circle_Quadrant : uint8_t {
    q1 = 0x1, /**< Top right corner */
    q2 = 0x2, /**< Bottom right corner */
    q3 = 0x3, /**< Bottom left corner */
    q4 = 0x4  /**< Top left corner */
  };

  ITC(uint8_t chip_select_pin, uint8_t data_command_pin, uint8_t reset_pin, uint8_t busy_pin);

  /**
   * @brief Initializes all command registers to the required values for standard operation. Call before any data is sent to the display.
   */
  void Initialize();

  /**
   * @brief Sets the current operating temperature of the display.
   * @param temperature The temperature of the display, required as the display behaves differently dependant on temperature.
   */
  void Temperature(uint8_t temperature);

  void Pixel(uint16_t x, uint16_t y, Colors color);
  // void Pixel(uint8_t x, uint8_t y);
  void Pixel(uint16_t index, Colors color);

  /**
   * @brief Writes the entire microcontroller display buffer to the display, overriding any existing image data.
   */
  void DisplayBuffer();

  /**
   * @brief Fills the pixel buffer with a value.
   * @param color The value that the be used to fill the pixel buffers.
   */
  void FillBuffer(Colors color);

  //* ════════════════════════════════════════════════════════════════════════════════════════════════════
 private:
  //* ────────────────────────────────────────────────────────────────────────────────────────────────────
  enum class Registers : uint8_t {
    soft_reset = 0x00,
    input_temperature = 0xE5,
    activate_temperature = 0xE0,
    panel_settings = 0x00,
    black_frame = 0x10,
    color_frame = 0x13,
    power_on = 0x04,
    display_refresh = 0x12,
    power_off = 0x02
  };

  uint16_t XYToIndex(uint16_t x, uint16_t y);

  /**
   * @brief Converts an enum class member into its underlying value.
   * @param enum_class_member
   */
  template <typename enum_type>
  constexpr auto value(enum_type enum_class_member) {
    return static_cast<std::underlying_type_t<enum_type>>(enum_class_member);
  }

  // Constant configuration values
  const uint8_t soft_reset_data_ = 0xE;
  const uint8_t activate_temperature_data_ = 0x2;
  const uint8_t panel_settings_first_byte_ = 0xCF;
  // const uint8_t panel_settings_first_byte_ = 0x0F;
  const uint8_t panel_settings_second_byte_ = 0x89;


  static constexpr uint16_t width_ = 296; /**< Number of pixels in the X axis of the display */
  static constexpr uint8_t width_in_bytes_ = 37; // Divide width by 8, might replace with constexpr if possible
  static constexpr uint16_t height_ = 152; /**< Number of pixels in the Y axis of the display */
  static constexpr uint16_t pixel_buffer_size_ = 5624; /**< width_ * height_ / 8 to get each buffer size */

  std::array<uint8_t, pixel_buffer_size_>(black_pixel_buffer_); /**< Used to store pixel buffer 1 (black pixels) */
  std::array<uint8_t, pixel_buffer_size_>(color_pixel_buffer_); /**< Used to store pixel buffer 2 (color pixels) */

  uint8_t chip_select_pin_ = 0;
  uint8_t command_data_pin_ = 0;
  uint8_t reset_pin_ = 0;
  uint8_t busy_pin_ = 0;

  void ResetPin(bool state);
  void ChipSelectPin(bool state);
  void CommandDataPin(bool state);
  bool BusyPin();

  void Write(uint8_t command_value);
  void Write(Registers command_register);
  void Write(Registers command_register, uint8_t command_value);
  void Write(Registers command_register, uint8_t command_value_1, uint8_t command_value_2);

  void WriteData(uint8_t data_value);
};

#endif  // PDI_ITC_HPP