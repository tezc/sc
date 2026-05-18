/*
 * BSD-3-Clause
 *
 * Copyright 2025 Georges Troulis
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "sc_disjoint.h"

void
sc_disjoint_init( struct sc_disjoint_node* node, const void* data )
{
    node->parent = node;
    node->rank   = 0;
    node->size   = 1;
    node->data   = data;
}

struct sc_disjoint_node*
sc_disjoint_parent( struct sc_disjoint_node* node )
{
    struct sc_disjoint_node* root = node;

    while( root != root->parent )
        root = root->parent;

    // Optimization for find:  Update EACH parent to point to root
    while( node != node->parent ){
        struct sc_disjoint_node* p = node->parent;
        node->parent = root;
        node = p;
    }

    return root;
}

void
sc_disjoint_merge( struct sc_disjoint_node* a, struct sc_disjoint_node* b )
{
    struct sc_disjoint_node* pa = sc_disjoint_parent( a );
    struct sc_disjoint_node* pb = sc_disjoint_parent( b );

    if( pa == pb )
        return;

    if( pa->rank < pb->rank ){
        struct sc_disjoint_node* temp = pa;
        pa = pb;
        pb = temp;
    }
    pb->parent = pa;

    if( pa->rank == pb->rank ){
        pa->rank ++;
    }

    pa->size += pb->size;
}
