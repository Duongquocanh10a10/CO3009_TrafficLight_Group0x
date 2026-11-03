#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

#define NUM_BUTTONS                 5

#define BTN_1_INDEX                 0
#define BTN_2_INDEX                 1
#define BTN_3_INDEX                 2
#define BTN_4_INDEX                 3
#define BTN_USER_INDEX              4

#define BTN_MODE                    BTN_1_INDEX
#define BTN_UP                      BTN_2_INDEX
#define BTN_SWITCH                  BTN_2_INDEX
#define BTN_DOWN                    BTN_3_INDEX
#define BTN_CONFIRM                 BTN_4_INDEX
#define BTN_USER                    BTN_USER_INDEX

#define NORMAL_STATE                GPIO_PIN_SET
#define PRESSED_STATE               GPIO_PIN_RESET

#define LONG_PRESS_INITIAL_TICKS    100
#define LONG_PRESS_REPEAT_TICKS     20

void button_reading(void);
uint8_t isButtonPressed(uint8_t index);
uint8_t isButtonLongPressed(uint8_t index);
void clearAllButtons(void);

#ifdef __cplusplus
}
#endif

#endif
