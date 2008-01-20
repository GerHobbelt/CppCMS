/* Copyright (c) 2008 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Red-black_tree_(C)?action=history&offset=20071121153556

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Red-black_tree_(C)?oldid=11693
*/

#include "rbtree.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> /* rand() */
#include <map>

static int compare_int(void* left, void* right);
static void print_tree(rbtree t);
static void print_tree_helper(rbtree_node n, int indent);

int compare_int(void* leftp, void* rightp) {
    long long int left = (long long int)leftp;
    long long int right = (long long int)rightp;
    if (left < right) 
        return -1;
    else if (left > right)
        return 1;
    else {
        assert (left == right);
        return 0;
    }
}

#define INDENT_STEP  4

void print_tree_helper(rbtree_node n, int indent);

void print_tree(rbtree t) {
    print_tree_helper(t->root, 0);
    puts("");
}

void print_tree_helper(rbtree_node n, int indent) {
    int i;
    if (n == NULL) {
        fputs("<empty tree>", stdout);
        return;
    }
    if (n->right != NULL) {
        print_tree_helper(n->right, indent + INDENT_STEP);
    }
    for(i=0; i<indent; i++)
        fputs(" ", stdout);
    if (n->color == BLACK)
        printf("%ld\n", (long )n->key);
    else
        printf("<%ld>\n", (long)n->key);
    if (n->left != NULL) {
        print_tree_helper(n->left, indent + INDENT_STEP);
    }
}

int main() {
    int i;
    #ifdef USE_RBTREE
	    rbtree t = rbtree_create();
	//    print_tree(t);
    #else
	    std::map<long long,long long> data;
    #endif

    for(i=0; i<500000; i++) {
        int x = rand() % 100000;
        int y = rand() % 100000;
#ifdef TRACE
        print_tree(t);
        printf("Inserting %d -> %d\n\n", x, y);
#endif
	#ifdef USE_RBTREE
        	rbtree_insert(t, (void*)x, (void*)y, compare_int);
	        assert(rbtree_lookup(t, (void*)x, compare_int) == (void*)y);
	#else
		data[x]=y;
		assert(data[x]==y);
	#endif
    }
    for(i=0; i<600000; i++) {
        int x = rand() % 100000;
#ifdef TRACE
        print_tree(t);
        printf("Deleting key %d\n\n", x);
#endif
	#ifdef USE_RBTREE
	        rbtree_delete(t, (void*)x, compare_int);
	#else
		data.erase(x);
	#endif

    }
    return 0;
}

