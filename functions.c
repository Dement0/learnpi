#define _GNU_SOURCE
#include "learnpi.h"
#include "functions.h"
#include <pigpio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void yyerror(char *s, ...) {
  printf("yyerror\n");
  va_list ap;
  va_start(ap, s);

  fprintf(stderr, "%d: error: ", yylineno);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

int get_value_type(struct val *value) {
	if(value) {
  	    return value->type;
	} else {
		return -1;
	}
}

struct val *create_LED(struct val ** pin) {
    struct val *result;
    result = create_complex_value(pin, 1, LED);

    // Set the current mode to output
    #ifdef RPI_SIMULATION
        int currentMode = gpioSetMode(result->datavalue.GPIO_PIN[0], 1);
    #else
        printf("Simulated gpioSetMode function after LED creation.\n");
        int currentMode = 0;
    #endif
    
    return result;
}

struct val *create_BUTTON(struct val ** pin) {
    struct val *result;
    result = create_complex_value(pin, 1, BUTTON);

    // Set the current mode to output and PULL_UP to HIGH
    #ifdef RPI_SIMULATION
        int currentMode = gpioSetMode(result->datavalue.GPIO_PIN[0], 0);
        int pullUpLevel = gpioSetPullUpDown(result->datavalue.GPIO_PIN[0], PI_PUD_UP)
    #else
        printf("Simulated gpioSetMode and gpioSetPullUpDown functions after BUTTON creation.\n");
        int currentMode = 0;
        int pullUpLevel = 2;
    #endif
    
    return result;
}

struct val *create_KEYPAD(struct val ** pin) {
    struct val *result;
    result = create_complex_value(pin, 8, KEYPAD);
    
    int currentMode = -1;
    int pullUpLevel = -1;
    int writeResult = -1;

    // Set the first 4 pins to input mode and PULL_UP to HIGH
    #ifdef RPI_SIMULATION
        for(int i = 0; i < 4; i++) {
            currentMode = gpioSetMode(result->datavalue.GPIO_PIN[0], 0);
            pullUpDnControl(result->datavalue.GPIO_PIN[i], PI_PUD_UP);
        }

        // Set the last 4 pins to output mode and write 1
        for (int i = 4; i < 8; i++) {
            currentMode = gpioSetMode(result->datavalue.GPIO_PIN[0], 1);
            gpioWrite(result->datavalue.GPIO_PIN[0], 1);
        }
    #else
        printf("Simulated gpioSetMode and gpioSetPullUpDown functions after KEYPAD creation.\n");
        currentMode = 1;
        pullUpLevel = 1;
        writeResult = 0;
    #endif

    // Check errors upon writing pins
    if(writeResult != 0) {
        printf("PI_BAD_GPIO or PI_BAD_LEVEL!\n");
        return NULL;
    }

    return result;
}

struct val *create_COMPLEXTYPE(struct val ** pin, int pin_no, int datatype) {
    // Allocate memory 
    struct val *result = malloc(sizeof(struct val));

    // Local helper
    int i = 0;
    unsigned int current;

    // Assign the argument datatype to result type    
    result->type = datatype;
    result->datavalue.GPIO_PIN = malloc(pin_no * sizeof(int));

    while(i < pin_no) {
        current = (unsigned int)pin[i]->datavalue.GPIO_PIN;
        if(current < 0 || current > 50) {
            yyerror("invalid value %d for pin declaration", pin[i]->datavalue.GPIO_PIN);
            free(result);
            break;
        }

        result->datavalue.GPIO_PIN[i] = current;
        i =+ 1;
    }
    return result;
}

struct val *sum(struct val *first, struct val *second) {
    struct val *result = malloc(sizeof(struct val));

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = INTEGER_TYPE;
                result->datavalue.integer = first->datavalue.integer + second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.integer + second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal + second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal + second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                result->type = STRING_TYPE;
                asprintf(&result->datavalue.string, "%s%s", first->datavalue.string, second->datavalue.string);
            }
            break;
        default:
            yyerror("Sum error between types.");
            free(result);
            return NULL;
    }

    return result;
}

struct val *subtract(struct val *first, struct val *second) {
    struct val *result = malloc(sizeof(struct val));

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = INTEGER_TYPE;
                result->datavalue.integer = first->datavalue.integer - second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.integer - second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal - second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal - second->datavalue.decimal;
            }
            break;
        default:
            yyerror("Subtraction error between types.");
            free(result);
            return NULL;
    }

    return result;
}

struct val *multiply(struct val *first, struct val *second) {
    struct val *result = malloc(sizeof(struct val));

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = INTEGER_TYPE;
                result->datavalue.integer = first->datavalue.integer * second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.integer * second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal * second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal * second->datavalue.decimal;
            }
            break;
        default:
            yyerror("Multiplication error between types.");
            free(result);
            return NULL;
    }

    return result;
}

