#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <syscalls.h>

time_t time(time_t* timer)
{
    time_t t = sys_time();
    if(timer)
        *timer = t;
    return t;
}

struct tm* localtime(const time_t* timer)
{
    uint64_t T = *timer;

    uint64_t day = T / (60 * 60 * 24);
    uint64_t sec = T % (60 * 60 * 24);

    uint64_t wday = (day + 4) % 7;

    uint64_t year = 1970;
    while(day >= 365)
    {
        day -= 365;
        if(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
            day--;
        year++;
    }
    day += 1;
    if(day == 0)
    {
        year--;
        day = 366;
    }

    uint64_t hour = sec / (60 * 60);
    sec %= (60 * 60);
    uint64_t min = sec / 60;
    sec %= 60;

    bool is_leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);

    uint64_t month;
    if(1 <= day && day <= 31) month = 1;
    else if(32 <= day && day <= 59 + is_leap) month = 2;
    else if(60 + is_leap <= day && day <= 90 + is_leap) month = 3;
    else if(91 + is_leap <= day && day <= 120 + is_leap) month = 4;
    else if(121 + is_leap <= day && day <= 151 + is_leap) month = 5;
    else if(152 + is_leap <= day && day <= 181 + is_leap) month = 6;
    else if(182 + is_leap <= day && day <= 212 + is_leap) month = 7;
    else if(213 + is_leap <= day && day <= 243 + is_leap) month = 8;
    else if(244 + is_leap <= day && day <= 273 + is_leap) month = 9;
    else if(274 + is_leap <= day && day <= 304 + is_leap) month = 10;
    else if(305 + is_leap <= day && day <= 334 + is_leap) month = 11;
    else if(335 + is_leap <= day && day <= 365 + is_leap) month = 12;

    uint64_t month_day;
    if(month == 1) month_day = day;
    else if(month == 2) month_day = day - 31;
    else if(month == 3) month_day = day - 59 - is_leap;
    else if(month == 4) month_day = day - 90 - is_leap;
    else if(month == 5) month_day = day - 120 - is_leap;
    else if(month == 6) month_day = day - 151 - is_leap;
    else if(month == 7) month_day = day - 181 - is_leap;
    else if(month == 8) month_day = day - 212 - is_leap;
    else if(month == 9) month_day = day - 243 - is_leap;
    else if(month == 10) month_day = day - 273 - is_leap;
    else if(month == 11) month_day = day - 304 - is_leap;
    else if(month == 12) month_day = day - 334 - is_leap;

    static struct tm time;
    time.tm_year = year - 1900;
    time.tm_yday = day - 1;
    time.tm_hour = hour;
    time.tm_min = min;
    time.tm_sec = sec;
    time.tm_mon = month - 1;
    time.tm_mday = month_day;
    time.tm_wday = wday;
    time.tm_isdst = -1; // TODO: tm_isdst

    return &time;
}

char* asctime(const struct tm* time_ptr)
{
    static char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static char* days[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    static char buffer[26];

    sprintf(buffer, "%s %s %02d %02d:%02d:%02d %d",
        days[time_ptr->tm_wday],
        months[time_ptr->tm_mon],
        time_ptr->tm_mday,
        time_ptr->tm_hour,
        time_ptr->tm_min,
        time_ptr->tm_sec,
        time_ptr->tm_year + 1900
    );

    return buffer;
}

char* ctime(const time_t* timer)
{
    return asctime(localtime(timer));
}
