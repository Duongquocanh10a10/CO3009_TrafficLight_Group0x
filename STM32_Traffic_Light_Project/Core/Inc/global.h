#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include "main.h"
#include <stdint.h>

/* =========================================================================
 * HARDWARE PIN ALIASES (Fallback if User Labels are not yet set in .ioc)
 * Mapping based on XNUCLEO-F103RB Schematic (MB1136):
 * D3=PB3, D4=PB5, D5=PB4, D6=PB10, D7=PA8, D8=PA9, D9=PC7, D10=PB6, USER=PC13
 * ========================================================================= */
#ifndef TL1_A_Pin
#define TL1_A_Pin          GPIO_PIN_3
#define TL1_A_GPIO_Port    GPIOB
#endif

#ifndef TL1_B_Pin
#define TL1_B_Pin          GPIO_PIN_5
#define TL1_B_GPIO_Port    GPIOB
#endif

#ifndef TL2_A_Pin
#define TL2_A_Pin          GPIO_PIN_4
#define TL2_A_GPIO_Port    GPIOB
#endif

#ifndef TL2_B_Pin
#define TL2_B_Pin          GPIO_PIN_10
#define TL2_B_GPIO_Port    GPIOB
#endif

#ifndef BTN_1_Pin
#define BTN_1_Pin          GPIO_PIN_8
#define BTN_1_GPIO_Port    GPIOA
#endif

#ifndef BTN_2_Pin
#define BTN_2_Pin          GPIO_PIN_9
#define BTN_2_GPIO_Port    GPIOA
#endif

#ifndef BTN_3_Pin
#define BTN_3_Pin          GPIO_PIN_7
#define BTN_3_GPIO_Port    GPIOC
#endif

#ifndef BTN_4_Pin
#define BTN_4_Pin          GPIO_PIN_6
#define BTN_4_GPIO_Port    GPIOB
#endif

#ifndef BTN_USER_Pin
#define BTN_USER_Pin       GPIO_PIN_13
#define BTN_USER_GPIO_Port GPIOC
#endif

/* =========================================================================
 * SYSTEM FSM STATES
 * ========================================================================= */
typedef enum {
    INIT_SYSTEM = 0,

    /* 1. Automatic Mode States */
    AUTO_RED1_GREEN2,
    AUTO_RED1_AMBER2,
    AUTO_GREEN1_RED2,
    AUTO_AMBER1_RED2,

    /* 2. Manual Mode States (with 3-second Amber safety transition) */
    MAN_RED1_GREEN2,
    MAN_RED1_AMBER2,
    MAN_GREEN1_RED2,
    MAN_AMBER1_RED2,

    /* 3. Tuning Mode States (Edit Green & Amber, Auto-compute Red) */
    TUNING_GREEN,
    TUNING_AMBER
} SystemState_t;

/* =========================================================================
 * TRAFFIC LIGHT COLOR & LANE DEFINITIONS
 * ========================================================================= */
typedef enum {
    LIGHT_OFF = 0,
    LIGHT_RED,
    LIGHT_AMBER,
    LIGHT_GREEN
} TrafficColor_t;

#define LANE_1                 1
#define LANE_2                 2

/* =========================================================================
 * BUTTON INDICES
 * ========================================================================= */
#define NUM_BUTTONS            5
#define BTN_MODE_IDX           0   /* D7  - PA8  */
#define BTN_ACTION_INC_IDX     1   /* D8  - PA9  */
#define BTN_DEC_IDX            2   /* D9  - PC7  */
#define BTN_CONFIRM_IDX        3   /* D10 - PB6  */
#define BTN_USER_IDX           4   /* B1  - PC13 */

/* =========================================================================
 * SOFTWARE TIMER INDICES (Tick = 10ms)
 * ========================================================================= */
#define TIMER_FSM_PHASE        0   /* Traffic phase transition timer       */
#define TIMER_ONE_SEC          1   /* 1-second countdown tick              */
#define TIMER_BLINK            2   /* 500ms LED blinking in Tuning mode    */

/* =========================================================================
 * SHARED GLOBAL VARIABLES (Defined in global.c)
 * ========================================================================= */
extern SystemState_t status;

extern int time_red;               /* Always = time_green + time_amber (s) */
extern int time_green;             /* Default: 5s (Range: 1..90s)          */
extern int time_amber;             /* Default: 2s (Range: 1..9s)           */

extern int temp_green;             /* Temporary buffer in Tuning mode      */
extern int temp_amber;             /* Temporary buffer in Tuning mode      */

extern int count_lane1;            /* Remaining seconds for Lane 1         */
extern int count_lane2;            /* Remaining seconds for Lane 2         */

extern float env_temperature;      /* Latest temperature from DHT20 (C)    */
extern float env_humidity;         /* Latest humidity from DHT20 (%RH)     */

extern uint8_t lcd_page_mode;      /* 0: Show DHT20 on row 2, 1: Alt view  */

void global_init(void);
void fsm_system_run(void);
void oneshot_PopupClear(void);

#endif /* INC_GLOBAL_H_ */
