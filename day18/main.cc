#include <execution>
#include <fstream>
#include <numeric>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/strings/substitute.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "glog/logging.h"

enum TokenType {
  kNumber,
  kOperator,
  kOpenParen,
  kCloseParen,
};

enum Operator {
  kAdd,
  kMultiply,
};

struct Token {
  TokenType type;
  // If a number, hold the value.
  int64_t value = 0;
  // If an operator, hold the operator.
  Operator op;
};

typedef std::list<Token> Expression;

Expression ParseExpression(std::string line) {
  Expression tokens;
  for (int i = 0; i < line.size(); ++i) {
    char c = line[i];
    if (c == ' ') continue;
    if (c == '(') {
      tokens.push_back({kOpenParen});
    } else if (c == ')') {
      tokens.push_back({kCloseParen});
    } else if (c == '+') {
      tokens.push_back({kOperator, 0, kAdd});
    } else if (c == '*') {
      tokens.push_back({kOperator, 0, kMultiply});
    } else {
      // It's a number! Read it.
      int64_t number = c - '0';
      while (line[i + 1] >= '0' && line[i + 1] <= '9') {
        ++i;
        number *= 10;
        number += line[i] - '0';
      }
      tokens.push_back({kNumber, number});
    }
  }
  return tokens;
}

typedef absl::flat_hash_map<Operator, int> PrecedenceMap;

// Shunting-yard! Postfix (RPN) is easy to evaluate since precedence goes away.
Expression InfixToPostfix(const Expression& expression,
                          PrecedenceMap precedence) {
  Expression output;
  Expression operators;

  for (auto token : expression) {
    switch (token.type) {
      case kNumber:
        output.push_back(token);
        break;
      case kOperator:
        while (!operators.empty() && operators.back().type != kOpenParen &&
               (precedence[operators.back().op] >= precedence[token.op])) {
          output.push_back(operators.back());
          operators.pop_back();
        }
        operators.push_back(token);
        break;
      case kOpenParen:
        operators.push_back(token);
        break;
      case kCloseParen:
        CHECK(!operators.empty()) << "Misaligned parens.";
        while (operators.back().type != kOpenParen) {
          output.push_back(operators.back());
          operators.pop_back();
          CHECK(!operators.empty()) << "Misaligned parens.";
        }
        // Eat the open paren.
        CHECK(operators.back().type == kOpenParen);
        operators.pop_back();
        break;
    }
  }
  while (!operators.empty()) {
    output.push_back(operators.back());
    operators.pop_back();
  }

  return output;
}

std::string PostfixToString(Expression expression) {
  std::string s;
  for (auto token : expression) {
    switch (token.type) {
      case kNumber:
        s = absl::StrCat(s, " ", token.value);
        break;
      case kOperator:
        s = absl::StrCat(s, " ", token.op == kAdd ? "+" : "*");
        break;
      case kOpenParen:
      case kCloseParen:
        CHECK(false);
    }
  }
  return s;
}

int64_t EvaluatePostfix(Expression expression) {
  LOG(INFO) << PostfixToString(expression);
  std::stack<int64_t> values;
  while (!expression.empty()) {
    auto token = expression.front();
    expression.pop_front();

    switch (token.type) {
      case kNumber:
        values.push(token.value);
        break;
      case kOperator: {
        int64_t left = values.top();
        values.pop();
        int64_t right = values.top();
        values.pop();
        values.push(token.op == kAdd ? left + right : left * right);
        break;
      }
      case kOpenParen:
      case kCloseParen:
        CHECK(false) << "Unexpected token in postfix: " << token.type;
        return 0;
    }
  }
  CHECK(values.size() == 1);
  return values.top();
}

int64_t EvaluateAllWithPrecedence(std::list<Expression> expressions,
                                  PrecedenceMap precedence) {
  int64_t total = 0;
  for (auto expression : expressions) {
    auto value = EvaluatePostfix(InfixToPostfix(expression, precedence));
    LOG(INFO) << value;
    total += value;
  }
  return total;
}

int main(int argc, char** argv) {
  google::InstallFailureSignalHandler();
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = 1;

  std::ifstream file(argv[1]);
  CHECK(file);

  std::string line;
  std::list<Expression> expressions;
  while (std::getline(file, line)) {
    expressions.push_back(ParseExpression(line));
  }

  {
    // For Part 1: evalute each with equal precedence.
    PrecedenceMap precedence = {{kAdd, 1}, {kMultiply, 1}};
    int64_t total = EvaluateAllWithPrecedence(expressions, precedence);
    LOG(INFO) << "PART 1: " << total;
  }
  {
    // For Part 2: evalute with Add at higher precedence.
    PrecedenceMap precedence = {{kAdd, 2}, {kMultiply, 1}};
    int64_t total = EvaluateAllWithPrecedence(expressions, precedence);
    LOG(INFO) << "PART 2: " << total;
  }
  return 0;
}
