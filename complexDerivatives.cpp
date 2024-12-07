
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
    TENDOFFILE,

    Tunspecified
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

enum NodeType {
    binaryExpr,
    funcCall,
    variable,
    constant
};

struct Node {
    NodeType type;
    
    TokenType tokType;

    double value;

    Node* a;
    Node* b;
};

class Parser {
private:
    vector<Token> toks;
    size_t pos;

    Node* expr() {
        Node* a = term();
        while (toks[pos].type == TokenType::Tplus ||
            toks[pos].type == TokenType::Tminus) {
            TokenType tokType = toks[pos].type;
            ++pos;
            Node* b = term();
            a = new Node{ NodeType::binaryExpr, tokType, 0, a, b }; 
        }
        return a;
    }

    Node* term() {
        Node* a = factor();
        while (toks[pos].type == TokenType::Tmult ||
            toks[pos].type == TokenType::Tdiv) {
            TokenType tokType = toks[pos].type;
            ++pos;
            Node* b = factor();
            a = new Node{ NodeType::binaryExpr, tokType, 0, a, b };
        }
        return a;
    }

    Node* factor() {
        Node* a = basic();
        while (toks[pos].type == TokenType::Tpow) {
            TokenType tokType = toks[pos].type;
            ++pos;
            Node* b = basic();
            a = new Node{ NodeType::binaryExpr, tokType, 0, a, b };
        }
        return a;
    }

    Node* func_call() {
        if (!((toks[pos].type == TokenType::Tsin) ||
            (toks[pos].type == TokenType::Tcos) ||
            (toks[pos].type == TokenType::Ttan) ||
            (toks[pos].type == TokenType::Tcot) ||
            (toks[pos].type == TokenType::Tlog))) {
            throw "Parser error: expected function identifier";
        }
        TokenType funcType = toks[pos].type;
        ++pos;
        if (toks[pos].type != TokenType::TlParen) throw "Parser error: expected '(' after function identifier";
        ++pos;
        Node* arg = expr();
        if (toks[pos].type != TokenType::TrParen) throw "Parser error: expected ')' after function argument";
        ++pos;
        return new Node{ NodeType::funcCall, funcType , 0, arg, nullptr };
    }

    Node* basic() {
        if (toks[pos].type == TokenType::Tconstant) {
            Node* ret = new Node{ NodeType::constant, TokenType::Tconstant, toks[pos].value, nullptr, nullptr };
            ++pos;
            return ret;
        }
        if (toks[pos].type == TokenType::Tvariable) {
            Node* ret = new Node{ NodeType::variable, TokenType::Tvariable, 0, nullptr, nullptr};
            ++pos;
            return ret;
        }
        if ((toks[pos].type == TokenType::Tsin) ||
            (toks[pos].type == TokenType::Tcos) ||
            (toks[pos].type == TokenType::Ttan) ||
            (toks[pos].type == TokenType::Tcot) ||
            (toks[pos].type == TokenType::Tlog)) {
            return func_call();
        }
        if (toks[pos].type == TokenType::TlParen) {
            ++pos;
            Node* ret = expr();
            if (toks[pos].type != TokenType::TrParen) throw "Parser error: expected ')' after '('";
            ++pos;
            return ret;
        }
        throw "Parser error: unexpected token";
    }

public:
    Parser(const vector<Token>& tokens) : toks(tokens), pos(0) {}

    Node* parse() {
        return expr();
    }
};

// For testing

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

string tokenTypeToStr(TokenType t) {
    switch (t) {
    case TokenType::Tconstant:
        return "constant";
    case TokenType::Tvariable:
        return "variable";
    case TokenType::Tsin:
        return "sin";
    case TokenType::Tcos:
        return "cos";
    case TokenType::Ttan:
        return "tan";
    case TokenType::Tcot:
        return "cot";
    case TokenType::Tlog:
        return "log";
    case TokenType::Tplus:
        return "plus";
    case TokenType::Tminus:
        return "minus";
    case TokenType::Tmult:
        return "mult";
    case TokenType::Tdiv:
        return "div";
    case TokenType::Tpow:
        return "pow";
    case TokenType::TlParen:
        return "lParen";
    case TokenType::TrParen:
        return "rParen";
    case TokenType::TENDOFFILE:
        return "ENDOFFILE";
    default:
        throw "Unknown Token";
    }
}

string tokenVecToString(const vector<Token> tokens) {
    string ret = "";
    for (const Token& tok : tokens) {
        ret += tokenTypeToStr(tok.type);
        if (tok.type == TokenType::Tconstant) {
            ret += " " + double_to_str(tok.value);
        }
        ret += "\n";
    }
    return ret;
}

string parseTreeToString(const Node* root) {
    switch (root->type) {
    case NodeType::variable:
        return "variable";
    case NodeType::constant:
        return double_to_str(root->value);
    case NodeType::funcCall:
        return tokenTypeToStr(root->tokType) + "(" + parseTreeToString(root->a) + ")";
    case NodeType::binaryExpr:
        return "{" + parseTreeToString(root->a) + tokenTypeToStr(root->tokType) + parseTreeToString(root->b) + "}";
    default:
        throw "Unknown Node";
    }
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
            tokenVecToString(Lexer("421").lex()),
            "constant 421\nENDOFFILE\n",
            "integer number");
        s.isEq(
            tokenVecToString(Lexer("301.875").lex()),
            "constant 301.875\nENDOFFILE\n",
            "rational number");
        s.isEq(
            tokenVecToString(Lexer("sin cos + tan cot 10 ^ log x").lex()),
            "sin\ncos\nplus\ntan\ncot\nconstant 10\npow\nlog\nvariable\nENDOFFILE\n",
            "functions with variables and contants");
        s.isEq(
            tokenVecToString(Lexer("7+ cos(2 + x) ^2* 3 - 5.234").lex()),
            "constant 7\nplus\ncos\nlParen\nconstant 2\nplus\nvariable\nrParen\npow\nconstant 2\nmult\nconstant 3\nminus\nconstant 5.234\nENDOFFILE\n",
            "full math expr");

        bool error = false;
        try {
            tokenVecToString(Lexer("2 + e").lex());
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
            tokenVecToString(Lexer("2 + e").lex()),
            "illegal char");
        s.isError(
            tokenVecToString(Lexer("sincos 2").lex()),
            "no space between functions");*/
    }

    {
        //cout << parseTreeToString(Parser(Lexer("3 + 3 -1").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("5 + 32 * 2").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("4* (3+11)").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("sin(4 * x + 2)").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("4+tan(4 * x + log(x))").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("4+tan(4 * x + log(x)").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("sin 3 + 4").lex()).parse()) << endl; // should throw error
        cout << parseTreeToString(Parser(Lexer("sin(cos(tan(cot(log(x + 2)))))").lex()).parse()) << endl; // should throw error
    }

    return 0;
}