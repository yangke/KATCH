//===-- PTree.h -------------------------------------------------*- C++ -*-===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __UTIL_PTREE_H__
#define __UTIL_PTREE_H__

#include <klee/Expr.h>

#include <utility>
#include <cassert>
#include <vector>
#include <iostream>

namespace klee {
  class ExecutionState;

  class PTree { 
    typedef ExecutionState* data_type;

  public:
    typedef class PTreeNode Node;
    Node *root;

    PTree(const data_type &_root);
    ~PTree();
    
    std::pair<Node*,Node*> split(Node *n,
                                 const data_type &leftData,
                                 const data_type &rightData);
    std::vector<Node*>& splitN(Node *n,
                              const std::vector<data_type>& data);
    void remove(Node *n);

    void dump(std::ostream &os);
  };
/*
  class PTreeRootIterator {
  public:
    typedef class PTreeNode Node;
    Node *node;
*/


  class PTreeNode {
    friend class PTree;
  public:
    enum Type { LEAF, TWO, MULTIPLE };
    Type type;
    PTreeNode *parent, *left, *right;
    ExecutionState *data;
    ref<Expr> condition;

    // a switch may introduce more than 2 children
    std::vector<PTreeNode*> children;
    void addRef();
    void release();
  private:
    unsigned infeasibles;
    PTreeNode(PTreeNode *_parent, ExecutionState *_data);
    ~PTreeNode();
  };
}

#endif
