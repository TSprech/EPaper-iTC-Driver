#include "PDi_iTC.hpp"

void ITC::Initialize() {
  pinMode(chip_select_pin_, OUTPUT);
  pinMode(command_data_pin_, OUTPUT);
  pinMode(reset_pin_, OUTPUT);

  // Start the SPI bus
  SPI.begin();
  SPI.beginTransaction(SPISettings(800000, MSBFIRST, SPI_MODE0));

  // The reset pin must be toggled with a specific timing
  delay(5);
  ResetPin(1);
  delay(5);
  ResetPin(0);
  delay(10);
  ResetPin(1);
  delay(5);

  Write(Registers::soft_reset, soft_reset_data_);  // Then the soft reset is issued
  delay(5);
}

void ITC::Temperature(uint8_t temperature) {
  Write(Registers::input_temperature, temperature);                                           // First the current display temperature is written
  Write(Registers::activate_temperature, activate_temperature_data_);                         // Send the command to activate the new temperature
  Write(Registers::panel_settings, panel_settings_first_byte_, panel_settings_second_byte_);  // Send arbitrary panel settings from datasheet
}

void ITC::Pixel(uint16_t x, uint16_t y, Colors color) {
}

void ITC::DisplayBuffer() {
  Write(Registers::black_frame);  // Send the register to prepare for the black pixel data
  for (uint16_t current_data_byte = 0; current_data_byte < black_pixel_buffer_.size(); ++current_data_byte) {
    Write(black_pixel_buffer_.at(current_data_byte));
  }

  Write(Registers::color_frame);  // Send the register to prepare for the color pixel data
  for (uint16_t current_data_byte = 0; current_data_byte < color_pixel_buffer_.size(); ++current_data_byte) {
    Write(color_pixel_buffer_.at(current_data_byte));
  }

  delay(50);

  Write(Registers::power_on);  // Turn the internal DC to DC converter on
  delay(5);
  while (BusyPin() != 1)
    ;
  Write(Registers::display_refresh);  // Issue the command to move the epaper particles
  delay(5);
  while (BusyPin() != 1)
    ;
  Write(Registers::power_off);  // Turn off the DC to DC converter once the particles are in position
  while (BusyPin() != 1)
    ;

  ResetPin(0);
}

void ITC::FillBuffer(Colors color) {
  // The enum values for each color are chosen such that if bit 0 is high, then the color is black,
  // if bit 1 is high then it is a color pixel, and if neither are it is white so that simple bitwise
  // operations are able to retrieve the value without having to check each color possibility
  std::fill(black_pixel_buffer_.begin(), black_pixel_buffer_.end(), value(color) & 0b1);
  std::fill(color_pixel_buffer_.begin(), black_pixel_buffer_.end(), (value(color) & 0b10) >> 1);
}

inline uint16_t ITC::XYToIndex(uint16_t x, uint16_t y) {
  return ((x / 2) + (y * 64));
}

inline uint16_t ITC::XYToPixel(uint16_t x, uint16_t y) {
  return (x & 0b1) ? (((x / 2) + 1) + (y * 64)) : ((x / 2) + (y * 64));
}

inline void ITC::ChipSelectPin(bool state) { digitalWrite(chip_select_pin_, state); }
inline void ITC::CommandDataPin(bool state) { digitalWrite(command_data_pin_, state); }
inline void ITC::ResetPin(bool state) { digitalWrite(reset_pin_, state); }
inline bool ITC::BusyPin() { return digitalRead(busy_pin_); }

// SPI handling methods
// Writes a single 8 bit value to the defined register
inline void ITC::Write(uint8_t command_value) {
  ChipSelectPin(0);
  SPI.transfer(command_value);
  ChipSelectPin(1);
}

inline void ITC::Write(Registers command_register) {
  CommandDataPin(0);
  ChipSelectPin(0);
  SPI.transfer(static_cast<uint8_t>(command_register));
  ChipSelectPin(1);
  CommandDataPin(1);
}

inline void ITC::Write(Registers command_register, uint8_t command_value) {
  Write(command_register);
  Write(command_value);
}

// Writes a 16 bit value as 2 8 bit values to the defined register
inline void ITC::Write(Registers command_register, uint8_t command_value_1, uint8_t command_value_2) {
  Write(command_register);
  Write(command_value_1);
  Write(command_value_2);
}