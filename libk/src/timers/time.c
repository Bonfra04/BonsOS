#include <timers/time.h>
#include <timers/rtc.h>

#define EPOCH_YEAR 1970

// Julian date calculation from https://en.wikipedia.org/wiki/Julian_day
static uint64_t get_jdn(uint8_t days, uint8_t months, uint16_t years)
{
    return (1461 * (years + 4800 + (months - 14)/12))/4 + (367 *
           (months - 2 - 12 * ((months - 14)/12)))/12 -
           (3 * ((years + 4900 + (months - 14)/12)/100))/4
           + days - 32075;
}

uint64_t time_sinceepoch()
{
    uint64_t years = rtc_year();
    uint64_t months = rtc_month();
    uint64_t days = rtc_monthday();
    uint64_t hours = rtc_hours();
    uint64_t minutes = rtc_minutes();
    uint64_t seconds = rtc_seconds();

    uint64_t jdn_current = get_jdn(days, months, years);
    uint64_t jdn_1970 = get_jdn(1, 1, 1970); 
    uint64_t jdn_diff = jdn_current - jdn_1970;

    return ((jdn_diff * 24 + hours) * 60 + minutes) * 60 + seconds;
}
