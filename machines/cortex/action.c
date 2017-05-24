#include "action.h"

#include "gpio.h"
#include "kos.h"
#include "platform.h"
#include "sd21.h"

#define SERVO_ID_VENT_FL	0
#define SERVO_ID_VENT_FR	3
#define SERVO_ID_VENT_RL	1
#define SERVO_ID_VENT_RR	2
#define SERVO_ID_WEDGE_L	6
#define SERVO_ID_WEDGE_R	7


/*
 * public
 */
void act_catch_module_front_right(void)
{
	uint8_t pause_ms = 250 / 20;

	/* ventouse front right open */
	sd21_control_servo(&sd21, SERVO_ID_VENT_FR, SD21_SERVO_OPEN);

	/* pump front right on */
	gpio_set_output(&PORTB, GPIO_ID_PUMP_FR, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse front right close */
	sd21_control_servo(&sd21, SERVO_ID_VENT_FR, SD21_SERVO_CLOSE);
}

void act_catch_module_rear_right(void)
{
	uint8_t pause_ms = 250 / 20;

	/* ventouse rear right open */
	sd21_control_servo(&sd21, SERVO_ID_VENT_RR, SD21_SERVO_OPEN);

	/* pump rear right on */
	gpio_set_output(&PORTB, GPIO_ID_PUMP_RR, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse rear right close */
	sd21_control_servo(&sd21, SERVO_ID_VENT_RR, SD21_SERVO_CLOSE);
}

void act_catch_module_front_left(void)
{
	uint8_t pause_ms = 250 / 20;

	/* ventouse front left open */
	sd21_control_servo(&sd21, SERVO_ID_VENT_FL, SD21_SERVO_OPEN);

	/* pump front left on */
	gpio_set_output(&PORTB, GPIO_ID_PUMP_FL, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse front left close */
	sd21_control_servo(&sd21, SERVO_ID_VENT_FL, SD21_SERVO_CLOSE);
}

void act_catch_module_rear_left(void)
{
	uint8_t pause_ms = 250 / 20;

	/* ventouse rear left open */
	sd21_control_servo(&sd21, SERVO_ID_VENT_RL, SD21_SERVO_OPEN);

	/* pump rear left on */
	gpio_set_output(&PORTB, GPIO_ID_PUMP_RL, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse rear left close */
	sd21_control_servo(&sd21, SERVO_ID_VENT_RL, SD21_SERVO_CLOSE);
}
