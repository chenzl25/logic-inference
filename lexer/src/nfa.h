#ifndef NFA_H
#define NFA_H
#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <set>
#include "re_parse.h"
#include "digraph.h"
using namespace std;

struct TreeNode;
class DFA;

class NFA {
public:
  friend class DFA;
  NFA(TreeNode* root);
  ~NFA();
  bool simulate(string txt);
  int match(string txt, string& result);
  bool match_from_head(string txt, string &result);
private:
  TreeNode* _root;
  Digraph::DNode* _start, *_end;
  void _re_tree_to_nfa();
  // notice the start and the end will be construct after call this function
  void _visit_construct_subnfa(TreeNode* tnode, Digraph::DNode* &start, Digraph::DNode* &end);
};
#endif
