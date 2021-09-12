#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <string.h>
#include "format_column.h"
#include "common.h"
#include "format_long.h"
#include "list.h"
#include "display.h"

void column_display(linklist l, int *ls, int *lm, char **tb, int *ts, int *tm, int cl, int ln)
{
    int x;
    lf_type t;

    for (int i = 0; i < ln; i++)
    {
        for (int j = 0; j < cl; j++)
        {
            x = j * ln + i;
            if (x < l->count)
            {
                // if options -s, -p, -m, -u or -g is set
                t = (lf_type)lget(l, x);
                if (tb)
                {
                    long_print(tb[x], tm[j] - 1, 1);
                }
                display(t->name, &t->st.st_mode, false);
                x = lm[j] - ls[x];
                for (int k = 0; k < x; k++)
                { // the +1 is for the last space between columns.
                    printf(" ");
                }
                if (j != cl - 1)
                { // don't add space at the last column
                    printf("  ");
                }
            }
        }
        printf("\n");
    }
}

void column_main(linklist l, char **tb)
{
    struct winsize w;
    int cl = l->count, ln = 1, winsiz, ok = 0;
    unsigned long int cnt;
    int *ls;        // list sizes
    int *lm;        // list max sizes
    int *ts = NULL; // array sizes of "tb"
    int *tm = NULL; // array max sizes of "tb"
    int x, y = 0;
    iterator it;
    lf_type t;
    int k;

    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    winsiz = w.ws_col;
    ls = (int *)lf_alloc(sizeof(int) * l->count);
    lm = (int *)lf_alloc(sizeof(int) * l->count);
    if (tb)
    {
        ts = (int *)lf_alloc(sizeof(int) * l->count);
        tm = (int *)lf_alloc(sizeof(int) * l->count);
    }
    // length of each name.
    it = lat(l, LFIRST);
    int nb_spaces;
    for (int i = 0; i < l->count; i++)
    {
        t = (lf_type)it->data;
        ls[i] = strwidth(t->name);
        nb_spaces = has_space(t->name);
        if (nb_spaces)
        {
            if (LFopt.nl->b)
            {
                ls[i] += nb_spaces;
            }
            if (LFopt.nl->q || !LFopt.nl->b)
            {
               ls[i] += 2;
            }
        }
        else if (LFopt.nl->q)
        {
            ls[i] += 2;
        }
        if (S_ISDIR(t->st.st_mode) && LFopt.nl->s)
        {
            ls[i] += 1;
        }
        if (tb)
        {
            ts[i] = strlen(tb[i]) + 1; // +1 for space between "tb" and "l"
        }
        linc(&it);
    }
    while (!ok)
    {
        cnt = 0;
        for (int i = 0; i < cl; i++)
        { // for each column "i", calculates the maximum of that column
            x = ls[i * ln];
            if (tb)
            {
                y = ts[i * ln];
            }
            for (int j = 1; j < ln; j++)
            {
                k = i * ln + j;
                if (k < l->count)
                {
                    if (ls[k] > x)
                    {
                        x = ls[k];
                    }
                    if (tb && (ts[k] > y))
                    {
                        y = ts[k];
                    }
                }
            }
            if (tb)
            {
                cnt += x + y;
                tm[i] = y;
            }
            else
            {
                cnt += x;
            }
            lm[i] = x;
        }
        cnt += 2 * (cl - 1); // space between columns
        if (cnt <= winsiz)
        {
            ok = 1;
        }
        else
        {
            cl--;
            if (l->count % cl)
            {
                ln = l->count / cl + 1;
            }
            else
            {
                ln = l->count / cl;
            }
        }
    }
    column_display(l, ls, lm, tb, ts, tm, cl, ln);
    free(ls);
    free(ts);
    free(lm);
    free(tm);
}
