#include <locale.h>
#include <stdio.h>

#include "utf8.h"

int main(int argc, char **argv)
{
    int i, j, s_is_alloc;
    wchar_t *pws;
    char *r, *s;
    size_t n;
    setlocale(LC_ALL, ""); /* this is always necessary */
    for (i = 1; i < argc; i++) {
        s_is_alloc = 0;
        printf("The argument #%d is \"%s\".\n", i, argv[i]);
        n = utf8_to_wchars(NULL, argv[i], (size_t)-1);
        if (n == 0) continue;
        if (n == (size_t)-1) {
            printf("The argument #%d isn't valid UTF-8.\n", i);
            printf("Trying to convert from locale...\n");
            n = utf8_of_local(NULL, argv[i], (size_t)-1);
            if (n == (size_t)-1) {
                printf("Can't convert \"%s\" to UTF-8.\n", argv[i]);
                continue;
            }
            s = (char *)malloc((n + 1) * sizeof(char));
            if (s == NULL) {
                printf("There is no more memory available.\n");
                return argc - i;
            }
            s_is_alloc = 1;
            n = utf8_of_local(s, argv[i], n + 1);
            n = utf8_to_wchars(NULL, s, (size_t)-1);
        } else {
            printf("The argument \"%s\" is valid UTF-8.\n", argv[i]);
            s = argv[i];
        }
        printf("It can be converted to %zu non-zero wide characters.\n", n);
        pws = (wchar_t *)malloc((n + 1) * sizeof(wchar_t));
        if (pws == NULL) {
            printf("There is no more memory available.\n");
            return argc - i;
        }
        n = utf8_to_wchars(pws, s, n + 1);
        printf("The wide character codes are:\n");
        for (j = 0; j < n; j++) {
            if (j > 0) putchar(' ');
            printf("0x%x", pws[j]);
        }
        putchar('\n');
        free(pws);
        if (s_is_alloc) {
            n = utf8_to_local(NULL, s, (size_t)-1);
            if (n > 0) {
                r = (char *)malloc(n + 1);
                if (r != NULL) {
                    n = utf8_to_local(r, s, n + 1);
                    printf("The string reconverted to locale is \"%s\".\n", r);
                    free(r);
                }
            }
            free(s);
        }
    }
    return argc - i;
}
