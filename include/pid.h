/*
 * pid.h
 *
 *  Created on: 24 avr. 2015
 *      Author: ldo
 */

#ifndef PID_H_
#define PID_H_

#define INTEGRAL_LIMIT	500000

/**
 * \struct PID
 */
typedef struct
{
  double kp; /*!< proportional gain */
  double ki; /*!< integral gain */
  double kd; /*!< derivative gain */
  double ti; /*!< error sum */
  double previous_error; /*!< previous sum */
} PID;

/**
 * \fn void pid_setup(PID *pid, const double kp, const double ki, const double kd)
 * \brief PID setup
 * \param pid struct PID
 * \param kp proportional gain
 * \param ki integral gain
 * \param kd derivative gain
 * */
void
pid_setup (PID *pid, const double kp, const double ki, const double kd);

/**
 * \fn void pid_reset(PID *pid)
 * \brief PID reset pid parameters
 * \param pid struct PID
 * */
void
pid_reset (PID *pid);

/**
 * \fn double pid_controller(const double error, PID *pid)
 * \brief compute pid controller
 * \param error
 * \param pid
 * \return the variable that will be adjusted by the pid
 * */
double
pid_controller (const double error, PID *pid);

#endif /* PID_H_ */
