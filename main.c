// SPDX-License-Identifier: MIT
// Copyright (C) 2019 Ahirui Otsu

#include <stdio.h>
#include <stdlib.h>
#include "node.h"

static char *readline(void)
{
        size_t size = 128;
        char *s = malloc(sizeof(*s) * size);
        if (s == NULL)
                return NULL;
        size_t len = 0;
        for (;;) {
                int ch = getchar();
                if (ch == EOF)
                        return NULL;
                if (ch == '\n')
                        break;
                if (len >= size) {
                        char *new = realloc(s, sizeof(*new) * size * 2);
                        if (new == NULL) {
                                free(s);
                                return NULL;
                        }
                        size *= 2;
                        s = new;
                }
                s[len++] = ch;
        }
        s[len] = '\0';
        return s;
}

int main(void)
{
        char *line;
        while ((line = readline()) != NULL) {
                Node *node = parse(line);
                if (node == NULL) {
                        fprintf(stderr, "parse error\n");
                } else {
                        printf("= %d\n", calc(node));
                }
                node_delete(node);
                free(line);
        }
        return EXIT_SUCCESS;
}
