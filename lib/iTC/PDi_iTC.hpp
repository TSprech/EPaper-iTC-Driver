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


  static constexpr uint16_t width_ = 296;
  static constexpr uint16_t height_ = 152;

  static constexpr uint16_t pixel_buffer_size_ = 5624;

  std::array<uint8_t, pixel_buffer_size_>(black_pixel_buffer_);
  std::array<uint8_t, pixel_buffer_size_>(color_pixel_buffer_);

  uint8_t chip_select_pin_ = 0;
  uint8_t command_data_pin_ = 0;
  uint8_t reset_pin_ = 0;
  uint8_t busy_pin_ = 0;

  uint16_t XYToIndex(uint16_t x, uint16_t y);
  uint16_t XYToPixel(uint16_t x, uint16_t y);

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