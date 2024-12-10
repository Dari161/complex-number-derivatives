
#include <iostream>

#include <functional>
#include <complex>
#include <tuple>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

using value_t = complex<double>;
using func_t = function<value_t(value_t)>;

// TODO: In the future more functions can be supported, like arcsin or cosecant

enum TokenType {
    Tconst,
    Tvariable,
    Tsin, Tcos, Ttan, Tcot, Tlog,
    Tplus, Tminus, Tmult, Tdiv, Tpow,
    TlParen, TrParen,
    TEND
};

struct Token {
    TokenType type; // only used for constants
    double value; // ints are converted to doubles too
};

const char VARIABLE = 'x';
const char DECIMALSEPARATOR = '.';

class Lexer {
private:

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
                if (!isDigit(eq[pos])) throw "Lexer error: Decimal separator must be followed by digits";
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

    int parenBalance = 0;

public:
    Lexer(const string& eq) : eq(eq), pos(0) {}

    vector<Token> lex() {
        vector<Token> res;
        while (eq[pos] != '\0') {
            if (isDigit(eq[pos])) {
                res.push_back(Token{ TokenType::Tconst, makeNum() });
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
                    ++parenBalance;
                    ++pos; break;
                case ')':
                    res.push_back(Token{ TokenType::TrParen, 0 });
                    --parenBalance;
                    if (parenBalance < 0) throw "Lexer error: more ')' than '('";
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
                        throw "Lexer error: unknown character";
                    }
                }
            } 
        }
        if (parenBalance != 0) throw "Lexer error: parenthesis are not balanced";
        res.push_back(Token{ TokenType::TEND, 0 });
        return res;
    }
};

enum NodeType {
    binaryOp,
    funcCall,
    variable,
    constant
};

struct Node {
    NodeType type;
    
    TokenType tokType;

    double value; // only used for constants

    Node* a; // used for left-hand-side of binary operations and for input argument of functions
    Node* b; // used for right-hand-side of binary operations
};

class Parser {
private:
    vector<Token> toks;
    size_t pos;

    void checkToken() {
        if (!((toks[pos].type == TokenType::TEND) ||
            (toks[pos].type == TokenType::Tplus) ||
            (toks[pos].type == TokenType::Tminus) ||
            (toks[pos].type == TokenType::Tmult) ||
            (toks[pos].type == TokenType::Tdiv) ||
            (toks[pos].type == TokenType::Tpow) ||
            (toks[pos].type == TokenType::TrParen)
            )) {
            throw "Parser error: expected binyaryOp, end of file or ')' after const, variable, funcCall or expression in parenthesis";
        }
    }

    Node* expr() {
        Node* a = term();
        while (toks[pos].type == TokenType::Tplus ||
            toks[pos].type == TokenType::Tminus) {
            TokenType tokType = toks[pos].type;
            ++pos;
            Node* b = term();
            a = new Node{ NodeType::binaryOp, tokType, 0, a, b }; 
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
            a = new Node{ NodeType::binaryOp, tokType, 0, a, b };
        }
        return a;
    }

