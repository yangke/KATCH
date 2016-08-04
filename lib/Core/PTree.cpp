//===-- PTree.cpp ---------------------------------------------------------===//
//
//                     The KLEE Symbolic Virtual Machine
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PTree.h"

#include <klee/Expr.h>
#include <klee/util/ExprPPrinter.h>

#include <vector>
#include <algorithm>
#include <iostream>

using namespace klee;

  /* *** */

PTree::PTree(const data_type &_root) : root(new Node(0,_root)) {
}

PTree::~PTree() {}

std::pair<PTreeNode*, PTreeNode*>
PTree::split(Node *n, 
             const data_type &leftData, 
             const data_type &rightData) {
  assert(n && n->type == PTreeNode::LEAF);
  n->type = PTreeNode::TWO;
  n->left = new Node(n, leftData);
  n->right = new Node(n, rightData);
  return std::make_pair(n->left, n->right);
}

std::vector<PTreeNode*>&
PTree::splitN(Node *n,
       const std::vector<data_type>& data) {
  assert(n && n->type == PTreeNode::LEAF);
  assert(data.size());

  n->children.reserve(data.size());
  n->type = PTreeNode::MULTIPLE;
  std::vector<ExecutionState*>::const_iterator it, ite;
  for (it = data.begin(), ite = data.end(); it != ite; ++it) {
    Node* nn = new Node(n, *it);
    n->children.push_back(nn);
  }
  return n->children;
}

void PTree::remove(Node *n) {
  assert(n->type == PTreeNode::LEAF && !n->infeasibles);
  do {
    Node *p = n->parent;
    delete n;
    if (p) {
      if (p->type == PTreeNode::TWO) {
        if (n == p->left) {
          p->left = 0;
        } else {
          assert(n == p->right);
          p->right = 0;
        }
      } else {
        assert(p->type == PTreeNode::MULTIPLE);
        std::vector<Node*>::iterator rem =
          std::find(p->children.begin(), p->children.end(), n);
        assert(rem != p->children.end());
        p->children.erase(rem);
      }
    }
    n = p;
  } while (n && !n->infeasibles && (
            (n->type == PTreeNode::TWO && !n->left && !n->right) ||
            (n->type == PTreeNode::MULTIPLE && !n->children.size())
          ) );
}

void PTree::dump(std::ostream &os) {
  ExprPPrinter *pp = ExprPPrinter::create(os);
  pp->setNewline("\\l");
  os << "digraph G {\n";
  os << "\tsize=\"10,7.5\";\n";
  os << "\tratio=fill;\n";
  os << "\trotate=90;\n";
  os << "\tcenter = \"true\";\n";
  os << "\tnode [style=\"filled\",width=.1,height=.1,fontname=\"Terminus\"]\n";
  os << "\tedge [arrowsize=.3]\n";
  std::vector<PTree::Node*> stack;
  stack.push_back(root);
  while (!stack.empty()) {
    PTree::Node *n = stack.back();
    stack.pop_back();
    if (n->condition.isNull()) {
      os << "\tn" << n << " [label=\"\"";
    } else {
      os << "\tn" << n << " [label=\"";
      pp->print(n->condition);
      os << "\",shape=diamond";
    }
    if (n->data)
      os << ",fillcolor=green";
    os << "];\n";
    if (n->left) {
      os << "\tn" << n << " -> n" << n->left << ";\n";
      stack.push_back(n->left);
    }
    if (n->right) {
      os << "\tn" << n << " -> n" << n->right << ";\n";
      stack.push_back(n->right);
    }
  }
  os << "}\n";
  delete pp;
}

PTreeNode::PTreeNode(PTreeNode *_parent, 
                     ExecutionState *_data) 
  : type(LEAF),
    parent(_parent),
    left(0),
    right(0),
    data(_data),
    condition(0),
    infeasibles(0) {
}

PTreeNode::~PTreeNode() {
}

void PTreeNode::addRef() {
  ++infeasibles;
}

void PTreeNode::release() {
  assert(infeasibles > 0);
  --infeasibles;
  //TODO: remove if last reference
}

