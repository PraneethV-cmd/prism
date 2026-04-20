#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// Lexer

// here different notations as ASCII is from [0-255]
enum Token {
	tok_eof = -1,
	tok_def = -2,
	tok_extern = -3,
	tok_identifier = -4,
	tok_number = -5,
	tok_error = -6,
};

static std::string IdentifierStr; //value filled in if tok_identifier
static double numVal; //value filled in if tok_number

//gettok is a function that returns the next token fron the current standard input
static int gettok() {
	static int LastChar = ' '

	while (isspace(LastChar)) {
		LastChar = getchar();
	}

	if (isalpha(LastChar)) {
		IdentifierStr = LastChar;
		while(isalnum(LastChar = getchar())) {
			IdentifierStr += LastChar;
		}

		if (IdentifierStr == "def") {
			return tok_def;
		}

		if (IdentifierStr == "extern") {
			return tok_extern;
		}
		return tok_identifier;
	}

	if (isdigit(LastChar) || LastChar == '.') {
		std::string NumStr;
		bool hasDot = false;
		bool hasDigit = false;

		while (isdigit(LastChar) || LastChar == '.') {
			if (LastChar == '.') {
				if (hasDot) break;
				hasDot = true;
			} else {
				hasDigit = true;
			}

			NumStr += LastChar;
			LastChar = getchar();
		}

		if (!hasDigit) {
			return '.';
		}

		if (LastChar == 'e' || LastChar == 'E') {
			NumStr += LastChar;
			LastChar = getchar();

			if (LastChar == '+' || LastChar == '-') {
				NumStr += LastChar;
				LastChar = getchar();
			}

			if (!isdigit(LastChar)) {
				return tok_error;
			}

			while (isdigit(LastChar)) {
				NumStr += LastChar;
				LastChar = getchar();
			}
		}

		char* end;
		numVal = strtod(NumStr.c_str(), &end);

		if (*end != '\0') {
			return tok_error;
		}

		return tok_number;
	}

	if (LastChar == '#') {
		do {
			LastChar = getchar();
		}while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

		if (LastChar != EOF) {
			return gettok();
		}
	}

	int ThisChar = LastChar;
	LastChar = getchar();
	return ThisChar;
}

// ExprAST -  base class for all expression nodes

class ExprAST {
public:
	virtual ~ExprAST() = default; // this is a destructor, called when an object is destroyed to free memory, clase files, release resources, clean up owned objects
	// generally speaking, destructors are virutal becasue we need to delete derived objects through base class pointers, casuing memory leaks
};

// NumberExprAST - expression class for numerical literals like "1.0"

class NumberExprAST : public ExprAST {
	double Val;

public:
	NumberExprAST(double Val) : Val(Val) {}
};

class VariableExprAST : public ExprAST {
	std::string Name;
public:
	VariableExprAST(std::string Name) : Name(Name) {}
};

class BinaryExprAST : public ExprAST {
	char Op;
	std::unique_ptr<ExprAST> LHS, RHS;

public:
	BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

// CallExprAST - expresseion class for function calls

class CallExprAST : public ExprAST {
	std::string Callee;
	std::vector<std::unique_ptr<ExprAST>> Args;
public:
	CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(std::move(Args)) {}
};


// PrototypeAST -  class that represents prototype for a function,
// which captures
// - name
// - argument names (thus implicitly the number of arguments the function takes)

class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(const std::string &Name, std::vector<std::string> Args) : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }

};

//FunctionAST is for function definition itself
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

// Parser

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }

// CurTok/getNextToken -  its a simple token buffer where
// CurTok is the current token the parser is looking for, getNextToken reads another
// token from the lexer and updates CurTok with its result

// Binary Operators have their precendence, so we hold the precendence
// for each binary operator that is defined
static std::map<char, int> BinopPrecendence;

//now we need to get the precendence of the pending binary operator token,
static int GetTokPrecedence() {
  if (!isascii(CurTok)) {
    return -1;
  }

  int TokPrec = BinopPrecendence[CurTok];
  if (TokPrec <= 0) {
    return -1;
  }

  return TokPrec;
}

//LogError* is a helper function for error handling
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "error: %s\n", Str);
  return nullptr;
}

static std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

//numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken();
  return std::move(Result);
}

//parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); //we eat (.
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }

  if (CurTok != ')') {
    return LogError("expected ')'");
  }
  getNextToken(); // eat ).
  return V;
}

//identifierExpr ::= identifier | indentifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;

  getNextToken();

  if (CurTok != '(') return std::make_unique<VariableExprAST>(IdName);

  getNextToken();
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression()) Args.push_back(std::move(Arg));
      else return nullptr;

      if (CurTok == ')') break;
      if (CurTok != ',') return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  getNextToken();
  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

//primary ::= identifierExpr | numberexpr | parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
    default: return LogError("unknown token when expecting an expression");
    case tok_identifier: return ParseIdentifierExpr();
    case tok_number: return ParseNumberExpr();
    case '(': return ParseParenExpr();
  }
}

