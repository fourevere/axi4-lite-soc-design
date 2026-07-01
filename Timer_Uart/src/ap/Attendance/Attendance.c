#include "Attendance.h"
#include "../Clock/Clock.h"
#include "../../HAL/SPI/SPI.h"
#include "../../driver/LCD1602_I2C/LCD1602_I2C.h"
#include "../../driver/RFID_RC522/RFID_RC522.h"
#include "../../common/delay/delay.h"
#include "xil_printf.h"

#define ATTENDANCE_SCAN_COOLDOWN_MS 1500u
#define ATTENDANCE_USER_COUNT       2u
#define ATTENDANCE_ERROR_REPORT_MS  500u
#define ATTENDANCE_DIAG_REPORT_MS   1000u

typedef struct {
    uint8_t uid[RFID_RC522_UID_SIZE];
    const char *name;
    uint8_t checkedIn;
} AttendanceUser_t;

static AttendanceUser_t attendanceUsers[ATTENDANCE_USER_COUNT] = {
    {{0x2Cu, 0x3Cu, 0x3Bu, 0x06u}, "JEONG GWANGGEUN", 0u},
    {{0xC7u, 0xDAu, 0x25u, 0x07u}, "KWON JAEWON", 0u}
};

extern volatile uint8_t rx_data;

static uint8_t lastUid[RFID_RC522_UID_SIZE];
static uint8_t lastUidValid;
static uint32_t lastScanTime;
static uint32_t lastErrorReportTime;
static uint32_t lastDiagReportTime;
static uint8_t rfidVersion;
static uint8_t rfidReady;
static uint8_t attendanceEnabled;

static char Attendance_HexNibble(uint8_t value)
{
    value &= 0x0Fu;
    if (value < 10u) {
        return (char)('0' + value);
    }
    return (char)('A' + (value - 10u));
}

static void Attendance_FormatHexByte(uint8_t value, char *text)
{
    text[0] = '0';
    text[1] = 'x';
    text[2] = Attendance_HexNibble(value >> 4);
    text[3] = Attendance_HexNibble(value);
    text[4] = '\0';
}

static uint8_t Attendance_IsRfidVersionValid(uint8_t version)
{
    return (version != 0x00u && version != 0xFFu) ? 1u : 0u;
}

static void Attendance_CopyText(char *line, uint8_t pos, const char *text)
{
    uint8_t index = 0u;

    while (text != 0 && text[index] != '\0' && pos < 16u) {
        line[pos] = text[index];
        pos++;
        index++;
    }
}

static void Attendance_FillLine(char *line)
{
    uint8_t index;

    for (index = 0u; index < 16u; index++) {
        line[index] = ' ';
    }
    line[16] = '\0';
}

static void Attendance_PrintLine(uint8_t row, const char *text)
{
    char line[17];

    Attendance_FillLine(line);
    Attendance_CopyText(line, 0u, text);

    (void)lcd1602_i2c_set_cursor(row, 0u);
    (void)lcd1602_i2c_puts(line);
}

static void Attendance_BuildVersionLine(char *line, const char *prefix, uint8_t version)
{
    char hexText[5];

    Attendance_FillLine(line);
    Attendance_FormatHexByte(version, hexText);
    Attendance_CopyText(line, 0u, prefix);
    Attendance_CopyText(line, 4u, hexText);
}

static void Attendance_PrintBoot(void)
{
    char line[17];

    (void)lcd1602_i2c_clear();

    if (Attendance_IsRfidVersionValid(rfidVersion) != 0u) {
        Attendance_PrintLine(0u, "RFID Ready");
        Attendance_PrintLine(1u, "Scan your card");
    }
    else {
        Attendance_BuildVersionLine(line, "VER", rfidVersion);
        Attendance_PrintLine(0u, "RFID SPI FAIL");
        Attendance_PrintLine(1u, line);
    }
}

static void Attendance_ForceLcdFailScreen(void)
{
    char line[17];

    (void)lcd1602_i2c_clear();
    Attendance_BuildVersionLine(line, "VER", rfidVersion);
    Attendance_PrintLine(0u, "RFID SPI FAIL");
    Attendance_PrintLine(1u, line);
}

