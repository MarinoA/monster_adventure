/*
 * project_settings.h
 *
 *  Created on: March 12, 2019
 *      Author: Alex Marino
 */

#ifndef PROJECT_SETTINGS_H
#define PROJECT_SETTINGS_H

#define FCPU 24000000

// include the library header
#include "library.h"
// let the system know which lower level modules are in use
// this way higher modules can selectively utilize their resources
#define USE_MODULE_TASK
#define USE_MODULE_SUBSYSTEM
#define USE_MODULE_BUFFER_PRINTF

#define SUBSYSTEM_UART UART0

#define UART0_TX_BUFFER_LENGTH 3500

#define USE_UART0

#define TASK_MAX_LENGTH 100

#endif /* PROJECT_SETTINGS_H */
