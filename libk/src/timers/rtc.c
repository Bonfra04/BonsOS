#include <timers/rtc.h>
#include <cmos.h>
#include <acpi.h>

#include <stdbool.h>

#define ACPI_CENTURY_FIELD_OFFSET 108

#define REG_SECONDS  0x00
#define REG_MINUTES  0x02
#define REG_HOURS    0x04
#define REG_WEEKDAY  0x06
#define REG_MONTHDAY 0x07
#define REG_MONTH    0x08
#define REG_YEAR     0x09

#define REG_STATUSA 0x0A
#define REG_STATUSB 0x0B

#define UPDATE_BIT  0x80
#define BCD_BIT     0x04
#define AMPM_BIT    0x02

static uint8_t reg_century;
static bool is_bcd;

static void wait_update()
{
    while(cmos_read_byte(REG_STATUSA) & UPDATE_BIT)
        asm("pause");
}

static inline uint8_t care_bcd(uint8_t val)
{
    return is_bcd ? (val & 0xF) + (val / 16 * 10) : val;
}

void rtc_init()
{
    void* entry = acpi_find_entry("FACP");
    reg_century = *((uint8_t*)entry + ACPI_CENTURY_FIELD_OFFSET);

    is_bcd = !(cmos_read_byte(REG_STATUSB) & BCD_BIT);
}

uint8_t rtc_seconds()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_SECONDS));
}

uint8_t rtc_minutes()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_MINUTES));
}

uint8_t rtc_hours()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_HOURS));
}

uint8_t rtc_weekday()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_WEEKDAY));
}

uint8_t rtc_monthday()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_MONTHDAY));
}

uint8_t rtc_month()
{
    wait_update();
    return care_bcd(cmos_read_byte(REG_MONTH));
}

uint16_t rtc_year()
{
    wait_update();
    uint8_t century = reg_century ? care_bcd(cmos_read_byte(reg_century)) : 20;
    uint8_t year = care_bcd(cmos_read_byte(REG_YEAR));
    return (uint16_t)century * 100 + year;
}
