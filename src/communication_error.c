#include "communication_error.h"
#include "communication.h"
#include "nrf_log.h"

void communication_error_display(uint8_t err_code)
{
    switch (err_code)
    {
        case MESSAGE_ERROR_INIT :
            NRF_LOG_PRINTF("Une commande doit commencer avec le caractere %c et finir avec %c\n", MESSAGE_TYPE_SOF, MESSAGE_TYPE_EOF);
            break;
        
        case MESSAGE_ERROR_NUMBER_WORD :
            NRF_LOG_PRINTF("Depassement du nombre de mot maximum de %d\n", MAX_WORD);
            break;
        
        case MESSAGE_ERROR_CLOSE_WORD :
            NRF_LOG_PRINTF("La fin de la commande doit etre suivi de \'%c\'\n", MESSAGE_SEPARATOR);
            break;
        
        case MESSAGE_ERROR_OPEN_WORD :
            NRF_LOG_PRINTF("Le debut de la commande doit etre suivi de \'%c\'\n", MESSAGE_SEPARATOR);
            break;
        
        case MESSAGE_ERROR_LEN_WORD :
            NRF_LOG_PRINTF("Le parametre ne contient rien\n");
            break;
        
        case MESSAGE_ERROR_NO_CMD :
            NRF_LOG_PRINTF("L instruction ne contient pas de commande\n");
            break;
    }
}


