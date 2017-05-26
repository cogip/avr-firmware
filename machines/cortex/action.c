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

#define CATCH_MODULE_PAUSE_MS		(1500 / 20 /* ms (sched value) */)

/*
 * public
 */
void act_catch_module_front_right(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FR : SERVO_ID_VENT_FL;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump front right on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse front right close */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);
}

void act_catch_module_rear_right(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_RR : SERVO_ID_VENT_RL;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_RR : GPIO_ID_PUMP_RL;

	/* ventouse rear right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump rear right on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse rear right close */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);
}

void act_catch_module_front_left(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FL : SERVO_ID_VENT_FR;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FL : GPIO_ID_PUMP_FR;

	/* ventouse front left open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump front left on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse front left close */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);
}

void act_catch_module_rear_left(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_RL : SERVO_ID_VENT_RR;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_RL : GPIO_ID_PUMP_RR;

	/* ventouse rear left open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump rear left on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();

	/* ventouse rear left close */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);
}

/*
 * Unload central point sequence
 */
void act_open_front_right_arm(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FR : SERVO_ID_VENT_FL;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump front right on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();
}

void act_open_front_left_arm(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FL : SERVO_ID_VENT_FR;
	/*const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;*/

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump front right on */
	/*gpio_set_output(&PORTB, pump_id, 1);*/

	while (pause_ms--)
		kos_yield();
}

void act_close_front_right_arm(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FR : SERVO_ID_VENT_FL;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);

	/* pump front right off */
	gpio_set_output(&PORTB, pump_id, 0);

	while (pause_ms--)
		kos_yield();
}

void act_close_front_left_arm(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FL : SERVO_ID_VENT_FR;
	/*const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;*/

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);

	/* pump front right on */
	/*gpio_set_output(&PORTB, pump_id, 1);*/

	while (pause_ms--)
		kos_yield();
}

void act_open_rear_right_arm(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS;
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_RR : SERVO_ID_VENT_RL;
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_RR : GPIO_ID_PUMP_RL;

	/* ventouse front right open */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_OPEN);

	/* pump front right on */
	gpio_set_output(&PORTB, pump_id, 1);

	while (pause_ms--)
		kos_yield();
}

void act_stop_front_right_pump(void)
{
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_FR : GPIO_ID_PUMP_FL;
	/* pump front right off */
	gpio_set_output(&PORTB, pump_id, 0);
}

void act_close_FR_arm_open_RR_arm(void)
{
	const uint8_t front_servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_FR : SERVO_ID_VENT_FL;
	const uint8_t rear_servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_RR : SERVO_ID_VENT_RL;

	/* ventouse front right close */
	sd21_control_servo(&sd21, front_servo_id, SD21_SERVO_CLOSE);

	/* ventouse rear right open */
	sd21_control_servo(&sd21, rear_servo_id, SD21_SERVO_OPEN);	
}

void act_stop_rear_right_pump(void)
{
	const uint8_t pump_id = mach_is_camp_yellow() ? GPIO_ID_PUMP_RR : GPIO_ID_PUMP_RL;
	/* pump rear right off */
	gpio_set_output(&PORTB, pump_id, 0);
}

void act_close_rear_right_arm(void)
{
	const uint8_t servo_id = mach_is_camp_yellow() ? SERVO_ID_VENT_RR : SERVO_ID_VENT_RL;
	/* ventouse rear right close */
	sd21_control_servo(&sd21, servo_id, SD21_SERVO_CLOSE);
}

void start_turbine(void)
{
	uint8_t pause_ms = CATCH_MODULE_PAUSE_MS * 2;
	sd21_control_servo(&sd21, 8, SD21_SERVO_OPEN);
	while (pause_ms--)
		kos_yield();
}

void stop_turbine(void)
{
	sd21_control_servo(&sd21, 8, SD21_SERVO_CLOSE);
}
