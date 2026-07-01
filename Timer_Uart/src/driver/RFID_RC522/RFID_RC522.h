/*
 * RFID_RC522.h
 *
 * MFRC522 RFID reader driver through the custom SPI HAL.
 */

#ifndef SRC_DRIVER_RFID_RC522_RFID_RC522_H_
#define SRC_DRIVER_RFID_RC522_RFID_RC522_H_

#include <stdint.h>
#include "../../HAL/SPI/SPI.h"

#define RFID_RC522_UID_SIZE           4u
#define RFID_RC522_MAX_LEN            16u
#define RFID_RC522_DEFAULT_TIMEOUT_MS 25u

typedef enum {
    RFID_RC522_OK = 0,
    RFID_RC522_NO_CARD = -1,
    RFID_RC522_TIMEOUT = -2,
    RFID_RC522_ERROR = -3,
    RFID_RC522_COLLISION = -4
} RFID_RC522_Status_t;

typedef struct {
    uint32_t spiStatus;
    uint8_t version;
    uint8_t txControl;
    uint8_t rfConfig;
    uint8_t comIrq;
    uint8_t error;
    uint8_t fifoLevel;
    uint8_t bitFraming;
    uint8_t status2;
} RFID_RC522_Diag_t;

void RFID_RC522_Init(SPI_TypeDef_t *spi);
uint8_t RFID_RC522_ReadVersion(SPI_TypeDef_t *spi);
void RFID_RC522_ReadDiagnostic(SPI_TypeDef_t *spi, RFID_RC522_Diag_t *diag);
RFID_RC522_Status_t RFID_RC522_ReadCard(SPI_TypeDef_t *spi, uint8_t uid[RFID_RC522_UID_SIZE]);
uint8_t RFID_RC522_IsSameUid(const uint8_t left[RFID_RC522_UID_SIZE], const uint8_t right[RFID_RC522_UID_SIZE]);
void RFID_RC522_CopyUid(uint8_t dest[RFID_RC522_UID_SIZE], const uint8_t src[RFID_RC522_UID_SIZE]);
void RFID_RC522_FormatUid(const uint8_t uid[RFID_RC522_UID_SIZE], char *text);

#endif /* SRC_DRIVER_RFID_RC522_RFID_RC522_H_ */

