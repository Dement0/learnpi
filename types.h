#ifndef TYPES_H
#define TYPES_H

enum built_in_function_types {
  BUILT_IN_PRINT = 1,
  BUILT_IN_SQUARE_ROOT,
  BUILT_IN_LED_ON,
  BUILT_IN_LED_OFF,
  BUILT_IN_IS_BUTTON_PRESSED,
  BUILT_IN_GET_PRESSED_KEY,
  BUILT_IN_BUZZ_START,
  BUILT_IN_BUZZ_STOP,
  BUILT_IN_MOVE_SERVO_TO_ANGLE,
  BUILT_IN_MOVE_SERVO_INFINITELY,
  BUILT_IN_SERVO_STOP,
  BUILT_IN_DELAY
};

// Primitive and composed types
enum type {
    BIT_TYPE,
    INTEGER_TYPE,
    DECIMAL_TYPE,
    STRING_TYPE,
    LED,
    BUTTON,
    KEYPAD,
    BUZZER,
    SERVO_MOTOR
};

#endif