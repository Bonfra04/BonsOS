#pragma once

#define LC_ALL      0
#define LC_COLLATE  1
#define LC_CTYPE    2
#define LC_MONETARY 3
#define LC_NUMERIC  4
#define LC_TIME 5

#ifdef __cplusplus
extern "C" {
#endif

struct lconv
{
    /* controlled by LC_MONETARY */
    char* currency_symbol;
    char* int_curr_symbol;
    char* mon_decimal_point;
    char* mon_grouping;
    char* mon_thousands_sep;
    char* negative_sign;
    char* positive_sign;
    char frac_digits;
    char int_frac_digits;
    char n_cs_precedes;
    char n_sep_by_space;
    char n_sign_posn;
    char p_cs_precedes;
    char p_sep_by_space;
    char p_sign_posn;
    /* controlled by LC_NUMERIC */
    char* decimal_point;
    char* grouping;
    char* thousands_sep;
};

struct lconv* localeconv(void);
char* setlocale(int category, const char* locale);

#ifdef __cplusplus
}
#endif
