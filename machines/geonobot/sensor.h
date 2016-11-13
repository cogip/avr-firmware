#ifndef SENSOR_H_
#define SENSOR_H_

uint8_t detect_right_cup(void);
uint8_t detect_left_cup(void);

uint8_t detect_elevator_up(void);
uint8_t detect_elevator_down(void);
uint8_t detect_spot(void);

uint8_t detect_color(void);
uint8_t detect_start(void);

#endif /* SENSOR_H_ */
