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

    // Language = ("foo" | "bar")*
    Language l = *(F("foo") | "bar");

    std::cout << "grammar: " << l.toString() << std::endl;
    std::cout << "input: " << std::flush;

    std::string input;
    std::getline(std::cin, input);

    std::cout << "matches? " << derp::matches(input, l) << std::endl;
}
