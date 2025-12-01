#include <stdio.h>
#include <stdlib.h> // change this to <stdlib.h>
#include <ctype.h>

typedef struct node {
    enum {
        ADD,
        MULTI,
        VAL
    }   type;
    int val;
    struct node *l;
    struct node *r;
}   node;

node    *new_node(node n)
{
    node *ret = calloc(1, sizeof(n));
    if (!ret)
        return (NULL);
    *ret = n;
    return (ret);
}

void    destroy_tree(node *n)
{
    if (!n)
        return ;
    if (n->type != VAL)
    {
        destroy_tree(n->l);
        destroy_tree(n->r);
    }
    free(n);
}

void    unexpected(char c)
{
    if (c)
        printf("Unexpected token '%c'\n", c);
    else
        printf("Unexpected end of file\n");
}

int accept(char **s, char c)
{
    if (**s == c)
    {
        (*s)++;
        return (1);
    }
    return (0);
}

int expect(char **s, char c)
{
    if (accept(s, c))
        return (1);
    unexpected(**s);
    return (0);
}

node *parse_sum(char **s);

node *parse_union(char **s)
{
	if (**s >= '0' && **s <= '9')
	{
		node n = {VAL, **s - '0', NULL, NULL};
		(*s)++;
		return new_node(n);
	}
	else if(accept(s, '('))
	{
		node *lhs = parse_sum(s);
		if (!expect(s, ')'))
		{
			destroy_tree(lhs);
			return NULL;
		}
		return lhs;
	}
	unexpected(**s);
	return NULL;
}

node *parse_multi(char **s)
{
	node *lhs = parse_union(s);
	if (!lhs) return NULL;

	while(accept(s, '*'))
	{
		node *rhs = parse_union(s);
		if (!rhs)
		{
			destroy_tree(lhs);
			return NULL;
		}
		node n = {MULTI, 0, lhs, rhs};
		lhs = new_node(n);
	}
	return lhs;
}

node *parse_sum(char **s)
{
	node *lhs = parse_multi(s);
	if (!lhs) return NULL;

	while (accept(s, '+'))
	{
		node *rhs = parse_multi(s);
		if (!rhs)
		{
			destroy_tree(lhs);
			return NULL;
		}
		node n = {ADD, 0, lhs, rhs};
		lhs = new_node(n);
	}
	return lhs;
}

node    *parse_expr(char *s)
{
    node *ret = parse_sum(&s);
	if (!ret) return NULL;

    if (*s) 
    {
		unexpected(*s);
        destroy_tree(ret);
        return (NULL);
    }
    return (ret);
}

int eval_tree(node *tree)
{
    switch (tree->type)
    {
        case ADD:
            return (eval_tree(tree->l) + eval_tree(tree->r));
        case MULTI:
            return (eval_tree(tree->l) * eval_tree(tree->r));
        case VAL:
            return (tree->val);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return (1);
    node *tree = parse_expr(argv[1]);
    if (!tree)
        return (1);
    printf("%d\n", eval_tree(tree));
    destroy_tree(tree);
}