Solution for the Kata on Codewars: https://www.codewars.com/kata/6088985e8d94ef0034d0f712

This is a symbolic differentiator.

Keep the precision limitations of double in mind.

Trigonometric functions arguments are in radians, not degrees.

Exponential towers are handled the same way Wolfram Alpha does. E.g. a^b^c is a^(b^c), not (a^b)^c.


Kata description:

The task
Create a function differentiate which takes a string as an argument and creates another 3 complex-argument functions which are:

the function in the string itself;
its first derivative;
its second derivative.
The input
The input is a string that is described by the following BNF:

<constant>  ::= [0-9]+ ( "." [0-9]+ )?
<variable>  ::= "x"
<func_name> ::= "sin" | "cos" | "tan" | "cot" | "log"

<expression> ::= <term> ( ( "+" | "-" ) <term> )*
<term>       ::= <factor> ( ( "*" | "/" ) <factor> )*
<factor>     ::= <basic> ( "^" <basic> )*
<func_call>  ::= <func_name> "(" <expression> ")"
<basic>      ::= <constant> | <variable> | <func_call> | ( "(" <expression> ")" )
Assume all inputs to be valid.

The output
The outputs are the parsed function, its first and second derivatives. All of them should receive and return a complex number. For example:

const auto f = differentiate("2 * x^3");

std::get<0>(f)({ 2, 2 }) == (-32, 32)
/*
  Because: 
  f(x) = 2 * x^3
  f(2 + 2i) = 2 * (2 + 2i)^3 = -32 + 32i
*/
  
std::get<1>(f)({ 2, 2 }) == (0, 48)
/*
  Because: 
  f'(x) = (2 * x^3)' = 6 * x^2
  f'(2 + 2i) = 6 * (2 + 2i)^2 = 48i
*/
  
std::get<2>(f)({ 2, 2 }) == (24, 24)
/*
  Because:
  f''(x) = (6 * x^2)' = 12 * x
  f''(2 + 2i) = 12 * (2 + 2i)^2 = 24 + 24i
*/
Notes
Sometimes the input function or its derivatives will be impossible to evaluate for some arguments (e.g. log(0)). This is still considered valid input. Though, the return value of the returned function will not be tested if this is a case.
Spaces are allowed between any tokens except inside of <constant>. E.g. x + 1 and cos ( 1 / 2 ) are valid, but 2. 0 or 2 1 are not.
PEMDAS rule is applied.
Exponential towers must be handled the same way Wolfram Alpha does. E.g. a^b^c is a^(b^c), not (a^b)^c.
log is the natural logarithm.
Trigonometric functions arguments are in radians, not degrees.
The real task
Have fun :)