
#include <iostream>

#include <functional>
#include <complex>
#include <tuple>
#include <string>
#include <vector>

using namespace std;

using value_t = complex<double>;
using func_t = function<value_t(value_t)>;

tuple<func_t, func_t, func_t> differentiate(const string& eq) {
    return {
        [](value_t) { return 0; },
        [](value_t) { return 0; },
        [](value_t) { return 0; }
    };
}

enum TokenType {
    Tconstant,
    Tvariable,
    Tsin, Tcos, Ttan, Tcot, Tlog,
    Tplus, Tminus, Tmult, Tdiv, Tpow,
    TlParen, TrParen,
    TENDOFFILE
};

struct Token {
    TokenType type;
    double value; // ints are converted to doubles too
};

const char VARIABLE = 'x';
const char DECIMALSEPARATOR = '.';

class Lexer {
private:
    const vector<string> functionNames {
    "sin", "cos", "tan", "cot", "log"
    };

    size_t maxFunctionNameLength = 0;

    string eq;
    size_t pos;

    bool isDigit(char ch) const {
        return ch >= '0' && ch <= '9';
    }

    double parseNum() {
        int integerPart = 0;
        double fractionPart = 0;

        bool isFraction = false;

        while (isDigit(eq[pos])) {
            integerPart *= 10;
            integerPart += eq[pos] - '0';

            ++pos;
            if (eq[pos] == DECIMALSEPARATOR) {
                ++pos;
                if (!isDigit(eq[pos])) throw "Lexing error: Decimal separator must be followed by digits";
                isFraction = true;
            }
        }

        if (isFraction) {
            double divisor = 1;
            while (isDigit(eq[pos])) {
                divisor /= 10;
                fractionPart += (eq[pos] - '0') * divisor;

                ++pos;
            }
        }

        return (double)integerPart + fractionPart;
    }

    bool isFunction(string& functionType) {
        switch (eq[pos]) {
        case 's':
            ++pos;
            if (eq[pos] == 'i') {
                ++pos;
                if (eq[pos] == 'n') {
                    functionType = "sin";
                    return true;
                } else {}
            }
            break;
        }


        for (size_t i = 0; i < maxFunctionNameLength; ++i) {
            bool foundFuncChar = false;
            for (string functionName : functionNames) {
                if (i >= functionName.length()) continue;
                if (eq[pos] == functionName[i]) {
                    foundFuncChar = true;
                }
            }
            if (foundFuncChar) {
                functionType += eq[pos];
                for (const string& functionName : functionNames) {
                    if (functionType == functionName) {
                        return true;
                    }
                }
            } else {
                return false;
            }
        }
        return false;
    }

public:
    Lexer(const string& eq) : eq(eq), pos(0) {
        for (const string& functionName : functionNames) {
            if (functionName.length() > maxFunctionNameLength) {
                maxFunctionNameLength = functionName.length();
            }
        }
    }

    vector<Token> lex() {
        vector<Token> res;
        while (eq[pos] != '\0') {
            if (isDigit(eq[pos])) {
                res.push_back(Token{ TokenType::Tconstant, parseNum() });
            } else if (eq[pos] == VARIABLE) {
                res.push_back(Token{ TokenType::Tvariable, 0 });
            } else {
                string functionType = "";
                if (isFunction(functionType)) {
                    if (functionType == "sin") {
                        res.push_back(Token{ TokenType::Tsin, 0 });
                    } else if (functionType == "cos") {
                        res.push_back(Token{ TokenType::Tcos, 0 });
                    } else if (functionType == "tan") {
                        res.push_back(Token{ TokenType::Ttan, 0 });
                    } else if (functionType == "cot") {
                        res.push_back(Token{ TokenType::Tcot, 0 });
                    } else if (functionType == "log") {
                        res.push_back(Token{ TokenType::Tlog, 0 });
                    } else {
                        throw "Lexing error: unknown function";
                    }
                } else {
                    switch (eq[pos]) {
                    case '+':
                        res.push_back(Token{ TokenType::Tplus, 0 });
                        break;
                    case '-':
                        res.push_back(Token{ TokenType::Tminus, 0 });
                        break;
                    case '*':
                        res.push_back(Token{ TokenType::Tmult, 0 });
                        break;
                    case '/':
                        res.push_back(Token{ TokenType::Tdiv, 0 });
                        break;
                    case '^':
                        res.push_back(Token{ TokenType::Tpow, 0 });
                        break;
                    case '(':
                        res.push_back(Token{ TokenType::TlParen, 0 });
                        break;
                    case ')':
                        res.push_back(Token{ TokenType::TrParen, 0 });
                        break;
                    case' ':
                        // just ignore space
                        break;
                    default:
                        throw "Lexing error: Unknown character";
                    }
                    ++pos;
                }
            } 
        }
        res.push_back(Token{ TokenType::TENDOFFILE, 0 });
        return res;
    }
};

int main() {
    Lexer myLexer("2+ 3 - 5.234 * cos(2 + x)");

    vector<Token> myTokens = myLexer.lex();
}