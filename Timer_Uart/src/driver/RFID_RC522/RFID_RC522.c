/*
 * RFID_RC522.c
 *
 * MFRC522 RFID reader driver through the custom SPI HAL.
 */

#include "RFID_RC522.h"
#include "../../common/delay/delay.h"

#define MFRC522_REG_COMMAND        0x01u
#define MFRC522_REG_COM_IEN        0x02u
#define MFRC522_REG_DIV_IEN        0x03u
#define MFRC522_REG_COM_IRQ        0x04u
#define MFRC522_REG_DIV_IRQ        0x05u
#define MFRC522_REG_ERROR          0x06u
#define MFRC522_REG_STATUS2        0x08u
#define MFRC522_REG_FIFO_DATA      0x09u
#define MFRC522_REG_FIFO_LEVEL     0x0Au
#define MFRC522_REG_CONTROL        0x0Cu
#define MFRC522_REG_BIT_FRAMING    0x0Du
#define MFRC522_REG_COLL           0x0Eu
#define MFRC522_REG_MODE           0x11u
#define MFRC522_REG_TX_CONTROL     0x14u
#define MFRC522_REG_TX_ASK         0x15u
#define MFRC522_REG_RF_CFG         0x26u
#define MFRC522_REG_CRC_RESULT_H   0x21u
#define MFRC522_REG_CRC_RESULT_L   0x22u
#define MFRC522_REG_T_MODE         0x2Au
#define MFRC522_REG_T_PRESCALER    0x2Bu
#define MFRC522_REG_T_RELOAD_H     0x2Cu
#define MFRC522_REG_T_RELOAD_L     0x2Du
#define MFRC522_REG_VERSION        0x37u

#define MFRC522_CMD_IDLE           0x00u
#define MFRC522_CMD_CALC_CRC       0x03u
#define MFRC522_CMD_TRANSCEIVE     0x0Cu
#define MFRC522_CMD_SOFT_RESET     0x0Fu

#define PICC_CMD_REQA              0x26u
#define PICC_CMD_WUPA              0x52u
#define PICC_CMD_ANTICOLL_CL1      0x93u
#define PICC_CMD_HLTA              0x50u

static uint8_t RFID_RC522_MakeWriteAddress(uint8_t reg)
{
    return (uint8_t)((reg << 1) & 0x7Eu);
}

static uint8_t RFID_RC522_MakeReadAddress(uint8_t reg)
{
    return (uint8_t)(((reg << 1) & 0x7Eu) | 0x80u);
}

static RFID_RC522_Status_t RFID_RC522_SelectTransfer(SPI_TypeDef_t *spi, uint8_t txData, uint8_t *rxData)
{
    SPI_Status_t status = SPI_TransferByte(spi, txData, rxData, RFID_RC522_DEFAULT_TIMEOUT_MS);
    if (status == SPI_OK) {
        return RFID_RC522_OK;
    }
    if (status == SPI_TIMEOUT || status == SPI_BUSY) {
        return RFID_RC522_TIMEOUT;
    }
    return RFID_RC522_ERROR;
}

static RFID_RC522_Status_t RFID_RC522_WriteRegister(SPI_TypeDef_t *spi, uint8_t reg, uint8_t value)
{
    RFID_RC522_Status_t status;

    SPI_SetSlaveSelect(spi, 1u);
    status = RFID_RC522_SelectTransfer(spi, RFID_RC522_MakeWriteAddress(reg), 0);
    if (status == RFID_RC522_OK) {
        status = RFID_RC522_SelectTransfer(spi, value, 0);
    }
    SPI_SetSlaveSelect(spi, 0u);

    return status;
}

static uint8_t RFID_RC522_ReadRegister(SPI_TypeDef_t *spi, uint8_t reg)
{
    uint8_t value = 0u;

    SPI_SetSlaveSelect(spi, 1u);
    (void)RFID_RC522_SelectTransfer(spi, RFID_RC522_MakeReadAddress(reg), 0);
    (void)RFID_RC522_SelectTransfer(spi, 0x00u, &value);
    SPI_SetSlaveSelect(spi, 0u);

    return value;
}