struct val *divide(struct val *first, struct val *second) {
    struct val *result = malloc(sizeof(struct val));

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE && second->datavalue.integer != 0) {
                if(first->datavalue.integer % second->datavalue.integer == 0) {
                    result->type = INTEGER_TYPE;
                    result->datavalue.integer = first->datavalue.integer / second->datavalue.integer;
                } else {
                    result->type = DECIMAL_TYPE;
                    result->datavalue.decimal = (double)first->datavalue.integer / (double)second->datavalue.integer;
                }
            } else if(get_value_type(second) == DECIMAL_TYPE  && second->datavalue.decimal != 0) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = (double)first->datavalue.integer / second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE  && second->datavalue.integer != 0) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal / (double)second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE  && second->datavalue.decimal != 0) {
                result->type = DECIMAL_TYPE;
                result->datavalue.decimal = first->datavalue.decimal / second->datavalue.decimal;
            }
            break;
        default:
            yyerror("Division error between types.");
            free(result);
            return NULL;
    }

    return result;
}

struct val *get_absolute_value(struct val *value) {
    struct val *result = malloc(sizeof(struct val));

    switch (get_value_type(value)) {
        case INTEGER_TYPE:
            if(value->datavalue.integer < 0) {
                result->datavalue.integer = -(value->datavalue.integer);
            } else {
                result->datavalue.integer = value->datavalue.integer;
            }
            break;
        case DECIMAL_TYPE:
            if(value->datavalue.integer < 0) {
                result->datavalue.decimal = -(value->datavalue.decimal);
            } else {
                result->datavalue.decimal = value->datavalue.decimal;
            }
            break;
        default:
            yyerror("Absolute value error");
            free(result);
    }

    result->type = get_value_type(value);
    return result;
}

struct val *change_sign(struct val *value) {
    struct val *result = malloc(sizeof(struct val));
    
    switch (get_value_type(value)) {
        case INTEGER_TYPE:
            result->datavalue.integer = -(value->datavalue.integer);
            break;
        case DECIMAL_TYPE:
            result->datavalue.decimal = -(value->datavalue.decimal);
            break;
        default:
            yyerror("Sign change error");
            free(result);
    }

    result->type = get_value_type(value);
    return result;
}

struct val *calculate_logical_and(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));

    if(get_value_type(first) == BIT_TYPE && get_value_type(second) == BIT_TYPE) {
        result->type = BIT_TYPE;
        result->datavalue.bit = first->datavalue.bit && second->datavalue.bit;
    } else {
        yyerror("Logical AND error");
        free(result);
    }

    return result;  
}

struct val *calculate_logical_or(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));

    if(get_value_type(first) == BIT_TYPE && get_value_type(second) == BIT_TYPE) {
        result->type = BIT_TYPE;
        result->datavalue.bit = first->datavalue.bit || second->datavalue.bit;
    } else {
        yyerror("Logical OR error");
        free(result);
    }

    return result; 
}

struct val *calculate_greater_than(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer > second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer > second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal > second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal > second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper == 1) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if greater than.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *calculate_less_than(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer < second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer < second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal < second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal < second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper == -1) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if less than.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *calculate_equals(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer == second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer == second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal == second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal == second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper == 0) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if equals.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *calculate_not_equals(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer != second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer != second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal != second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal != second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper != 0) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if not equals.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *calculate_greater_equal_than(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer >= second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer >= second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal >= second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal >= second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper == 0 || helper == 1) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if greater equal than.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *calculate_less_equal_than(struct val *first, struct val *second) {
    struct val * result = malloc(sizeof(struct val));
    int helper;

    switch(get_value_type(first)) {
        case INTEGER_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.bit = first->datavalue.integer <= second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.decimal = first->datavalue.integer <= second->datavalue.decimal;
            }
            break;
        case DECIMAL_TYPE:
            if(get_value_type(second) == INTEGER_TYPE) {
                result->datavalue.decimal = first->datavalue.decimal <= second->datavalue.integer;
            } else if(get_value_type(second) == DECIMAL_TYPE) {
                result->datavalue.bit = first->datavalue.decimal <= second->datavalue.decimal;
            }
            break;
        case STRING_TYPE:
            if(get_value_type(second) == STRING_TYPE) {
                // String comparision
                helper = strcmp(first->datavalue.string, second->datavalue.string);

                // Check if strings are equal
                if(helper == 0 || helper == -1) {
                    result->datavalue.bit = 1;
                } else {
                    result->datavalue.bit = 0;
                }
            }
            break;
        default:
            yyerror("Cannot calculate if less equal than.");
            free(result);
    }

    result->type = BIT_TYPE;
    return result;
}

struct val *create_bit_value(int bit_value) {
    if(bit_value != 0 && bit_value != 1) {
        return NULL;
    }