    Node* factor() {
        Node* a = basic();
        //while (toks[pos].type == TokenType::Tpow) {
        if (toks[pos].type == TokenType::Tpow) {
            TokenType tokType = toks[pos].type;
            ++pos;
            //Node* b = basic();
            // Instead of looping through all ^ operators, the function makes a recursive call when encountering ^. This ensures that the right-hand side (e.g., b^c) is fully parsed before combining it with the left-hand side.
            Node* b = factor(); // we use recursive call to the right here, because exponentiation operator is right-associative
            a = new Node{ NodeType::binaryOp, tokType, 0, a, b };
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
        checkToken();
        return new Node{ NodeType::funcCall, funcType , 0, arg, nullptr };
    }

    Node* basic() {
        if (toks[pos].type == TokenType::Tconst) {
            Node* ret = new Node{ NodeType::constant, TokenType::Tconst, toks[pos].value, nullptr, nullptr };
            ++pos;
            checkToken();
            return ret;
        }
        if (toks[pos].type == TokenType::Tvariable) {
            Node* ret = new Node{ NodeType::variable, TokenType::Tvariable, 0, nullptr, nullptr};
            ++pos;
            checkToken();
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
            checkToken();
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

Node* diff(Node* root) {
    switch (root->type) {
    case NodeType::constant: // c' = 0
        return new Node{ NodeType::constant, TokenType::Tconst, 0, nullptr, nullptr };
    case NodeType::variable: // x' = 1  (3x is a multiplication, so it will be 3'x + 3x' == 3; this differentiation happens in multiplication, not here)
        return new Node{ NodeType::constant, TokenType::Tconst, 1, nullptr, nullptr };
    case NodeType::funcCall:
    {
        // f(x) = x' * f'(x)     (chain rule)
        Node* res = new Node{ NodeType::binaryOp, TokenType::Tmult, 0, diff(root->a), nullptr };
        switch (root->tokType) {
        case TokenType::Tsin: // (sin(x))' = cos(x)
        {
            res->b = new Node{ NodeType::funcCall, TokenType::Tcos, 0,
                root->a,
                nullptr
            };
        }
            break;
        case TokenType::Tcos: // (cos(x))' = -1 * sin(x)
        {
            Node* minusOne = new Node{ NodeType::constant, TokenType::Tconst, -1, nullptr, nullptr };
            Node* sinFunc = new Node{ NodeType::funcCall, TokenType::Tsin, 0,
                root->a,
                nullptr
            };
            res->b = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                minusOne,
                sinFunc
            };
        }
            break;
        case TokenType::Ttan: // (tan(x))' = 1 / cos(x)^2
        {
            Node* one = new Node{ NodeType::constant, TokenType::Tconst, 1, nullptr, nullptr };
            Node* cosFunc = new Node{ NodeType::funcCall, TokenType::Tcos, 0,
                root->a,
                nullptr
            };
            Node* two = new Node{ NodeType::constant, TokenType::Tconst, 2, nullptr, nullptr };
            Node* powBinOp = new Node{ NodeType::binaryOp, TokenType::Tpow, 0,
                cosFunc,
                two
            };
            res->b = new Node{ NodeType::binaryOp, TokenType::Tdiv, 0,
                one,
                powBinOp
            };
        }
            break;
        case TokenType::Tcot: // (cot(x))' = -1
        {
            Node* minusOne = new Node{ NodeType::constant, TokenType::Tconst, -1, nullptr, nullptr };
            Node* sinFunc = new Node{ NodeType::funcCall, TokenType::Tsin, 0,
                root->a,
                nullptr
            };
            Node* two = new Node{ NodeType::constant, TokenType::Tconst, 2, nullptr, nullptr };
            Node* powBinOp = new Node{ NodeType::binaryOp, TokenType::Tpow, 0,
                sinFunc,
                two
            };
            res->b = new Node{ NodeType::binaryOp, TokenType::Tdiv, 0,
                minusOne,
                powBinOp
            };
        }
            break;
        case TokenType::Tlog: // log is ln here // (ln(x))' = 1 / x
        {
            Node* one = new Node{ NodeType::constant, TokenType::Tconst, 1, nullptr, nullptr };
            res->b = new Node{ NodeType::binaryOp, TokenType::Tdiv, 0,
                one,
                root->a
            };
        }
            break;
        default:
            throw "Diff error: unknows funcCall TokenType";
        }
        return res;
    }
    case NodeType::binaryOp:
        switch (root->tokType) {
        case TokenType::Tplus: // (a+b)' = a' + b'
            return new Node{ NodeType::binaryOp, TokenType::Tplus, 0,
                diff(root->a),
                diff(root->b)
            };
        case TokenType::Tminus: // (a - b)' = a' - b'
            return new Node{ NodeType::binaryOp, TokenType::Tminus, 0,
                diff(root->a),
                diff(root->b)
            };
        case TokenType::Tmult: // (a * b)' = a' * b + a * b'
        {
            Node* left = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                diff(root->a),
                root->b
            };
            Node* right = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                root->a,
                diff(root->b)
            };
            return new Node{ NodeType::binaryOp, TokenType::Tplus, 0,
                left,
                right
            };
        }
        case TokenType::Tdiv: // (a / b)' = (a' * b - a * b') / b^2
        {
            Node* left = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                diff(root->a),
                root->b
            };
            Node* right = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                root->a,
                diff(root->b)
            };
            Node* top = new Node{ NodeType::binaryOp, TokenType::Tminus, 0,
                left,
                right
            };
            Node* two = new Node{ NodeType::constant, TokenType::Tconst, 2, nullptr, nullptr };
            Node* bottom = new Node{ NodeType::binaryOp, TokenType::Tpow, 0,
                root->b,
                two
            };
            return new Node{ NodeType::binaryOp, TokenType::Tdiv, 0,
                top,
                bottom
            };
        }
        case TokenType::Tpow: // (f^g)' = (f^g) * (g' * ln(f) + g * (f' / f)), where f and g are function of x: f = f(x), g = g(x), (Though they can be independent of x)
            // Examples:
            // Where g is constant:
            // (x^4)' = x^4 * (0 * ... + 4 * (1/x)) = x^4 * (4/x) = 4x^4 / x = 4x^3
            // ((2x)^4)' = (2x)^4 * (0 * ... + 4 * (2/(2x))) = (2x)^4 * (8/(2x)) = 8(2x)^4 / (2x) = 8(2x)^3 = 2 * 4(2x)^3
            // Where f is constant:
            // (4^x)' = 4^x * (1 * ln(4) + x * (0/4)) = 4^x * (ln(4) + 0) = 4^x * ln(4)
            // (4^(2x))' = 4^(2x) * (2 * ln(4) + 2x * (0/4)) = 4^(2x) * (2 * ln(4) + 0) = 4^(2x) * 2 * ln(4)
        {
            Node* left = new Node{ NodeType::binaryOp, TokenType::Tpow, 0,
                root->a,
                root->b
            };
            Node* logFunc = new Node{ NodeType::funcCall, TokenType::Tlog, 0,
                root->a,
                nullptr
            };
            Node* innerLeft = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                diff(root->b),
                logFunc
            };
            Node* div = new Node{ NodeType::binaryOp, TokenType::Tdiv, 0,
                diff(root->a),
                root->a
            };
            Node* innerRight = new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                root->b,
                div
            };
            Node* right = new Node{ NodeType::binaryOp, TokenType::Tplus, 0,
                innerLeft,
                innerRight,
            };
            return new Node{ NodeType::binaryOp, TokenType::Tmult, 0,
                left,
                right
            };
        }
        default:
            throw "Diff error: unknows binaryOp TokenType";
        }
    default:
        throw "Diff error: unknows NodeType";
    }
}