static void RFID_RC522_SetBitMask(SPI_TypeDef_t *spi, uint8_t reg, uint8_t mask)
{
    uint8_t value = RFID_RC522_ReadRegister(spi, reg);
    (void)RFID_RC522_WriteRegister(spi, reg, value | mask);
}

static void RFID_RC522_ClearBitMask(SPI_TypeDef_t *spi, uint8_t reg, uint8_t mask)
{
    uint8_t value = RFID_RC522_ReadRegister(spi, reg);
    (void)RFID_RC522_WriteRegister(spi, reg, (uint8_t)(value & (uint8_t)~mask));
}

static void RFID_RC522_ClearComIrq(SPI_TypeDef_t *spi)
{
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COM_IRQ, 0x7Fu);
}

static void RFID_RC522_ClearDivIrq(SPI_TypeDef_t *spi, uint8_t mask)
{
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_DIV_IRQ, (uint8_t)(mask & 0x7Fu));
}

static void RFID_RC522_Reset(SPI_TypeDef_t *spi)
{
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, MFRC522_CMD_SOFT_RESET);
    delay_ms(50u);
}

static void RFID_RC522_AntennaOn(SPI_TypeDef_t *spi)
{
    uint8_t value = RFID_RC522_ReadRegister(spi, MFRC522_REG_TX_CONTROL);
    if ((value & 0x03u) != 0x03u) {
        RFID_RC522_SetBitMask(spi, MFRC522_REG_TX_CONTROL, 0x03u);
    }
}

static RFID_RC522_Status_t RFID_RC522_ToCard(SPI_TypeDef_t *spi,
                                             uint8_t command,
                                             const uint8_t *sendData,
                                             uint8_t sendLen,
                                             uint8_t *backData,
                                             uint8_t backMaxLen,
                                             uint16_t *backBits)
{
    uint8_t irqEnable = 0x00u;
    uint8_t waitIrq = 0x00u;
    uint8_t irqStatus = 0u;
    uint8_t fifoLevel;
    uint8_t lastBits;
    uint8_t errorReg;
    uint8_t readLen;
    uint32_t startTime;
    uint8_t index;

    if (command == MFRC522_CMD_TRANSCEIVE) {
        irqEnable = 0x77u;
        waitIrq = 0x30u;
    }

    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COM_IEN, irqEnable | 0x80u);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
    RFID_RC522_ClearComIrq(spi);
    RFID_RC522_SetBitMask(spi, MFRC522_REG_FIFO_LEVEL, 0x80u);

    for (index = 0u; index < sendLen; index++) {
        (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_FIFO_DATA, sendData[index]);
    }

    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, command);
    if (command == MFRC522_CMD_TRANSCEIVE) {
        RFID_RC522_SetBitMask(spi, MFRC522_REG_BIT_FRAMING, 0x80u);
    }

    startTime = millis();
    do {
        irqStatus = RFID_RC522_ReadRegister(spi, MFRC522_REG_COM_IRQ);
        if ((irqStatus & waitIrq) != 0u) {
            break;
        }
        if ((irqStatus & 0x01u) != 0u) {
            RFID_RC522_ClearBitMask(spi, MFRC522_REG_BIT_FRAMING, 0x80u);
            return RFID_RC522_NO_CARD;
        }
    } while ((millis() - startTime) < RFID_RC522_DEFAULT_TIMEOUT_MS);

    RFID_RC522_ClearBitMask(spi, MFRC522_REG_BIT_FRAMING, 0x80u);

    if ((irqStatus & waitIrq) == 0u) {
        return RFID_RC522_TIMEOUT;
    }

    errorReg = RFID_RC522_ReadRegister(spi, MFRC522_REG_ERROR);
    if ((errorReg & 0x08u) != 0u) {
        return RFID_RC522_COLLISION;
    }
    if ((errorReg & 0x13u) != 0u) {
        return RFID_RC522_ERROR;
    }

    if (command == MFRC522_CMD_TRANSCEIVE && backData != 0 && backBits != 0) {
        fifoLevel = RFID_RC522_ReadRegister(spi, MFRC522_REG_FIFO_LEVEL);
        lastBits = RFID_RC522_ReadRegister(spi, MFRC522_REG_CONTROL) & 0x07u;

        if (fifoLevel == 0u) {
            *backBits = 0u;
            return RFID_RC522_ERROR;
        }
        if (lastBits != 0u) {
            *backBits = (uint16_t)((fifoLevel - 1u) * 8u + lastBits);
        }
        else {
            *backBits = (uint16_t)(fifoLevel * 8u);
        }

        readLen = fifoLevel;
        if (readLen > backMaxLen) {
            readLen = backMaxLen;
        }
        if (readLen > RFID_RC522_MAX_LEN) {
            readLen = RFID_RC522_MAX_LEN;
        }
        for (index = 0u; index < readLen; index++) {
            backData[index] = RFID_RC522_ReadRegister(spi, MFRC522_REG_FIFO_DATA);
        }
    }

    return RFID_RC522_OK;
}

