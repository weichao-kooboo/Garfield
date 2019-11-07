#pragma once

#ifndef _SP_RBTREE_H_INCLUDED_
#define _SP_RBTREE_H_INCLUDED_
#include "sp_core.h"

typedef sp_uint_t  sp_rbtree_key_t;
typedef sp_int_t   sp_rbtree_key_int_t;

typedef struct sp_rbtree_node_s  sp_rbtree_node_t;

struct sp_rbtree_node_s {
	sp_rbtree_key_t       key;
	sp_rbtree_node_t     *left;
	sp_rbtree_node_t     *right;
	sp_rbtree_node_t     *parent;
	u_char                 color;
	u_char                 data;
};


typedef struct sp_rbtree_s  sp_rbtree_t;

typedef void(*sp_rbtree_insert_pt) (sp_rbtree_node_t *root,
	sp_rbtree_node_t *node, sp_rbtree_node_t *sentinel);

struct sp_rbtree_s {
	sp_rbtree_node_t     *root;
	sp_rbtree_node_t     *sentinel;
	sp_rbtree_insert_pt   insert;
};


#define sp_rbtree_init(tree, s, i)                                           \
    sp_rbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
    (tree)->insert = i


void sp_rbtree_insert(sp_rbtree_t *tree, sp_rbtree_node_t *node);
void sp_rbtree_delete(sp_rbtree_t *tree, sp_rbtree_node_t *node);
void sp_rbtree_insert_value(sp_rbtree_node_t *root, sp_rbtree_node_t *node,
	sp_rbtree_node_t *sentinel);
void sp_rbtree_insert_timer_value(sp_rbtree_node_t *root,
	sp_rbtree_node_t *node, sp_rbtree_node_t *sentinel);
sp_rbtree_node_t *sp_rbtree_next(sp_rbtree_t *tree,
	sp_rbtree_node_t *node);


#define sp_rbt_red(node)               ((node)->color = 1)
#define sp_rbt_black(node)             ((node)->color = 0)
#define sp_rbt_is_red(node)            ((node)->color)
#define sp_rbt_is_black(node)          (!sp_rbt_is_red(node))
#define sp_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define sp_rbtree_sentinel_init(node)  sp_rbt_black(node)


static sp_inline sp_rbtree_node_t *
sp_rbtree_min(sp_rbtree_node_t *node, sp_rbtree_node_t *sentinel)
{
	while (node->left != sentinel) {
		node = node->left;
	}

	return node;
}
#endif // !_SP_RBTREE_H_INCLUDED_