static void Attendance_PrintDiagnostic(const char *prefix, uint8_t showLcd)
{
    RFID_RC522_Diag_t diag;
    char line[17];

    RFID_RC522_ReadDiagnostic(SPI0, &diag);
    rfidVersion = diag.version;
    rfidReady = Attendance_IsRfidVersionValid(rfidVersion);

    xil_printf("%s SPI=0x%x VER=0x%x TX=0x%x RF=0x%x IRQ=0x%x ERR=0x%x FIFO=0x%x BF=0x%x ST2=0x%x\r\n",
               prefix,
               diag.spiStatus,
               diag.version,
               diag.txControl,
               diag.rfConfig,
               diag.comIrq,
               diag.error,
               diag.fifoLevel,
               diag.bitFraming,
               diag.status2);

    if (showLcd != 0u) {
        (void)lcd1602_i2c_clear();
        Attendance_BuildVersionLine(line, "VER", diag.version);
        if (Attendance_IsRfidVersionValid(diag.version) != 0u) {
            Attendance_PrintLine(0u, "RFID Diagnostic");
            Attendance_PrintLine(1u, line);
        }
        else {
            Attendance_PrintLine(0u, "RFID SPI FAIL");
            Attendance_PrintLine(1u, line);
        }
    }
}

static AttendanceUser_t *Attendance_FindUser(const uint8_t uid[RFID_RC522_UID_SIZE])
{
    uint8_t index;

    for (index = 0u; index < ATTENDANCE_USER_COUNT; index++) {
        if (RFID_RC522_IsSameUid(attendanceUsers[index].uid, uid)) {
            return &attendanceUsers[index];
        }
    }
    return 0;
}

static void Attendance_BuildActionLine(char *line, const char *action, const char *timeText)
{
    Attendance_FillLine(line);
    Attendance_CopyText(line, 0u, action);
    Attendance_CopyText(line, 4u, timeText);
}

static void Attendance_ShowKnownUser(AttendanceUser_t *user, const uint8_t uid[RFID_RC522_UID_SIZE])
{
    ClockTime_t timeData;
    char timeText[11];
    char line2[17];
    char uidText[9];
    const char *action;

    if (user->checkedIn == 0u) {
        user->checkedIn = 1u;
        action = "IN";
    }
    else {
        user->checkedIn = 0u;
        action = "OUT";
    }

    Clock_GetTime(&timeData);
    Clock_FormatTimeTenth(&timeData, timeText);
    Attendance_BuildActionLine(line2, action, timeText);
    RFID_RC522_FormatUid(uid, uidText);

    Attendance_PrintLine(0u, user->name);
    Attendance_PrintLine(1u, line2);
    xil_printf("RFID UID=%s NAME=%s STATE=%s TIME=%s\r\n", uidText, user->name, action, timeText);
}

static void Attendance_ShowUnknownUser(const uint8_t uid[RFID_RC522_UID_SIZE])
{
    char uidText[9];
    char line2[17];

    RFID_RC522_FormatUid(uid, uidText);
    Attendance_BuildActionLine(line2, "UID", uidText);
    Attendance_PrintLine(0u, "Unknown Card");
    Attendance_PrintLine(1u, line2);
    xil_printf("RFID UID=%s NAME=UNKNOWN\r\n", uidText);
}

static uint8_t Attendance_IsRepeatedScan(const uint8_t uid[RFID_RC522_UID_SIZE])
{
    uint32_t curTime = millis();

    if (lastUidValid != 0u && RFID_RC522_IsSameUid(lastUid, uid) &&
        (curTime - lastScanTime) < ATTENDANCE_SCAN_COOLDOWN_MS) {
        return 1u;
    }

    RFID_RC522_CopyUid(lastUid, uid);
    lastUidValid = 1u;
    lastScanTime = curTime;
    return 0u;
}

static void Attendance_ShowSpiLoopLcd(uint8_t rxA5, uint8_t rx5A)
{
    char line[17];

    Attendance_FillLine(line);
    Attendance_CopyText(line, 0u, "A5>");
    line[3] = Attendance_HexNibble(rxA5 >> 4);
    line[4] = Attendance_HexNibble(rxA5);
    Attendance_CopyText(line, 7u, "5A>");
    line[10] = Attendance_HexNibble(rx5A >> 4);
    line[11] = Attendance_HexNibble(rx5A);

    (void)lcd1602_i2c_clear();
    Attendance_PrintLine(0u, "SPI Loop Test");
    Attendance_PrintLine(1u, line);
}

static void Attendance_RunSpiLoopbackDiagnostic(const char *prefix, uint8_t showLcd)
{
    uint8_t rxA5 = 0u;
    uint8_t rx5A = 0u;
    SPI_Status_t statusA5;
    SPI_Status_t status5A;
    uint32_t statusBefore;
    uint32_t statusAfter;

    statusBefore = SPI_GetStatus(SPI0);
    SPI_SetSlaveSelect(SPI0, 1u);
    statusA5 = SPI_TransferByte(SPI0, 0xA5u, &rxA5, RFID_RC522_DEFAULT_TIMEOUT_MS);
    status5A = SPI_TransferByte(SPI0, 0x5Au, &rx5A, RFID_RC522_DEFAULT_TIMEOUT_MS);
    SPI_SetSlaveSelect(SPI0, 0u);
    statusAfter = SPI_GetStatus(SPI0);

    xil_printf("%s ST0=0x%x A5_RX=0x%x A5_ST=%d 5A_RX=0x%x 5A_ST=%d ST1=0x%x\r\n",
               prefix,
               statusBefore,
               rxA5,
               (int)statusA5,
               rx5A,
               (int)status5A,
               statusAfter);

    if (showLcd != 0u) {
        Attendance_ShowSpiLoopLcd(rxA5, rx5A);
    }
}

