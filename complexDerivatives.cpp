
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
    /*const vector<string> functionNames{
        "sin", "cos", "tan", "cot", "log"
    };*/

    string eq;
    size_t pos;

    static bool isDigit(char ch) {
        return ch >= '0' && ch <= '9';
    }

    static bool isAlpha(char ch) {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    double makeNum() {
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
                break;
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

    string makeFunction() {
        string functionType = "";
        while (isAlpha(eq[pos])) {
            functionType += eq[pos];
            ++pos;
        }
        return functionType;
    }

public:
    Lexer(const string& eq) : eq(eq), pos(0) {}

    vector<Token> lex() {
        vector<Token> res;
        while (eq[pos] != '\0') {
            if (isDigit(eq[pos])) {
                res.push_back(Token{ TokenType::Tconstant, makeNum() });
            } else if (eq[pos] == VARIABLE) {
                res.push_back(Token{ TokenType::Tvariable, 0 });
                ++pos;
            } else {
                switch (eq[pos]) {
                case '+':
                    res.push_back(Token{ TokenType::Tplus, 0 });
                    ++pos; break;
                case '-':
                    res.push_back(Token{ TokenType::Tminus, 0 });
                    ++pos; break;
                case '*':
                    res.push_back(Token{ TokenType::Tmult, 0 });
                    ++pos; break;
                case '/':
                    res.push_back(Token{ TokenType::Tdiv, 0 });
                    ++pos; break;
                case '^':
                    res.push_back(Token{ TokenType::Tpow, 0 });
                    ++pos; break;
                case '(':
                    res.push_back(Token{ TokenType::TlParen, 0 });
                    ++pos; break;
                case ')':
                    res.push_back(Token{ TokenType::TrParen, 0 });
                    ++pos; break;
                case' ':
                    ++pos;
                    // just ignore space
                    break;
                default:
                    string functionType = makeFunction();
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
                    } else if (functionType != "") {
                        throw "Lexing error: unknown character";
                    }
                }
            } 
        }
        res.push_back(Token{ TokenType::TENDOFFILE, 0 });
        return res;
    }
};

string double_to_str(double d) {
    if (floor(d) == d) {
        return to_string((int)d);
    }
    string res = to_string(d);
    while (res.back() == '0') {
        res.pop_back();
    }
    return res;
}

string TokenVecToString(const vector<Token> tokens) {
    string result = "";
    for (const Token& tok : tokens) {
        switch (tok.type) {
        case TokenType::Tconstant:
            result += "constant";
            result += " " + double_to_str(tok.value);
            break;
        case TokenType::Tvariable:
            result += "variable";
            break;
        case TokenType::Tsin:
            result += "sin";
            break;
        case TokenType::Tcos:
            result += "cos";
            break;
        case TokenType::Ttan:
            result += "tan";
            break;
        case TokenType::Tcot:
            result += "cot";
            break;
        case TokenType::Tlog:
            result += "log";
            break;
        case TokenType::Tplus:
            result += "plus";
            break;
        case TokenType::Tminus:
            result += "minus";
            break;
        case TokenType::Tmult:
            result += "mult";
            break;
        case TokenType::Tdiv:
            result += "div";
            break;
        case TokenType::Tpow:
            result += "pow";
            break;
        case TokenType::TlParen:
            result += "lParen";
            break;
        case TokenType::TrParen:
            result += "rParen";
            break;
        case TokenType::TENDOFFILE:
            result += "ENDOFFILE";
            break;
        default:
            throw "Unknown token";
        }
        result += "\n";
    }
    return result;
}

template <typename T>
class TestSuit {
private:
    size_t testNumber;
public:
    TestSuit(const string& name, const string& description = "") : testNumber(0) {
        cout << "Tests about " << name << endl;
        if (description != "") {
            cout << "description: " << description << endl;
        }
    }
    void isEq(const T& given, const T& expected, const string& description = "") {
        cout << "Test (" << description << ") " << testNumber << ": ";
        if (given == expected) {
            cout << "correct" << endl;
        } else {
            cout << "incorrect:" << endl;
            cout << "got: " << given << endl;
            cout << "expected: " << expected << endl;
        }
    }

    void isError(const T& given, const string& description = "") {
        cout << "Test (" << description << ") " << testNumber << ": ";
    }
};

int main() {

    {
        TestSuit<string> s("Lexer");

        s.isEq(
            TokenVecToString(Lexer("421").lex()),
            "constant 421\nENDOFFILE\n",
            "integer number");
        s.isEq(
            TokenVecToString(Lexer("301.875").lex()),
            "constant 301.875\nENDOFFILE\n",
            "rational number");
        s.isEq(
            TokenVecToString(Lexer("sin cos + tan cot 10 ^ log x").lex()),
            "sin\ncos\nplus\ntan\ncot\nconstant 10\npow\nlog\nvariable\nENDOFFILE\n",
            "functions with variables and contants");
        s.isEq(
            TokenVecToString(Lexer("7+ cos(2 + x) ^2* 3 - 5.234").lex()),
            "constant 7\nplus\ncos\nlParen\nconstant 2\nplus\nvariable\nrParen\npow\nconstant 2\nmult\nconstant 3\nminus\nconstant 5.234\nENDOFFILE\n",
            "full math expr");

        bool error = false;
        try {
            TokenVecToString(Lexer("2 + e").lex());
        } catch (exception e) {
            cout << e.what();
            error = true;
        } catch (...) {
            error = true;
        }
        if (error) {
            cout << "good";
        } else {
            cout << "bad";
        }

        /*s.isError(
            TokenVecToString(Lexer("2 + e").lex()),
            "illegal char");
        s.isError(
            TokenVecToString(Lexer("sincos 2").lex()),
            "no space between functions");*/
    }

    return 0;
}