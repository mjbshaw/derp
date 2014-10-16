#ifndef LIB_DERP_LANGUAGE_HPP
#define LIB_DERP_LANGUAGE_HPP

#include "priv/GarbageCollector.hpp"
#include "priv/Language.hpp"

#include <string>
#include <unordered_map>
#include <vector>

#include <cassert>

namespace derp
{

template <typename T, typename A = priv::GarbageCollector<priv::Language<T>>>
class Language
{
public:
    typedef A GarbageCollector;

    Language(A& gc) : gc(gc), l(gc.allocate()) {}
    Language(const Language<T, A>&) = default;
    Language(Language<T, A>&&) = default;

    Language(A& gc, T c) : gc(gc), l(priv::terminal(gc, c)) {}

    Language(A& gc, const std::string& str) : gc(gc), l(priv::sequence(gc, str)) {}

    Language<T, A>& operator= (const T& t) { *l = *priv::terminal(gc, t); return *this; }

    Language<T, A>& operator= (const std::string& str) { *l = *priv::sequence(gc, str); return *this; }

    Language<T, A>& operator= (const Language<T, A>& other);
    Language<T, A>& operator= (Language<T, A>&& other);

    bool operator==(const Language<T, A>& other) const;
    bool operator<(const Language<T, A>& other) const;

    std::string toString() const;

    template <typename C = std::vector<std::pair<Language<T, A>, std::string>>>
    std::string toString(const C& c) const;

private:
    A& gc;
    priv::Language<T>* l;

    Language(A& gc, priv::Language<T>* l) : gc(gc), l(l) {}

    static Language<T, A> null(A& gc) { return Language(gc, &priv::Language<T>::null); }
    static Language<T, A> empty(A& gc) { return Language(gc, &priv::Language<T>::empty); }

    template <typename RT, typename RA>
    friend Language<RT, RA> operator& (const Language<RT, RA>& left, const Language<RT, RA>& right);

    template <typename RA>
    friend Language<char, RA> operator& (const Language<char, RA>& left, char right);

    template <typename RA>
    friend Language<char, RA> operator& (const Language<char, RA>& left, const std::string& right);

    template <typename RA>
    friend Language<char, RA> operator& (char left, const Language<char, RA>& right);

    template <typename RA>
    friend Language<char, RA> operator& (const std::string& left, const Language<char, RA>& right);

    template <typename RT, typename RA>
    friend Language<RT, RA> operator| (const Language<RT, RA>& left, const Language<RT, RA>& right);

    template <typename RA>
    friend Language<char, RA> operator| (const Language<char, RA>& left, char right);

    template <typename RA>
    friend Language<char, RA> operator| (const Language<char, RA>& left, const std::string& right);

    template <typename RA>
    friend Language<char, RA> operator| (char left, const Language<char, RA>& right);

    template <typename RA>
    friend Language<char, RA> operator| (const std::string& left, const Language<char, RA>& right);

    template <typename RT, typename RA>
    friend Language<RT, RA> operator* (const Language<RT, RA>& pattern);

    template <typename RT, typename RA>
    friend Language<RT, RA> operator+ (const Language<RT, RA>& pattern);

    template <typename RT, typename RA>
    friend Language<RT, RA> operator- (const Language<RT, RA>& pattern);

    template <typename RA>
    friend bool matches(const std::string& input, Language<T, RA>& lang);

    template <typename L>
    friend class Factory;