static RFID_RC522_Status_t RFID_RC522_CalculateCRC(SPI_TypeDef_t *spi, const uint8_t *data, uint8_t len, uint8_t crc[2])
{
    uint8_t index;
    uint32_t startTime;

    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
    RFID_RC522_ClearDivIrq(spi, 0x04u);
    RFID_RC522_SetBitMask(spi, MFRC522_REG_FIFO_LEVEL, 0x80u);

    for (index = 0u; index < len; index++) {
        (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_FIFO_DATA, data[index]);
    }
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, MFRC522_CMD_CALC_CRC);

    startTime = millis();
    while ((RFID_RC522_ReadRegister(spi, MFRC522_REG_DIV_IRQ) & 0x04u) == 0u) {
        if ((millis() - startTime) >= RFID_RC522_DEFAULT_TIMEOUT_MS) {
            (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_COMMAND, MFRC522_CMD_IDLE);
            return RFID_RC522_TIMEOUT;
        }
    }

    crc[0] = RFID_RC522_ReadRegister(spi, MFRC522_REG_CRC_RESULT_L);
    crc[1] = RFID_RC522_ReadRegister(spi, MFRC522_REG_CRC_RESULT_H);
    return RFID_RC522_OK;
}

static RFID_RC522_Status_t RFID_RC522_Request(SPI_TypeDef_t *spi, uint8_t requestMode, uint8_t *tagType)
{
    uint16_t backBits = 0u;
    RFID_RC522_Status_t status;

    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_BIT_FRAMING, 0x07u);
    status = RFID_RC522_ToCard(spi, MFRC522_CMD_TRANSCEIVE, &requestMode, 1u, tagType, 2u, &backBits);
    if (status != RFID_RC522_OK) {
        return status;
    }
    if (backBits != 0x10u) {
        return RFID_RC522_ERROR;
    }
    return RFID_RC522_OK;
}

static RFID_RC522_Status_t RFID_RC522_Anticoll(SPI_TypeDef_t *spi, uint8_t uid[RFID_RC522_UID_SIZE])
{
    uint8_t serialData[5];
    uint8_t command[2];
    uint8_t checksum = 0u;
    uint16_t backBits = 0u;
    uint8_t index;
    RFID_RC522_Status_t status;

    command[0] = PICC_CMD_ANTICOLL_CL1;
    command[1] = 0x20u;

    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_BIT_FRAMING, 0x00u);
    status = RFID_RC522_ToCard(spi, MFRC522_CMD_TRANSCEIVE, command, 2u, serialData, 5u, &backBits);
    if (status != RFID_RC522_OK) {
        return status;
    }
    if (backBits != 40u) {
        return RFID_RC522_ERROR;
    }

    for (index = 0u; index < RFID_RC522_UID_SIZE; index++) {
        checksum ^= serialData[index];
        uid[index] = serialData[index];
    }
    if (checksum != serialData[4]) {
        return RFID_RC522_ERROR;
    }

    return RFID_RC522_OK;
}

static void RFID_RC522_Halt(SPI_TypeDef_t *spi)
{
    uint8_t buffer[4];
    uint8_t crc[2];
    uint16_t backBits = 0u;

    buffer[0] = PICC_CMD_HLTA;
    buffer[1] = 0x00u;
    if (RFID_RC522_CalculateCRC(spi, buffer, 2u, crc) == RFID_RC522_OK) {
        buffer[2] = crc[0];
        buffer[3] = crc[1];
        (void)RFID_RC522_ToCard(spi, MFRC522_CMD_TRANSCEIVE, buffer, 4u, 0, 0u, &backBits);
    }
}

