#include "ble.h"

#define MESSAGE_SUCCES                      0
#define MESSAGE_ERROR_INIT                  1
#define MESSAGE_ERROR_NUMBER_WORD           2
#define MESSAGE_ERROR_CLOSE_WORD            3
#define MESSAGE_ERROR_OPEN_WORD             4
#define MESSAGE_ERROR_LEN_WORD              5
#define MESSAGE_ERROR_NO_CMD                6

void communication_error_display(uint8_t err_code);

