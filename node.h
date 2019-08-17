// SPDX-License-Identifier: MIT
// Copyright (C) 2019 Ahirui Otsu

#ifndef NODE_H
#define NODE_H

#include <stdlib.h>

typedef enum type {
        EXPR,
        MUL_EXPR,
        UNARY_EXPR,
        UNARY_OPR,
        NUM
} Type;

typedef struct node {
        Type type;
        size_t len;
        size_t size;
        int num;
        struct child {
                char op;
                struct node *node;
        } *children;
} Node;

extern void node_delete(Node *node);
extern Node *parse(const char *s);
extern int calc(const Node *node);

#endif // !NODE_H