class Calculator {
private:
    value_t substitutionValue;
public:
    Calculator(value_t substitutionValue) : substitutionValue(substitutionValue) {}
    
    value_t calc(Node* root) {
        switch (root->type) {
        case NodeType::constant:
            return root->value;
        case NodeType::variable:
            return substitutionValue;
        case NodeType::funcCall:
            switch (root->tokType) {
            case TokenType::Tsin:
                return sin(calc(root->a));
            case TokenType::Tcos:
                return cos(calc(root->a));
            case TokenType::Ttan:
                return tan(calc(root->a)); // tangent never throws an error: tan(pi/2) should be undefined, but due to rounding we can never put pi/2 as ab argument
            case TokenType::Tcot:
            {
                value_t tanValue = tan(calc(root->a));
                if (tanValue == 0.0) throw "Calculator error: division by 0 (cot = 1 / tan)";
                return 1.0 / tanValue;
            }
            case TokenType::Tlog:
            {
                value_t a = calc(root->a);
                if (abs(a) <= 0.0) throw "Calculator error: log argument is outside of log's domain";
                return log(a); // natural log (base e)
            }
            default:
                throw "Calculator error: unknows funcCall TokenType";
            }
        case NodeType::binaryOp:
            switch (root->tokType) {
            case TokenType::Tplus:
                return calc(root->a) + calc(root->b);
            case TokenType::Tminus:
                return calc(root->a) - calc(root->b);
            case TokenType::Tmult:
                return calc(root->a) * calc(root->b);
            case TokenType::Tdiv:
            {
                value_t b = calc(root->b);
                if (b == 0.0) throw "Calculator error: division by 0";
                return calc(root->a) / b;
            }
            case TokenType::Tpow:
                return pow(calc(root->a), calc(root->b));
            default:
                throw "Calculator error: unknows binaryOp TokenType";
            }
        default:
            throw "Calculator error: unknows NodeType";
        }
    }
};

