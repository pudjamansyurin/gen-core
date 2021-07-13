/*
 * request.h
 *
 *  Created on: Jun 14, 2021
 *      Author: Pudja Mansyurin
 */

#ifndef INC_APP__REQUEST_H_
#define INC_APP__REQUEST_H_

/* Includes
 * --------------------------------------------*/
#include "App/command.h"

/* Public functions prototype
 * --------------------------------------------*/
void REQ_Execute(const command_t *cmd, response_t *resp);

#endif /* INC_APP__REQUEST_H_ */
