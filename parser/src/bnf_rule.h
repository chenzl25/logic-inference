#ifndef BNF_RULE_H
#define BNF_RULE_H
#include <string>
#include <vector>
#include <set>

#define EPS_STRING ("ε")
#define EOF_STRING ("$")

using namespace std;


class BnfRule {
public:
  class Symbol {
  public:
    bool is_terminal;
    string type;
    string value; // a string stand for terminal or non-terminal
    Symbol();
    Symbol(const Symbol &another);
    Symbol(bool is_t, string v, string t);
    Symbol& operator = (const Symbol &another);
    friend bool operator < (const BnfRule::Symbol& one, const BnfRule::Symbol& another);
    friend bool operator == (const BnfRule::Symbol& one, const BnfRule::Symbol& another);
  };

  BnfRule::Symbol head;
  vector<BnfRule::Symbol> body;
  string action_string;
};

#endif