tuple<func_t, func_t, func_t> differentiate(const string& eq) {

    Lexer myLexer(eq);
    vector<Token> myTokens = myLexer.lex();

    Parser myParser(myTokens);
    Node* eqTree = myParser.parse(); // build abstract syntax tree

    Node* firstDiffTree = diff(eqTree);

    Node* secondDiffTree = diff(firstDiffTree);

    return {
        [eqTree](value_t substitutionValue) {
            Calculator myCalculator(substitutionValue);
            return myCalculator.calc(eqTree);
        },
        [firstDiffTree](value_t substitutionValue) {
            Calculator myCalculator(substitutionValue);
            return myCalculator.calc(firstDiffTree);
        },
        [secondDiffTree](value_t substitutionValue) {
            Calculator myCalculator(substitutionValue);
            return myCalculator.calc(secondDiffTree);
        }
    };
}

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
    case TokenType::Tconst:
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
    case TokenType::TEND:
        return "ENDTOKEN";
    default:
        throw "Unknown Token";
    }
}

char tokenTypeToSymbol(TokenType t) {
    switch (t) {
    case TokenType::Tconst:
        return 'c';
    case TokenType::Tvariable:
        return VARIABLE;
    case TokenType::Tplus:
        return '+';
    case TokenType::Tminus:
        return '-';
    case TokenType::Tmult:
        return '*';
    case TokenType::Tdiv:
        return '/';
    case TokenType::Tpow:
        return '^';
    case TokenType::TlParen:
        return '(';
    case TokenType::TrParen:
        return ')';
    default:
        throw "Unknown Token or token cannot be represented by a symbol (for example funtions or TEND TokenType)";
    }
}

string tokenVecToString(const vector<Token>& tokens) {
    string ret = "";
    for (const Token& tok : tokens) {
        ret += tokenTypeToStr(tok.type);
        if (tok.type == TokenType::Tconst) {
            ret += " " + double_to_str(tok.value);
        }
        ret += "\n";
    }
    return ret;
}

