#include <iostream>
#include <string>
#include <stdlib.h>
#include "util.h"
#include "re_parse.h"
#include "digraph.h"
using namespace std;
//*********************************
// regular expression like that
// (), |, `concatenate`, , *, .
// '.' stand for arbitrary symbol
//*********************************

#define EOF -2  //detect the end of the input string

struct TreeNode;

static string input_string; // the global input_string
static int cur_pos;         // current position of input_string  
static char cur_char, lookahead;       // input_string[cur_pos]

TreeNode* parse_char();
TreeNode* parse_or();
TreeNode* parse_cat();
TreeNode* parse_star();
TreeNode* parse_re();
TreeNode* parse(string s);
void preprocess();
void move();
void match(char t);
void visit_print(TreeNode* root);

// the api for the other files
TreeNode* parse(string s) {
  input_string = s;
  preprocess();
  return parse_re();
}


//  Grammer
//  re   -> or
//  or   -> or '|' cat | cat
//  cat  -> cat star   | star
//  star -> char '*'   | char
//  char -> '(' re ')' | '.' | 'a' | ..... | 'z' | .....(other characters)

// Invariant:
// 1.before to parse we let cur_pos point to the start position
// 2.after the parse we let cur_pos point to the last after the end position 


TreeNode* parse_re() {
  // debug("parse_re");
  return parse_or();
}
TreeNode* parse_or() {
  // debug("parse_or");
  TreeNode* root = parse_cat();
  while (cur_char == '|') {
    move();
    root = new TreeNode(ENUM::TYPE_OR, NULL, root, parse_cat());
  }
  return root;
}
TreeNode* parse_cat() {
  // debug("parse_cat");
  TreeNode* root = parse_star();
  while (cur_char != EOF &&
         cur_char != '*' &&
         cur_char != ')' &&
         cur_char != '|') {
    root = new TreeNode(ENUM::TYPE_CAT, NULL, root, parse_star());
  }
  return root;
}

TreeNode* parse_star() {
  // debug("parse_star");
  TreeNode* root = parse_char();
  if (cur_char == '*' && lookahead == '*') {
    error("repetition-operator *");
    exit(-1);
  }
  if (cur_char == '*' && lookahead != '*') {
    move();
    root = new TreeNode(ENUM::TYPE_STAR, NULL, root, NULL);
  }
  return root;
}

TreeNode* parse_char() {
  // debug("parse_char");
  TreeNode* root = NULL;
  switch (cur_char) {
    case '(' :
      move();
      if (cur_char == ')') {
        match(')');
        return new TreeNode(ENUM::TYPE_CHAR, EPS, NULL, NULL);
      }
      root = parse_re();
      match(')');
      break;
    case '\\' : // escape case
      move();
      if (cur_char == '\\' ||
          cur_char == '('  ||
          cur_char == ')'  ||
          cur_char == '{'  ||
          cur_char == '}'  ||
          cur_char == '['  ||
          cur_char == ']'  ||
          cur_char == '*'  ||
          cur_char == '|'  ||
          cur_char == '^'  ||
          cur_char == '$'  ||
          cur_char == '-'  ||
          cur_char == '+'  ||
          cur_char == '.'  ||
          cur_char == ','  ||
          cur_char == '?'  ||
          cur_char == '\'' ||
          cur_char == '\"'  ) {
        root = new TreeNode(ENUM::TYPE_CHAR, cur_char, NULL, NULL);
        move();
      } else {
        error("escape error, only these: \\ ( ) { } [ ] * | ^ $ - + . , ? \' \" can be escaped");
        exit(-1);
      }
      break;
    case '.':
      root = new TreeNode(ENUM::TYPE_ANY, NULL, NULL, NULL);
      move();
      break;
    default:
      root = new TreeNode(ENUM::TYPE_CHAR, cur_char, NULL, NULL);
      move();
  }
  return root;
}