    struct val *bit_val = malloc(sizeof(struct val));
    bit_val->type = BIT_TYPE;
    bit_val->datavalue.bit = bit_value;
    return bit_val;
}

struct val *create_integer_value(int integer_value) {
    struct val *integer_val = malloc(sizeof(struct val));
    integer_val->type = INTEGER_TYPE;
    integer_val->datavalue.integer = integer_value;
    return integer_val;
}

struct val *create_decimal_value(double decimal_value) {
    struct val *decimal_val = malloc(sizeof(struct val));
    decimal_val->type = DECIMAL_TYPE;
    decimal_val->datavalue.decimal = decimal_value;
    return decimal_val;
}

struct val *create_string_value(char *string_value) {
    struct val *string_val = malloc(sizeof(struct val));
	string_val->type = STRING_TYPE;
    string_val->datavalue.string = strdup(string_value);
    return string_val;
}

/*
 * Function to create or declare a LED
 * if it's a declaration, assigns 0 to GPIO_PIN
 * else evaluates the assignment
 */
struct val *create_led_value(struct val ** pin, int is_declaration) {
    struct val *led_val = malloc(sizeof(struct val));

    if(is_declaration == 1) {
        led_val->type = LED;
        led_val->datavalue.GPIO_PIN = 0;
    } else {
        led_val = create_complex_value(pin, 1, LED);
    }
    
    return led_val;
}

/*
 * Function to create or declare a BUTTON
 * if it's a declaration, assigns 0 to GPIO_PIN
 * else evaluates the assignment
 */
struct val *create_button_value(struct val ** pin, int is_declaration) {
    struct val * result = malloc(sizeof(struct val));

    if(is_declaration == 1) {
        result->type = BUTTON;
        result->datavalue.GPIO_PIN = 0;
    } else {
        result = create_complex_value(pin, 1, BUTTON);
    }
    
    return result;
}

/*
 * Function to create or declare a KEYPAD
 * if it's a declaration, assigns 0 to GPIO_PIN
 * else evaluates the assignment
 */
struct val *create_keypad_value(struct val ** pin, int is_declaration) {
    struct val * result = malloc(sizeof(struct val));

    if(is_declaration == 1) {
        result->type = KEYPAD;
        result->datavalue.GPIO_PIN = 0;
    } else {
        result = create_complex_value(pin, 8, KEYPAD);
    }
    
    return result;
}

struct val *create_complex_value(struct val ** pin, int number_of_pins, int datatype) {
    struct val *result = malloc(sizeof(struct val));
    result->type = datatype;

    printf("Number of pins: %d\n", number_of_pins);

    if(number_of_pins > 0 && pin) {
        result->datavalue.GPIO_PIN = malloc(number_of_pins * sizeof(int));
        printf("Datavalue.GPIO_PIN is: %d\n", *result->datavalue.GPIO_PIN);
        int current_pin;

        for(int i = 0; i < number_of_pins; i++) {
            //printf("Pin value is: %d\n", pin[i]->datavalue.GPIO_PIN);
            if(pin[i] == NULL) {
                yyerror("No pin found while creating complex value.\n");
                free(result);
                return NULL;
            }

            current_pin = pin[i]->datavalue.GPIO_PIN;

            if(current_pin < 0 || current_pin > 50) {
                yyerror("%d is not a valid pin number.", current_pin);
                free(result);
                break;
            }
            result->datavalue.GPIO_PIN[i] = (unsigned int)current_pin;

            printf("Current pin value is: %d\n", current_pin);
        }
    }

    printf("Complex value creation completed.\n");
    return result;
}

/*
* Sets the GPIO level to on.
* gpio: 0-53
* level: 0-1
* Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_LEVEL.
*/
int led_on(struct val * value) {
    printf("Trying to set the led on...\n");
    return gpioWrite(value->datavalue.GPIO_PIN[0], 1);
}

/*
* Sets the GPIO level to off.
* gpio: 0-53
* level: 0-1
* Returns 0 if OK, otherwise PI_BAD_GPIO or PI_BAD_LEVEL.
*/
int led_off(struct val * value) {
    return gpioWrite(value->datavalue.GPIO_PIN[0], 0);
}

/*
 * Reads the GPIO level of the button.
 * gpio: 0-53
 * Returns the GPIO level if OK, otherwise PI_BAD_GPIO.
 */
struct val *is_button_pressed(struct val * value) {
    struct val *result = malloc(sizeof(struct val));
    result->type = BIT_TYPE;
    result->datavalue.bit = 0;

    // Check if pin number is correct
    if(gpioRead(value->datavalue.GPIO_PIN[0]) == PI_BAD_GPIO) {
        return NULL;
    }

    if(gpioRead(value->datavalue.GPIO_PIN[0]) == 0) {
        result->datavalue.bit = 1;
    }
    
    return result;
}
