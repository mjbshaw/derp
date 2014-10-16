#include <derp/Language.hpp>

#include <iostream>
#include <string>

int main()
{
    using Language = derp::Language<char>;
    using GC = Language::GarbageCollector;
    using Factory = derp::Factory<Language>;

    GC gc;
    Factory F(gc);

    // alpha = [_a-zA-Z]
    // identifier = alpha+
    Language alpha = F('_') |
        'a' | 'b' | 'c' | 'd' | 'e' | 'f' | 'g' | 'h' | 'i' | 'j' | 'k' | 'l' | 'm' | 'n' | 'o' | 'p' | 'q' | 'r' | 's' | 't' | 'u' | 'v' | 'w' | 'x' | 'y' | 'z' |
        'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z';
    Language symbol = +alpha;

    // digit = [0-9]
    // number = '-'? digit* \.? digit+
    Language digit = F('0') | '1' | '2' | '3' | '4' | '5' | '6' | '8' | '9';
    Language number = -F('-') & *digit & -F('.') & +digit;

    // boolean = "#t" | "#f"
    Language boolean = F("#t") | "#f";

    // whitespace = [ \r\n\t]*
    Language whitespace = *(F(' ') | '\r' | '\n' | '\t');

    // atom = symbol | number | boolean
    Language atom = symbol | number | boolean;

    // sexplist = sexp whitespace sexplist | ""
    // sexp = atom | '(' whitespace sexplist whitespace ')'
    Language sexplist = F();
    Language sexp = F();
    sexplist = (sexp & whitespace & sexplist) | "";
    sexp = atom | '(' & whitespace & sexplist & whitespace & ')';

    const std::vector<std::pair<Language, std::string>> names = {
        {alpha, "alpha"},
        {symbol, "symbol"},
        {digit, "digit"},
        {number, "number"},
        {boolean, "boolean"},
        {whitespace, "whitespace"},
        {sexplist, "sexplist"},
        {sexp, "sexp"}
    };
    std::cout << "grammar: " << std::endl;
    for (const std::pair<Language, std::string>& pair : names)
    {
        std::cout << pair.second << " = " << pair.first.toString(names) << std::endl;
    }

    std::cout << "input: " << std::flush;

    std::string input;
    std::getline(std::cin, input);

    std::cout << "matches? " << derp::matches(input, sexp) << std::endl;
}
