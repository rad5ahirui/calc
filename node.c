// SPDX-License-Identifier: MIT
// Copyright (C) 2019 Ahirui Otsu

#include "node.h"
#include <stdio.h>
#include <ctype.h>

static void die(void);
static Node *node_new(Type type);
static Node *node_add(Node *node, Node *child);
static size_t skip(const char *s, size_t i);
static Node *num(const char *s, size_t *i);
static Node *unary_expr2(const char *s, size_t *i);
static Node *unary_expr(const char *s, size_t *i);
static Node *mul_expr(const char *s, size_t *i);
static Node *expr(const char *s, size_t *i);

static void die(void)
{
        fprintf(stderr, "unexpected error\n");
        exit(EXIT_FAILURE);
}

static Node *node_new(Type type)
{
        Node *node = malloc(sizeof(*node));
        const size_t size = 2;
        if (node == NULL)
                die();
        node->children = malloc(sizeof(*node->children) * size);
        if (node->children == NULL)
                die();
        node->len = 0;
        node->size = size;
        node->type = type;
        return node;
}

static Node *node_add(Node *node, Node *child)
{
        if (node->len >= node->size) {
                const size_t size = node->size * 2;
                struct child *new = realloc(node->children, sizeof(*new) * size);
                if (new == NULL)
                        die();
                node->size = size;
                node->children = new;
        }
        node->children[node->len++].node = child;
        return node;
}

void node_delete(Node *node)
{
        if (node == NULL)
                return;
        for (size_t i = 0; i < node->len; i++)
                node_delete(node->children[i].node);
        free(node->children);
        free(node);
}

static size_t skip(const char *s, size_t i)
{
        while (s[i] != '\0' && isspace(s[i]))
                i++;
        return i;
}

static Node *num(const char *s, size_t *i)
{
        size_t j = skip(s, *i);
        char ch;
        int n = 0;
        while ((ch = s[j]) != '\0' && ch >= '0' && ch <= '9') {
                n *= 10;
                n += ch - '0';
                j++;
        }
        if (j == *i)
                return NULL;
        Node *node = node_new(NUM);
        node->num = n;
        *i = skip(s, j);
        return node;
}

static Node *unary_expr2(const char *s, size_t *i)
{
        *i = skip(s, *i);
        size_t j = *i;
        Node *child = num(s, &j);
        if (child != NULL) {
                *i = j;
                return child;
        }
        char ch = s[j];
        if (ch != '(') {
                fprintf(stderr, "expected num at %zu[%c]\n", j, s[j]);
                goto err;
        }
        j++;
        if ((child = expr(s, &j)) == NULL) {
                fprintf(stderr, "expected expr at %zu[%c]\n", j, s[j]);
                goto err;
        }
        if (s[j] != ')') {
                fprintf(stderr, "expected ) at %zu[%c]\n", j, s[j]);
                goto err;
        }
        *i = skip(s, j + 1);
        return child;
err:
        node_delete(child);
        return NULL;
}

static Node *unary_expr(const char *s, size_t *i)
{
        *i = skip(s, *i);
        size_t j = *i;
        Node *node = node_new(UNARY_EXPR);
        Node *operator = NULL;
        Node *child = NULL;
        char ch = s[j];
        if (ch == '+' || ch == '-') {
                operator = node_new(UNARY_OPR);
                operator->num = (ch == '+') ? 1 : -1;
                j++;
        }
        if ((child = unary_expr2(s, &j)) == NULL)
                goto err;
        *i = j;
        return node_add(node, (operator == NULL) ? child : node_add(operator, child));
err:
        node_delete(child);
        node_delete(operator);
        node_delete(node);
        return NULL;
}

static Node *mul_expr(const char *s, size_t *i)
{
        *i = skip(s, *i);
        size_t j = *i;
        Node *node = node_new(MUL_EXPR);
        Node *child = unary_expr(s, &j);
        if (child == NULL)
                goto err;
        node_add(node, child);
        char op;
        while ((op = s[j]) == '*' || op == '/') {
                j++;
                node->children[node->len - 1].op = op;
                if ((child = unary_expr(s, &j)) == NULL)
                        goto err;
                node_add(node, child);
        }
        *i = j;
        return node;
err:
        node_delete(node);
        return NULL;
}

static Node *expr(const char *s, size_t *i)
{
        size_t j = *i;
        Node *node = node = node_new(EXPR);
        Node *child = mul_expr(s, &j);
        if (child == NULL)
                goto err;
        node_add(node, child);
        char op;
        while ((op = s[j]) == '+' || op == '-') {
                j++;
                node->children[node->len - 1].op = op;
                if ((child = mul_expr(s, &j)) == NULL)
                        goto err;
                node_add(node, child);
        }
        *i = j;
        return node;
err:
        node_delete(node);
        return NULL;
}

Node *parse(const char *s)
{
        size_t i = 0;
        Node *node = expr(s, &i);
        if (s[i] == '\0')
                return node;
        fprintf(stderr, "expected +, -, *, or / at %zu[%c]\n", i, s[i]);
        node_delete(node);
        return NULL;
}

int calc(const Node *node)
{
        switch (node->type) {
                int res;
        case EXPR:
        case MUL_EXPR:
                res = calc(node->children[0].node);
                for (size_t i = 1; i < node->len; i++) {
                        switch (node->children[i - 1].op) {
                        case '+':
                                res += calc(node->children[i].node);
                                break;
                        case '-':
                                res -= calc(node->children[i].node);
                                break;
                        case '*':
                                res *= calc(node->children[i].node);
                                break;
                        case '/':
                                res /= calc(node->children[i].node);
                                break;
                        }
                }
                return res;
        case UNARY_EXPR:
                return calc(node->children[0].node);
        case UNARY_OPR:
                return node->num * calc(node->children[0].node);
        case NUM:
                return node->num;
        }
}