static void Attendance_ReportReadError(RFID_RC522_Status_t status)
{
    RFID_RC522_Diag_t diag;
    uint32_t curTime = millis();

    if ((curTime - lastErrorReportTime) < ATTENDANCE_ERROR_REPORT_MS) {
        return;
    }
    lastErrorReportTime = curTime;

    RFID_RC522_ReadDiagnostic(SPI0, &diag);
    rfidVersion = diag.version;
    xil_printf("RFID read status=%d SPI=0x%x VER=0x%x IRQ=0x%x ERR=0x%x FIFO=0x%x BF=0x%x\r\n",
               (int)status,
               diag.spiStatus,
               diag.version,
               diag.comIrq,
               diag.error,
               diag.fifoLevel,
               diag.bitFraming);
}

static void Attendance_HandleRfidWait(void)
{
    uint32_t curTime = millis();

    if ((curTime - lastDiagReportTime) < ATTENDANCE_DIAG_REPORT_MS) {
        return;
    }
    lastDiagReportTime = curTime;

    Attendance_PrintDiagnostic("RFID WAIT", 1u);
    Attendance_RunSpiLoopbackDiagnostic("SPI AUTO", 0u);
    if (rfidReady != 0u) {
        RFID_RC522_Init(SPI0);
        delay_ms(5u);
        rfidVersion = RFID_RC522_ReadVersion(SPI0);
        rfidReady = Attendance_IsRfidVersionValid(rfidVersion);
        Attendance_PrintBoot();
        xil_printf("RFID RECOVER VER=0x%x\r\n", rfidVersion);
    }
}

void Attendance_Init(void)
{
    attendanceEnabled = 0u;
    lastUidValid = 0u;
    lastScanTime = 0u;
    lastErrorReportTime = 0u;
    lastDiagReportTime = 0u;
    rfidVersion = 0u;
    rfidReady = 0u;

    SPI_Init(SPI0);
    delay_ms(5u);
    RFID_RC522_Init(SPI0);
    delay_ms(5u);
    rfidVersion = RFID_RC522_ReadVersion(SPI0);
    rfidReady = Attendance_IsRfidVersionValid(rfidVersion);

    Attendance_PrintDiagnostic("RFID INIT", 0u);
}

void Attendance_Enable(void)
{
    attendanceEnabled = 1u;
    lastDiagReportTime = millis();
    Attendance_PrintBoot();
    xil_printf("MODE RFID Attendance\r\n");
}

void Attendance_Disable(void)
{
    attendanceEnabled = 0u;
}

uint8_t Attendance_IsRfidReady(void)
{
    return rfidReady;
}

void Attendance_Command(uint8_t command)
{
    if (command == 'v' || command == 'V') {
        Attendance_PrintDiagnostic("RFID DIAG", 1u);
    }
    else if (command == 's' || command == 'S') {
        Attendance_RunSpiLoopbackDiagnostic("SPI LOOP", 1u);
    }
}

void Attendance_Excute(void)
{
    uint8_t uid[RFID_RC522_UID_SIZE];
    RFID_RC522_Status_t status;
    AttendanceUser_t *user;

    if (attendanceEnabled == 0u) {
        return;
    }

    if (rx_data == 'v' || rx_data == 'V' || rx_data == 's' || rx_data == 'S') {
        Attendance_Command(rx_data);
        rx_data = 0;
        return;
    }

    if (rfidReady == 0u) {
        Attendance_ForceLcdFailScreen();
        Attendance_HandleRfidWait();
        return;
    }

    status = RFID_RC522_ReadCard(SPI0, uid);
    if (status == RFID_RC522_NO_CARD || status == RFID_RC522_TIMEOUT) {
        return;
    }
    if (status != RFID_RC522_OK) {
        Attendance_ReportReadError(status);
        return;
    }
    if (Attendance_IsRepeatedScan(uid)) {
        return;
    }

    user = Attendance_FindUser(uid);
    if (user != 0) {
        Attendance_ShowKnownUser(user, uid);
    }
    else {
        Attendance_ShowUnknownUser(uid);
    }
}