string parseTreeToString(const Node* root) {
    switch (root->type) {
    case NodeType::variable:
        return string("") + VARIABLE; // convert char to string
    case NodeType::constant:
        return double_to_str(root->value);
    case NodeType::funcCall:
        return tokenTypeToStr(root->tokType) + "(" + parseTreeToString(root->a) + ")";
    case NodeType::binaryOp:
        return "{" + parseTreeToString(root->a) + tokenTypeToSymbol(root->tokType) + parseTreeToString(root->b) + "}";
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
};

int main() {

    {
        /*TestSuit<string> s("Lexer");

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
        }*/

        /*s.isError(
            tokenVecToString(Lexer("2 + e").lex()),
            "illegal char");
        s.isError(
            tokenVecToString(Lexer("sincos 2").lex()),
            "no space between functions");*/
    }

    {
        cout << "Testing Parser:" << endl;
        cout << parseTreeToString(Parser(Lexer("3 + 3 -1").lex()).parse()) << endl;
        cout << parseTreeToString(Parser(Lexer("5 + 32 * 2").lex()).parse()) << endl;
        cout << parseTreeToString(Parser(Lexer("4* (3+11)").lex()).parse()) << endl;
        cout << parseTreeToString(Parser(Lexer("sin(4 * x + 2)").lex()).parse()) << endl;
        cout << parseTreeToString(Parser(Lexer("4+tan(4 * x + log(x))").lex()).parse()) << endl;
        //cout << parseTreeToString(Parser(Lexer("4+tan(4 * x + log(x)").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("sin 3 + 4").lex()).parse()) << endl; // should throw error
        cout << parseTreeToString(Parser(Lexer("sin(cos(tan(cot(log(x + 2)))))").lex()).parse()) << endl;
        cout << parseTreeToString(Parser(Lexer("2^3^4^x").lex()).parse()) << endl; // 2^(3^(4^x)))
        cout << parseTreeToString(Parser(Lexer("2^3*5").lex()).parse()) << endl; // (2^3) + 5
        
        //cout << parseTreeToString(Parser(Lexer("3+x+2x").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("3x+2+x").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("(3+4)x").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("((3)").lex()).parse()) << endl; // should throw error
        //cout << parseTreeToString(Parser(Lexer("(3))").lex()).parse()) << endl; // should throw error
        cout << parseTreeToString(Parser(Lexer("cos(3)))").lex()).parse()) << endl; // should throw error
    }

    {
        cout << "Testing diff:" << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x").lex()).parse())) << endl; // variable
        cout << parseTreeToString(diff(Parser(Lexer("10").lex()).parse())) << endl; // const
        cout << parseTreeToString(diff(Parser(Lexer("10 * x").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x + 4").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("3*x + 2*x").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x^5").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x^4 + 3*x^2").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("2*x-3").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("1/x").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x^3/x^7").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("x^2*x").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("7^x").lex()).parse())) << endl;

        cout << parseTreeToString(diff(Parser(Lexer("sin(x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("cos(x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("tan(x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("cot(x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("log(x)").lex()).parse())) << endl;

        cout << parseTreeToString(diff(Parser(Lexer("sin(3*x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("tan(sin(x+3)+x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("cot(log(x+9)+3*x)").lex()).parse())) << endl;
        cout << parseTreeToString(diff(Parser(Lexer("sin(cos(3*x))").lex()).parse())) << endl;

        //cout << parseTreeToString(diff(Parser(Lexer("3*x + 2x").lex()).parse())) << endl; // should throw error

    }

    {
        cout << "Testing Calculator:" << endl;
        cout << Calculator(10).calc(Parser(Lexer("20 + x").lex()).parse()) << endl;
        cout << Calculator(value_t(10, 4)).calc(Parser(Lexer("20 + x").lex()).parse()) << endl;
        cout << Calculator(value_t(0, 1)).calc(Parser(Lexer("2.718281828459^(3.14159265359*x)").lex()).parse()) << endl; // euler identity: e^(pi*i) = -1
        cout << Calculator(3).calc(diff(Parser(Lexer("x^4 + 10*x").lex()).parse())) << endl;


        // tan(x/x^x*x^x-x^(x^x)/cos(63.5+40.1)^x/(10.5^x/x^88+54.3^57.9*x^2.1/x-47.1^9.5)), x = (6.04,8.62)
        cout << Calculator(value_t(6.04, 8.62)).calc(diff(diff(Parser(Lexer("tan(x/x^x*x^x-x^(x^x)/cos(63.5+40.1)^x/(10.5^x/x^88+54.3^57.9*x^2.1/x-47.1^9.5))").lex()).parse()))) << endl;
        cout << Calculator(value_t(6.04, 8.62)).calc(diff(Parser(Lexer("tan(x/x^x*x^x-x^(x^x)/cos(63.5+40.1)^x/(10.5^x/x^88+54.3^57.9*x^2.1/x-47.1^9.5))").lex()).parse())) << endl;
    }

    return 0;
}