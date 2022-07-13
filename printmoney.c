
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>


int print_money_internal(char* str, size_t size, double value, int intr, struct lconv* lc)
{
    // Only used 1st char of mon_grouping
    // Whitespace is used instead of mon_thousands_sep in case of mon_grouping > 0
    static char buf[150] = {0};

    size_t frac_digits = intr ? lc->int_frac_digits : lc->frac_digits;
    if (frac_digits == CHAR_MAX)
        return -1;
    size_t grouping = lc->mon_grouping[0];
    char* curr = intr ? lc->int_curr_symbol : lc->currency_symbol;
    size_t curr_len = strlen(curr);
    char* point = lc->mon_decimal_point;
    size_t point_len = strlen(point);

    double vshift = value;
    for (size_t fd = 0; fd < frac_digits; fd++)
        vshift *= 10.0;
    long long vshifted = (long long)round(vshift);

    char* sign;
    char order;
    char space;
    char signpos;
    if (vshifted >= 0)
    {
        sign = lc->positive_sign;
        order = intr ? lc->int_p_cs_precedes : lc->p_cs_precedes;
        space = intr ? lc->int_p_sep_by_space : lc->p_sep_by_space;
        signpos = intr ? lc->int_p_sign_posn : lc->p_sign_posn;
    }
    else
    {
        vshifted = -vshifted;
        sign = lc->negative_sign;
        order = intr ? lc->int_n_cs_precedes : lc->n_cs_precedes;
        space = intr ? lc->int_n_sep_by_space : lc->n_sep_by_space;
        signpos = intr ? lc->int_n_sign_posn : lc->n_sign_posn;
    }
    size_t sign_len = strlen(sign);

    // Value
    static char vbuf[150] = {0};
    size_t vlen = sprintf(vbuf, "%0*lld", frac_digits+1, vshifted);
    char* vbufpos = vbuf + vlen;
    // Calculate end pointer
    char* bufpos = buf + vlen + point_len + curr_len;
    size_t grpcount = 0;
    if (lc->mon_thousands_sep[0] != 0 && vlen - frac_digits > 0)
        grpcount = (vlen - frac_digits - 1) / grouping;
    bufpos += grpcount;
    bufpos += (space != 0);
    if (signpos == 0)
        bufpos += 2;
    else
        bufpos += sign_len;
    size_t output_size = bufpos - buf;
    *bufpos = 0;

    #define PRINT_CHAR(c)  (*--bufpos = (c))
    #define PRINT_SIGN      memcpy(bufpos -= sign_len, sign, sign_len)
    #define PRINT_CURR      memcpy(bufpos -= curr_len, curr, curr_len)
    #define PRINT_POINT     memcpy(bufpos -= point_len, point, point_len)
    #define PRINT_VBUF(n)   memcpy(bufpos -= (n), vbufpos -= (n), n)

    // Print after value
    if (!order)
    {
        if (signpos == 0)
            PRINT_CHAR(')');
        else if (signpos == 2 || signpos == 4)
            PRINT_SIGN;
        if (signpos != 2)
            PRINT_CURR;
        if (signpos == 3)
            PRINT_SIGN;
        if (space)
            PRINT_CHAR(' ');
        if (signpos == 2)
            PRINT_CURR;
    }
    else
    {
        if (signpos == 0)
            PRINT_CHAR(')');
        else if (signpos == 2)
            PRINT_SIGN;
    }

    PRINT_VBUF(frac_digits);
    PRINT_POINT;
    for (size_t i = 0; i < grpcount; i++)
    {
        PRINT_VBUF(grouping);
        PRINT_CHAR(' ');
    }
    size_t remain = vbufpos - vbuf;
    PRINT_VBUF(remain);

    // Print before value
    if (!order)
    {
        if (signpos == 0)
            PRINT_CHAR('(');
        else if (signpos == 1)
            PRINT_SIGN;
    }
    else
    {
        if (space)
            PRINT_CHAR(' ');
        if (signpos == 4)
            PRINT_SIGN;
        PRINT_CURR;
        if (signpos == 0)
            PRINT_CHAR('(');
        else if (signpos == 1 || signpos == 3)
            PRINT_SIGN;
    }

    // Debug assertions (uncomment to debug)
    //assert(bufpos == buf);
    //assert(vbufpos == vbuf);
    //assert(strlen(buf) == output_size);

    // Done
    strncpy(str, buf, size);
    return output_size < size ? output_size : size;
}


int print_money(char* str, size_t size, double m, int intr)
{
    return print_money_internal(str, size, m, intr, localeconv());
}
