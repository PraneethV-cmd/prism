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
	static int LastChar = ' ';

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







