#include <locale.h>
#include <limits.h>
#include <string.h>

size_t locales_amount = 1;

static const struct lconv locale_values[] = 
{
    {
        /* LC_MONETARY */
        "",         /* currency_symbol */
        "",         /* int_curr_symbol */
        "",         /* mon_decimal_point */
        "",         /* mon_grouping */
        "",         /* mon_thousands_sep */
        "",         /* negative_sign */
        "",         /* positive_sign */
        CHAR_MAX,   /* frac_digits */
        CHAR_MAX,   /* int_frac_digits */
        CHAR_MAX,   /* n_cs_precedes */
        CHAR_MAX,   /* n_sep_by_space */
        CHAR_MAX,   /* n_sign_posn */
        CHAR_MAX,   /* p_cs_precedes */
        CHAR_MAX,   /* p_sep_by_space */
        CHAR_MAX,   /* p_sign_posn */
        /* LC_NUMERIC */
        ".",        /* decimal_point */
        "",         /* grouping */
        "",         /* thousands_sep */
    }
};

static const char* locale_names[] =
{
    "C"
};

size_t active_locale = 0;

struct lconv* localeconv(void)
{
    return &(locale_values[active_locale]);
}

char* setlocale(int category, const char* locale)
{
    return locale_names[active_locale];
}