// move the cur_pos
void move() {
  cur_pos++;
  if (cur_pos < input_string.length()) cur_char = input_string[cur_pos];
  else cur_char = EOF;

  if (cur_pos+1 < input_string.length()) lookahead = input_string[cur_pos+1];
  else lookahead = EOF;
}
// assert to match the character
void match(char t) {
  if (cur_char != t) {
    error("syntax error, we expect " + t);
    exit(-1);
  }
  move();
}
void visit_delete(TreeNode* root) {
  if (root) {
    visit_delete(root->left);
    visit_delete(root->right);
    delete root;
  }
}
void visit_print(TreeNode* root) {
  if (root) {
    visit_print(root->left);
    visit_print(root->right);
    switch (root->type) {
      case ENUM::TYPE_CAT:
        cout << " -";
        break;
      case ENUM::TYPE_OR:
        cout << " |";
        break;
      case ENUM::TYPE_STAR:
        cout << " *";
        break;
      case ENUM::TYPE_CHAR:
        if (root->ch == EPS) cout << " " << "[e]";
        else cout << " "  << root->ch;
        break;
      case ENUM::TYPE_ANY:
        cout << " [any]";
        break;
      case ENUM::TYPE_AUGEND:
        cout << " [#]";
        break;
      default:
        break;
    }
  }
}
// all this function want to do is to conver [] {} to | () *
void preprocess() {
  debug("raw; " + input_string);
  string tem;
  // support [] {} + ? 
  if (input_string.length() > 0) {
    assert(input_string[0] != '*' &&
           input_string[0] != '{' &&
           input_string[0] != '+' &&
           input_string[0] != '?' ,
           "* { + ?  can't not be the first char");
  }
  for (int i = 0; i < input_string.length();) {
    if (input_string[i] == '\\') { // support \d \w \s
      if (i+1 < input_string.length()) {
        switch(input_string[i+1]) {
          case 'd':
            input_string.erase(i, 2);
            input_string.insert(i,"[0-9]");
            break;
          case 'w':
            input_string.erase(i, 2);
            input_string.insert(i,"[a-zA-Z0-9]");
            break;
          case 's':
            input_string.erase(i, 2);
            input_string.insert(i,"[ \t]");
            break;
          default:
            break; 
        continue;
        }
      } else {
        // do nothing
      }
    }
    if (input_string[i] == '+') {
      // for escape
      if (input_string[i-1] == '\\') {
        tem += input_string[i];
        i++;
        continue;
      }
      string plus_one;
      if (tem.length()-2 >= 0 && tem[tem.length()] == '\\') {
        plus_one = tem[tem.length()-2] + tem[tem.length()-1];
      } else {
        if (tem[tem.length()-1] == ')') {
          int num_rp = 1;
          for (int j = tem.length()-2; j >= 0; j--) {
            if (tem[j] == ')' && j-1>=0 && tem[j-1] != '\\') num_rp++;
            if (((tem[j] == '(' && j==0)) || (tem[j] == '(' && j-1>=0 && tem[j-1] != '\\')) num_rp--;
            if (num_rp == 0) {
              plus_one = tem.substr(j, tem.length() - j);
              break;
            }
          }
          assert(num_rp == 0, "the () should be balanced");
        } else {
          // the simple case like: asd{5,10}  repeat the d 5 to 10 times
          plus_one = tem[tem.length()-1];
        }
      }
      tem += plus_one + '*';
      i++;
      continue;
    }
    if (input_string[i] == '?') {
      // for escape
      if (input_string[i-1] == '\\') {
        tem += input_string[i];
        i++;
        continue;
      }
      input_string.erase(i, 1);
      input_string.insert(i, "{0,1}");
      continue;
    }
    if (input_string[i] == '[') {
      // for escape
      if (i-1>=0 && input_string[i-1] == '\\') {
        tem += input_string[i];
        i++;
        continue;
      }
      tem += '(';
      i++;
      while(input_string[i] != ']') { // support:[a-zA-Z0-9],[abcd]  don't support:[^asd]
        assert(i+1 < input_string.length(), "the escape can not be the end");
        if (isdigit(input_string[i]) || is_uppercase_letter(input_string[i]) || is_lowercase_letter(input_string[i])) {
          if (input_string[i+1] == '-') {
            assert(i+2 < input_string.length(), "- should not be the end");
            if (isdigit(input_string[i])) assert(isdigit(input_string[i+2]),  "between the - should be same");
            if (is_lowercase_letter(input_string[i])) assert(is_lowercase_letter(input_string[i+2]),  "between the - should be same type");
            if (is_uppercase_letter(input_string[i])) assert(is_uppercase_letter(input_string[i+2]),  "between the - should be same type");
            assert(input_string[i+2] >= input_string[i], "invalid character range");
            for (int j = 0; j <= input_string[i+2] - input_string[i]; j++) {
              tem += char(input_string[i] + j);
              tem += '|';
            }
            i += 3;
            continue;
          } else {
            /* do nothing deal with latter*/
          }
        }
        if (input_string[i] == '\\') {
          if (input_string[i+1] == 'd') {
            input_string.erase(i, 2);
            input_string.insert(i, "0123456789");
          } else if (input_string[i+1] == 'w') {
            input_string.erase(i, 2);
            input_string.insert(i, "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
          } else if (input_string[i+1] == 's') {
            input_string.erase(i, 2);
            input_string.insert(i, " \t");
          } else if  (input_string[i+1] == '\\' ||
                      input_string[i+1] == '('  ||
                      input_string[i+1] == ')'  ||
                      input_string[i+1] == '{'  ||
                      input_string[i+1] == '}'  ||
                      input_string[i+1] == '*'  ||
                      input_string[i+1] == '|'  ||
                      input_string[i+1] == '^'  ||
                      input_string[i+1] == '$'  ||
                      input_string[i+1] == '-'  ||
                      input_string[i+1] == '+'  ||
                      input_string[i+1] == '.'  ||
                      input_string[i+1] == ','  ||
                      input_string[i+1] == '?'  ||
                      input_string[i+1] == '\'' ||
                      input_string[i+1] == '\"' ) {
            /*only i++ here we deal with later*/
            i++; // skip '\\'
          } else {
            error("escape error, only these: d, w, s, \\ ( ) { } * | ^ $ - + . , ?, \' \" can be escaped in [ ]");
          }
        } // end the while not ]
        tem += input_string[i];
        tem += '|';
        i++;
      } // end while (input_string[i] != ']')
      assert(tem[tem.length()-1] == '|', "the last pos in the tem must be '|' ");
      tem[tem.length()-1] = ')';
      i++; // skip the ]
      continue;
    } // end if [
    if (input_string[i] == '{') {
      // for escape
      if (i-1>=0 && input_string[i-1] == '\\') {
        tem += input_string[i];
        i++;
        continue;
      }
      int l = 0,r = 0;
      int lb_pos = i, rb_pos; // pos of '{' and '}'
      string repeat_one;
      i++; // skip the '{'
      while (isdigit(input_string[i])){
       l = l*10 + (int)(input_string[i]-'0');
       i++;
      }
      bool r_infinite = false;
      if (input_string[i] == ',' && i+1 < input_string.length() && input_string[i+1] == '}') {
        r_infinite = true;
        i++;// skip the ','
      }
      if (input_string[i] == ',' && i+1 < input_string.length() && input_string[i+1] != '}') {
        i++; // skip the ','
        while (isdigit(input_string[i])){
         r = r*10 + (int)(input_string[i]-'0');
         i++;
        }
        assert(r >= l, "{l,r} r should >= l");
      }
      assert(input_string[i] == '}', "{} should be balanced");
      rb_pos = i;
      if (tem.length()-2 >= 0 && tem[tem.length()] == '\\') {
        repeat_one = tem[tem.length()-2] + tem[tem.length()-1];
      } else {
        if (tem[tem.length()-1] == ')') {
          int num_rp = 1;
          for (int j = tem.length()-2; j >= 0; j--) {
            if (tem[j] == ')' && j-1>=0 && tem[j-1] != '\\') num_rp++;
            if (((tem[j] == '(' && j==0)) || (tem[j] == '(' && j-1>=0 && tem[j-1] != '\\')) num_rp--;
            if (num_rp == 0) {
              repeat_one = tem.substr(j, tem.length() - j);
              break;
            }
          }
          assert(num_rp == 0, "the () should be balanced");
        } else {
          // the simple case like: asd{5,10}  repeat the d 5 to 10 times
          repeat_one = tem[tem.length()-1];
        }
      }
      if (!r_infinite) {
        tem = tem.substr(0, tem.length() - repeat_one.length()); // go back to the repeat_one += before enter '{'
        if ((r == 0 && l > 0) || r == l) {
          int count = l;
          while(count--) {
            tem += repeat_one;
          }
        }
        // for exmaple the (aa){3,5} => (aa)(aa)((aa)|(aa)(aa)|(aa)(aa)(aa))
        if ((r != 0) && r != l) {
          int old_l = l;
          if (l == 0) {
            tem =  "((" + tem + ')' + "|((" + tem + ')';
          }
          if (l > 0) {
            l--;
            int count = l;
            if (count > 0) {
              while(count--) {
                tem += repeat_one;
              }
            }
          }
          tem += '(';
          for (int j = 1; j <= r - l; j++) {
            int count = j;
            while (count--) {
              tem += repeat_one;
            }
            if (j != r-l) tem += '|';
            else tem += ')';
          }
          if (old_l == 0) tem += "))";
        }
      } // end if (!r_infinite)
      if (r_infinite) {
        int count = l;
        while(count--) {
          tem += repeat_one;
        }
        tem += '*';
      }
      i = rb_pos + 1;
      continue;
    } // end if '{'
    tem += input_string[i];
    i++;
  }
  input_string = tem;
  debug("after preprocess: "+ input_string);
  // prepare to parse. Before to parse we let the cur_pos point to the start position
  cur_pos = -1;
  move();
}

