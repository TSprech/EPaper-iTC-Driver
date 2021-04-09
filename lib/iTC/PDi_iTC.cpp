#include "PDi_iTC.hpp"

void ITC::Initialize() {
  // Set pin directions
  pinMode(chip_select_pin_, OUTPUT);
  pinMode(command_data_pin_, OUTPUT);
  pinMode(reset_pin_, OUTPUT);

  // Start the SPI bus
  SPI.begin();
  SPI.beginTransaction(SPISettings(800000, MSBFIRST, SPI_MODE0)); // Speed this up once tested, max is 10MHz

  // The reset pin must be toggled with a specific timing, as detailed in the datasheet
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
  if ((x >= width_) || (y >= height_)) { // Only checking for top limit as the parameters are unsigned to guard from negative values
    return; // If it is out of bounds just return
  }

  uint16_t index = XYToIndex(x, y); // Figure out what array index the pixel being drawn to is contained within

  black_pixel_buffer_.at(index) &= ~(0b1 << (x % 8)); // Mask off the bit and change it to 0
  // First expression isolates the black pixel data from the color, then the second part shifts it over into the correct position
  black_pixel_buffer_.at(index) |= ((value(color) & 0b1) << (x % 8));

  color_pixel_buffer_.at(index) &= ~(0b1 << (x % 8)); // Mask off the bit and change it to 0
  // First expression isolates the color pixel data and shifts it into bit position 0, then the second part shifts it over into the correct position
  color_pixel_buffer_.at(index) |= (((value(color) & 0b1) >> 1) << (x % 8));
}

void ITC::DisplayBuffer() {
  Write(Registers::black_frame);  // Send the register to prepare for the black pixel data
  for (uint16_t current_data_byte = 0; current_data_byte < black_pixel_buffer_.size(); ++current_data_byte) {
    Write(black_pixel_buffer_.at(current_data_byte)); // Go through the entire buffer and write it byte by byte
  }

  Write(Registers::color_frame);  // Send the register to prepare for the color pixel data
  for (uint16_t current_data_byte = 0; current_data_byte < color_pixel_buffer_.size(); ++current_data_byte) {
    Write(color_pixel_buffer_.at(current_data_byte)); // Go through the entire buffer and write it byte by byte
  }

  delay(50); // TODO: Figure out how short this delay needs to be

  Write(Registers::power_on);  // Turn the internal DC to DC converter on
  delay(5);
  while (BusyPin() != 1); // Wait until the display signals it is ready for the next instructions
  Write(Registers::display_refresh);  // Issue the command to move the epaper particles to refresh the screen
  delay(5);
  while (BusyPin() != 1); // Wait until the display signals it is ready for the next instructions
  Write(Registers::power_off);  // Turn off the DC to DC converter once the particles are in position
  while (BusyPin() != 1); // Wait until the display signals it is ready for the next instructions

  ResetPin(0); // Set the reset pin low as specified
}

void ITC::FillBuffer(Colors color) {
  // The enum values for each color are chosen such that if bit 0 is high, then the color is black,
  // if bit 1 is high then it is a color pixel, and if neither are it is white so that simple bitwise
  // operations are able to retrieve the value without having to check each color possibility
  std::fill(black_pixel_buffer_.begin(), black_pixel_buffer_.end(), value(color) & 0b1);
  std::fill(color_pixel_buffer_.begin(), black_pixel_buffer_.end(), (value(color) & 0b10) >> 1);
}

inline uint16_t ITC::XYToIndex(uint16_t x, uint16_t y) {
  return (x / 8) + (y * (width_ / 8)); // Calculate what byte index in the pixel buffers the selected pixel will reside at
}

inline void ITC::ChipSelectPin(bool state) { digitalWrite(chip_select_pin_, state); } // Short method to set the chip select pin
inline void ITC::CommandDataPin(bool state) { digitalWrite(command_data_pin_, state); } // Short method to set the command data pin
inline void ITC::ResetPin(bool state) { digitalWrite(reset_pin_, state); } // Short method to set the reset pin
inline bool ITC::BusyPin() { return digitalRead(busy_pin_); } // Short method to read the busy pin's state

// SPI handling methods
// Writes a single 8 bit value to the defined register
inline void ITC::Write(uint8_t command_value) {
  ChipSelectPin(0); // CS pin must be toggled between each byte written
  SPI.transfer(command_value); // Write the byte to the bus
  ChipSelectPin(1);
}

inline void ITC::Write(Registers command_register) {
  CommandDataPin(0); // As it is a command register being written, toggle the command pin
  ChipSelectPin(0); // CS pin must be toggled between each byte written
  SPI.transfer(value(command_register)); // Write the byte to the bus
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