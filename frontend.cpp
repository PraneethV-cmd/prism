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