static char RFID_RC522_HexNibble(uint8_t value)
{
    value &= 0x0Fu;
    if (value < 10u) {
        return (char)('0' + value);
    }
    return (char)('A' + (value - 10u));
}

void RFID_RC522_Init(SPI_TypeDef_t *spi)
{
    RFID_RC522_Reset(spi);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_T_MODE, 0x8Du);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_T_PRESCALER, 0x3Eu);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_T_RELOAD_L, 30u);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_T_RELOAD_H, 0u);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_TX_ASK, 0x40u);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_MODE, 0x3Du);
    (void)RFID_RC522_WriteRegister(spi, MFRC522_REG_RF_CFG, 0x70u);
    RFID_RC522_ClearBitMask(spi, MFRC522_REG_STATUS2, 0x08u);
    RFID_RC522_AntennaOn(spi);
}

uint8_t RFID_RC522_ReadVersion(SPI_TypeDef_t *spi)
{
    return RFID_RC522_ReadRegister(spi, MFRC522_REG_VERSION);
}

void RFID_RC522_ReadDiagnostic(SPI_TypeDef_t *spi, RFID_RC522_Diag_t *diag)
{
    if (diag == 0) {
        return;
    }

    diag->spiStatus = SPI_GetStatus(spi);
    diag->version = RFID_RC522_ReadRegister(spi, MFRC522_REG_VERSION);
    diag->txControl = RFID_RC522_ReadRegister(spi, MFRC522_REG_TX_CONTROL);
    diag->rfConfig = RFID_RC522_ReadRegister(spi, MFRC522_REG_RF_CFG);
    diag->comIrq = RFID_RC522_ReadRegister(spi, MFRC522_REG_COM_IRQ);
    diag->error = RFID_RC522_ReadRegister(spi, MFRC522_REG_ERROR);
    diag->fifoLevel = RFID_RC522_ReadRegister(spi, MFRC522_REG_FIFO_LEVEL);
    diag->bitFraming = RFID_RC522_ReadRegister(spi, MFRC522_REG_BIT_FRAMING);
    diag->status2 = RFID_RC522_ReadRegister(spi, MFRC522_REG_STATUS2);
}

RFID_RC522_Status_t RFID_RC522_ReadCard(SPI_TypeDef_t *spi, uint8_t uid[RFID_RC522_UID_SIZE])
{
    uint8_t tagType[2];
    RFID_RC522_Status_t status;

    status = RFID_RC522_Request(spi, PICC_CMD_REQA, tagType);
    if (status != RFID_RC522_OK) {
        status = RFID_RC522_Request(spi, PICC_CMD_WUPA, tagType);
    }
    if (status != RFID_RC522_OK) {
        return status;
    }

    status = RFID_RC522_Anticoll(spi, uid);
    if (status == RFID_RC522_OK) {
        RFID_RC522_Halt(spi);
    }
    return status;
}

uint8_t RFID_RC522_IsSameUid(const uint8_t left[RFID_RC522_UID_SIZE], const uint8_t right[RFID_RC522_UID_SIZE])
{
    uint8_t index;

    for (index = 0u; index < RFID_RC522_UID_SIZE; index++) {
        if (left[index] != right[index]) {
            return 0u;
        }
    }
    return 1u;
}

void RFID_RC522_CopyUid(uint8_t dest[RFID_RC522_UID_SIZE], const uint8_t src[RFID_RC522_UID_SIZE])
{
    uint8_t index;

    for (index = 0u; index < RFID_RC522_UID_SIZE; index++) {
        dest[index] = src[index];
    }
}

void RFID_RC522_FormatUid(const uint8_t uid[RFID_RC522_UID_SIZE], char *text)
{
    uint8_t index;

    for (index = 0u; index < RFID_RC522_UID_SIZE; index++) {
        text[index * 2u] = RFID_RC522_HexNibble(uid[index] >> 4);
        text[index * 2u + 1u] = RFID_RC522_HexNibble(uid[index]);
    }
    text[RFID_RC522_UID_SIZE * 2u] = '\0';
}