    friend struct std::hash<Language<T, A>>;
};

template <typename T, typename A>
Language<T, A>& Language<T, A>::operator= (const Language<T, A>& other)
{
    assert(&gc == &other.gc);

    switch (other.l->type)
    {
        case priv::Language<T>::LAZY_LANGUAGE:       assert(other.l->pattern); break;
        case priv::Language<T>::NULL_LANGUAGE:       break;
        case priv::Language<T>::EMPTY_LANGUAGE:      break;
        case priv::Language<T>::TERMINAL_LANGUAGE:   break;
        case priv::Language<T>::ALTERNATE_LANGUAGE:  assert(other.l->left); assert(other.l->right); break;
        case priv::Language<T>::SEQUENCE_LANGUAGE:   assert(other.l->left); assert(other.l->right); break;
        case priv::Language<T>::REPETITION_LANGUAGE: assert(other.l->pattern); break;
    }

    *l = *other.l;
    return *this;
}

template <typename T, typename A>
Language<T, A>& Language<T, A>::operator= (Language<T, A>&& other)
{
    assert(&gc == &other.gc);

    switch (other.l->type)
    {
        case priv::Language<T>::LAZY_LANGUAGE:       assert(other.l->pattern); break;
        case priv::Language<T>::NULL_LANGUAGE:       break;
        case priv::Language<T>::EMPTY_LANGUAGE:      break;
        case priv::Language<T>::TERMINAL_LANGUAGE:   break;
        case priv::Language<T>::ALTERNATE_LANGUAGE:  assert(other.l->left); assert(other.l->right); break;
        case priv::Language<T>::SEQUENCE_LANGUAGE:   assert(other.l->left); assert(other.l->right); break;
        case priv::Language<T>::REPETITION_LANGUAGE: assert(other.l->pattern); break;
    }

    *l = std::move(*other.l);
    return *this;
}

template <typename T, typename A>
bool Language<T, A>::operator==(const Language<T, A>& other) const
{
    assert(&gc == &other.gc);

    return *l == *other.l;
}

template <typename T, typename A>
bool Language<T, A>::operator<(const Language<T, A>& other) const
{
    assert(&gc == &other.gc);

    return l < other.l;
}

template <typename T, typename A>
std::string Language<T, A>::toString() const
{
    std::vector<priv::Language<T>*> v;
    gc.steal(v);
    for (priv::Language<T>* lang : v)
    {
        lang->marker = 1;
    }
    gc.give(v);

    return l->toString(0);
}

template <typename T, typename A>
template <typename C>
std::string Language<T, A>::toString(const C& c) const
{
    std::vector<priv::Language<T>*> v;
    gc.steal(v);
    for (priv::Language<T>* lang : v)
    {
        lang->marker = 1;
    }
    gc.give(v);

    std::unordered_map<const priv::Language<T>*, const std::string&> cp;
    for (const auto& i : c)
    {
        cp.emplace(i.first.l, i.second);
    }

    return l->toString(0, cp, true);
}

// Sequence
template <typename T, typename A>
Language<T, A> operator& (const Language<T, A>& left, const Language<T, A>& right)
{
    assert(&left.gc == &right.gc);
    return Language<T, A>(left.gc, priv::sequence(left.gc, left.l, right.l));
}

template <typename A>
Language<char, A> operator& (const Language<char, A>& left, char right)
{
    return Language<char, A>(left.gc, priv::sequence(left.gc, left.l, priv::terminal(left.gc, right)));
}

template <typename A>
Language<char, A> operator& (const Language<char, A>& left, const std::string& right)
{
    return Language<char, A>(left.gc, priv::sequence(left.gc, left.l, priv::sequence(left.gc, right)));
}

template <typename A>
Language<char, A> operator& (char left, const Language<char, A>& right)
{
    return Language<char, A>(right.gc, priv::sequence(right.gc, priv::terminal(right.gc, left), right.l));
}

template <typename A>
Language<char, A> operator& (const std::string& left, const Language<char, A>& right)
{
    return Language<char, A>(right.gc, priv::sequence(right.gc, priv::sequence(right.gc, left), right.l));
}

// Alternate
template <typename T, typename A>
Language<T, A> operator| (const Language<T, A>& left, const Language<T, A>& right)
{
    assert(&left.gc == &right.gc);
    return Language<T, A>(left.gc, priv::alternate(left.gc, left.l, right.l));
}

template <typename A>
Language<char, A> operator| (const Language<char, A>& left, char right)
{
    return Language<char, A>(left.gc, priv::alternate(left.gc, left.l, priv::terminal(left.gc, right)));
}

template <typename A>
Language<char, A> operator| (const Language<char, A>& left, const std::string& right)
{
    return Language<char, A>(left.gc, priv::alternate(left.gc, left.l, priv::sequence(left.gc, right)));
}

template <typename A>
Language<char, A> operator| (char left, const Language<char, A>& right)
{
    return Language<char, A>(right.gc, priv::alternate(right.gc, priv::terminal(right.gc, left), right.l));
}

template <typename A>
Language<char, A> operator| (const std::string& left, const Language<char, A>& right)
{
    return Language<char, A>(right.gc, priv::alternate(right.gc, priv::sequence(right.gc, left), right.l));
}

// Kleene star
template <typename T, typename A>
Language<T, A> operator* (const Language<T, A>& pattern)
{
    return Language<T, A>(pattern.gc, priv::repetition(pattern.gc, pattern.l));
}

// Kleene plus
template <typename T, typename A>
Language<T, A> operator+ (const Language<T, A>& pattern)
{
    return pattern & *pattern;
}

// Optional
template <typename T, typename A>
Language<T, A> operator- (const Language<T, A>& pattern)
{
    return Language<T, A>::empty(pattern.gc) | pattern;
}

template <typename L>
class Factory
{
public:
    Factory(typename L::GarbageCollector& gc) : gc(gc) {}

    template <typename U>
    L operator() (const U& val) const
    {
        return L(gc, val);
    }

    L operator() () const
    {
        return L(gc);
    }

    L null() const
    {
        return L::null(gc);
    }

    L empty() const
    {
        return L::empty(gc);
    }

private:
    typename L::GarbageCollector& gc;
};

template <typename A>
bool matches(const std::string& input, Language<char, A>& language)
{
    A& gc = language.gc;
    priv::Language<char>* lang = language.l;

    std::vector<priv::Language<char>*> invincible;
    gc.steal(invincible);

    unsigned int counter = 0;
    for (priv::Language<char>* l : invincible)
    {
        l->marker = counter;
        l->memoize = nullptr;
    }

    for (char c : input)
    {
        ++counter;
        lang = lang->derive(c, counter, gc);
        gc.collect(priv::IsDead<char>(counter));
    }

    bool matched = lang->isNullable(counter, gc);

    gc.collect();
    gc.give(invincible);

    return matched;
}

} // namespace derp

namespace std
{

template <typename T, typename A>
struct hash<derp::Language<T, A>>
{
    size_t operator() (const derp::Language<T, A>& l) const
    {
        return hash<derp::priv::Language<T>*>()(l.l);
    }
};
    
} // namespace std

#endif
