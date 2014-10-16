#ifndef LIB_DERP_PRIV_LANGUAGE_HPP
#define LIB_DERP_PRIV_LANGUAGE_HPP

#include <string>

#include <cassert>

namespace derp
{

namespace priv
{

// Tokes are of type T
template <typename T>
struct Language
{
    enum Type
    {
        LAZY_LANGUAGE,
        NULL_LANGUAGE,
        EMPTY_LANGUAGE,
        TERMINAL_LANGUAGE,
        ALTERNATE_LANGUAGE,
        SEQUENCE_LANGUAGE,
        REPETITION_LANGUAGE
    };

    Language() = default;
    Language(const Language<T>&) = default;
    Language(Language<T>&&) = default;
    Language<T>& operator= (const Language<T>&) = default;
    Language<T>& operator= (Language<T>&&) = default;

    bool operator== (const Language<T>& other) const;

    Language(Type type) : type(type) {}

    // The marker is for preventing infinite recursion and for marking which
    // objects were used in the current iteration (for the garbage collector)
    unsigned int marker;

    // The type of the language
    Type type;

    // For TERMINAL and LAZY
    T t; // terminal

    // For ALTERNATE and SEQUENCE
    Language<T>* left;
    Language<T>* right;

    // For REPETITION and LAZY
    Language<T>* pattern;

    // For ALTERNATE and SEQUENCE
    bool leastFixedPointFound;
    bool nullable;

    // For ALTERNATE, SEQUENCE, and REPETITION
    Language<T>* memoize;

    // Helper functions
    std::string toString(unsigned int counter);
    template <typename C>
    std::string toString(unsigned int counter, const C& c, bool skipLookup = false);
    template <typename F>
    void explore(unsigned int counter, F callback);

    // Important functions
    template <typename A>
    bool isNullable(unsigned int counter, A& allocate);
    template <typename A>
    Language<T>* derive(T token, unsigned int counter, A& allocate);
    template <typename A>
    Language<T>* force(unsigned int counter, A& allocate);
    void mark(unsigned int counter);
    Language<T>* compact();

    static Language<T> null;
    static Language<T> empty;
};

template <typename T>
Language<T> Language<T>::null(Language<T>::NULL_LANGUAGE);

template <typename T>
Language<T> Language<T>::empty(Language<T>::EMPTY_LANGUAGE);

template <typename T>
bool Language<T>::operator== (const Language<T>& other) const
{
    return marker == other.marker &&
        type == other.type &&
        t == other.t &&
        left == other.left &&
        right == other.right &&
        pattern == other.pattern &&
        leastFixedPointFound == other.leastFixedPointFound &&
        nullable == other.nullable &&
        memoize == other.memoize;
}

template <typename T>
std::string Language<T>::toString(unsigned int counter)
{
    if (marker == counter)
    {
        switch (type)
        {
            case Language<T>::NULL_LANGUAGE:     break;
            case Language<T>::EMPTY_LANGUAGE:    break;
            case Language<T>::TERMINAL_LANGUAGE: break;
            default:                             return "\u221E"; // Infinity symbol
        }
    }

    marker = counter;

    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:       return "D_" + std::string(1, t) + "(" + pattern->toString(counter) + ")";
        case Language<T>::NULL_LANGUAGE:       return "\u2205";
        case Language<T>::EMPTY_LANGUAGE:      return "\u025B";
        case Language<T>::TERMINAL_LANGUAGE:   return "'" + std::string(1, t) + "'";
        case Language<T>::ALTERNATE_LANGUAGE:  return "(" + left->toString(counter) + " | " + right->toString(counter) + ")";
        case Language<T>::SEQUENCE_LANGUAGE:   return left->toString(counter) + " " + right->toString(counter);
        case Language<T>::REPETITION_LANGUAGE: return "(" + pattern->toString(counter) + ")*";
    }
}

template <typename T>
template <typename C>
std::string Language<T>::toString(unsigned int counter, const C& c, bool skipLookup)
{
    if (!skipLookup)
    {
        auto i = c.find(this);
        if (i != c.end()) return i->second;
    }

    if (marker == counter)
    {
        switch (type)
        {
            case Language<T>::NULL_LANGUAGE:     break;
            case Language<T>::EMPTY_LANGUAGE:    break;
            case Language<T>::TERMINAL_LANGUAGE: break;
            default:                             return "\u221E"; // Infinity symbol
        }
    }

    marker = counter;

    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:       return "D_" + std::string(1, t) + "(" + pattern->toString(counter, c) + ")";
        case Language<T>::NULL_LANGUAGE:       return "\u2205";
        case Language<T>::EMPTY_LANGUAGE:      return "\u025B";
        case Language<T>::TERMINAL_LANGUAGE:   return "'" + std::string(1, t) + "'";
        case Language<T>::ALTERNATE_LANGUAGE:  return "(" + left->toString(counter, c) + " | " + right->toString(counter, c) + ")";
        case Language<T>::SEQUENCE_LANGUAGE:   return left->toString(counter, c) + " " + right->toString(counter, c);
        case Language<T>::REPETITION_LANGUAGE: return "(" + pattern->toString(counter, c) + ")*";
    }
}

template <typename T>
template <typename F>
void Language<T>::explore(unsigned int counter, F callback)
{
    if (marker == counter) return;

    callback(static_cast<const Language<T>*>(this));

    marker = counter;

    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:       pattern->explore(counter, callback); return;
        case Language<T>::NULL_LANGUAGE:       return;
        case Language<T>::EMPTY_LANGUAGE:      return;
        case Language<T>::TERMINAL_LANGUAGE:   return;
        case Language<T>::ALTERNATE_LANGUAGE:  left->explore(counter, callback); right->explore(counter, callback); return;
        case Language<T>::SEQUENCE_LANGUAGE:   left->explore(counter, callback); right->explore(counter, callback); return;
        case Language<T>::REPETITION_LANGUAGE: pattern->explore(counter, callback); return;
    }
}

template <typename T>
template <typename A>
bool Language<T>::isNullable(unsigned int counter, A& allocate)
{
    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:       return force(counter, allocate)->isNullable(counter, allocate);
        case Language<T>::NULL_LANGUAGE:       return false;
        case Language<T>::EMPTY_LANGUAGE:      return true;
        case Language<T>::TERMINAL_LANGUAGE:   return false;
        case Language<T>::ALTERNATE_LANGUAGE:
            {
                if (leastFixedPointFound) return nullable;

                leastFixedPointFound = true;
                nullable = false;
                bool back = left->isNullable(counter, allocate) || right->isNullable(counter, allocate);
                nullable = back;

                while ((left->isNullable(counter, allocate) || right->isNullable(counter, allocate)) != back)
                {
                    back = left->isNullable(counter, allocate) || right->isNullable(counter, allocate);
                    nullable = back;
                }

                return back;
            }
        case Language<T>::SEQUENCE_LANGUAGE:
            {
                if (leastFixedPointFound) return nullable;

                leastFixedPointFound = true;
                nullable = false;
                bool back = left->isNullable(counter, allocate) && right->isNullable(counter, allocate);
                nullable = back;

                while ((left->isNullable(counter, allocate) && right->isNullable(counter, allocate)) != back)
                {
                    back = left->isNullable(counter, allocate) && right->isNullable(counter, allocate);
                    nullable = back;
                }

                return back;

            }
        case Language<T>::REPETITION_LANGUAGE: return true;
    }

    assert(false);
    return false;
}

template <typename T>
template <typename A>
Language<T>* Language<T>::derive(T token, unsigned int counter, A& allocate)
{
    if (marker != counter)
    {
        marker = counter;
        memoize = nullptr;
    }

    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:      return force(counter, allocate)->derive(token, counter, allocate);
        case Language<T>::NULL_LANGUAGE:      return &null;
        case Language<T>::EMPTY_LANGUAGE:     return &null;
        case Language<T>::TERMINAL_LANGUAGE:  return (t == token) ? &empty : &null;
        case Language<T>::ALTERNATE_LANGUAGE:
            {
                Language<T>* result;
                if (memoize != nullptr)
                {
                    result = memoize;
                }
                else
                {
                    Language<T>* alt = allocate();
                    alt->marker = counter;
                    alt->memoize = nullptr;
                    alt->leastFixedPointFound = false;
                    alt->type = Language<T>::ALTERNATE_LANGUAGE;

                    alt->left = allocate();
                    alt->left->marker = counter;
                    alt->left->type = Language<T>::LAZY_LANGUAGE;
                    alt->left->t = token;
                    alt->left->pattern = left;

                    alt->right = allocate();
                    alt->right->marker = counter;
                    alt->right->type = Language<T>::LAZY_LANGUAGE;
                    alt->right->t = token;
                    alt->right->pattern = right;

                    memoize = alt;

                    alt->left = alt->left->force(counter, allocate);
                    alt->right = alt->right->force(counter, allocate);

                    result = memoize = alt->compact();
                }

                return result;
            }
        case Language<T>::SEQUENCE_LANGUAGE:
            {
                Language<T>* result;
                if (memoize != nullptr)
                {
                    result = memoize;
                }
                else
                {
                    Language<T>* seq = allocate();
                    seq->marker = counter;
                    seq->memoize = nullptr;
                    seq->leastFixedPointFound = false;
                    seq->type = Language<T>::SEQUENCE_LANGUAGE;

                    seq->left = allocate();
                    seq->left->marker = counter;
                    seq->left->type = Language<T>::LAZY_LANGUAGE;
                    seq->left->t = token;
                    seq->left->pattern = left;

                    seq->right = right;
                    right->mark(counter);

                    if (left->isNullable(counter, allocate))
                    {
                        Language<T>* alt = allocate();
                        alt->marker = counter;
                        alt->memoize = nullptr;
                        alt->leastFixedPointFound = false;
                        alt->type = Language<T>::ALTERNATE_LANGUAGE;

                        alt->left = allocate();
                        alt->left->marker = counter;
                        alt->left->type = Language<T>::LAZY_LANGUAGE;
                        alt->left->t = token;
                        alt->left->pattern = right;

                        alt->right = seq;

                        memoize = alt;

                        seq->left = seq->left->force(counter, allocate);
                        alt->left = alt->left->force(counter, allocate);

                        alt->right = seq->compact();

                        result = memoize = alt->compact();
                    }
                    else
                    {
                        memoize = seq;

                        seq->left = seq->left->force(counter, allocate);

                        result = memoize = seq->compact();
                    }
                }

                return result;
            }
        case Language<T>::REPETITION_LANGUAGE:
            {
                Language<T>* result;
                if (memoize != nullptr)
                {
                    result = memoize;
                }
                else
                {
                    Language<T>* seq = allocate();
                    seq->marker = counter;
                    seq->memoize = nullptr;
                    seq->leastFixedPointFound = false;
                    seq->type = Language<T>::SEQUENCE_LANGUAGE;

                    seq->left = allocate();
                    seq->left->marker = counter;
                    seq->left->type = Language<T>::LAZY_LANGUAGE;
                    seq->left->t = token;
                    seq->left->pattern = pattern;

                    seq->right = this;

                    memoize = seq;

                    seq->left = seq->left->force(counter, allocate);

                    result = memoize = seq->compact();
                }

                return result;
            }
    }

    assert(false);
    return nullptr;
}

template <typename T>
template <typename A>
Language<T>* Language<T>::force(unsigned int counter, A& allocate)
{
    if (type != Language<T>::LAZY_LANGUAGE) return this;

    Language<T>* optimal = pattern->force(counter, allocate)->derive(t, counter, allocate);
    *this = *optimal;

    return optimal;
}

template <typename T>
void Language<T>::mark(unsigned int counter)
{
    if (marker == counter) return;

    marker = counter;
    memoize = nullptr;

    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:       pattern->mark(counter); return;
        case Language<T>::NULL_LANGUAGE:       return;
        case Language<T>::EMPTY_LANGUAGE:      return;
        case Language<T>::TERMINAL_LANGUAGE:   return;
        case Language<T>::ALTERNATE_LANGUAGE:  left->mark(counter); right->mark(counter); return;
        case Language<T>::SEQUENCE_LANGUAGE:   left->mark(counter); right->mark(counter); return;
        case Language<T>::REPETITION_LANGUAGE: pattern->mark(counter); return;
    }
}

template <typename T>
Language<T>* Language<T>::compact()
{
    // This method must only be called from derive(), and children *must* be validly marked
    Language<T>* optimal;
    switch (type)
    {
        case Language<T>::LAZY_LANGUAGE:      return this;
        case Language<T>::NULL_LANGUAGE:      return &null;
        case Language<T>::EMPTY_LANGUAGE:     return &empty;
        case Language<T>::TERMINAL_LANGUAGE:  return this;
        case Language<T>::ALTERNATE_LANGUAGE:
            {
                if (left->type == Language<T>::NULL_LANGUAGE)
                {
                    optimal = right;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = *optimal;
                    return optimal;
                }
                else if (right->type == Language<T>::NULL_LANGUAGE)
                {
                    optimal = left;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = *optimal;
                    return optimal;
                }

                if (left->type == Language<T>::EMPTY_LANGUAGE)
                {
                    left = &empty;
                }

                if (right->type == Language<T>::EMPTY_LANGUAGE)
                {
                    // Swap left and right so left is empty language (for short circuiting isNullable())
                    right = left;
                    left = &empty;
                }

                if (left == right)
                {
                    optimal = left;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = *optimal;
                    return optimal;
                }

                return this;
            }
        case Language<T>::SEQUENCE_LANGUAGE:
            {
                if (left->type == Language<T>::NULL_LANGUAGE ||
                    right->type == Language<T>::NULL_LANGUAGE)
                {
                    optimal = &null;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = null;
                    return optimal;
                }
                else if (left->type == Language<T>::EMPTY_LANGUAGE)
                {
                    optimal = right;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = *optimal;
                    return optimal;
                }
                else if (right->type == Language<T>::EMPTY_LANGUAGE)
                {
                    optimal = left;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = *optimal;
                    return optimal;
                }

                return this;
            }
        case Language<T>::REPETITION_LANGUAGE:
            {
                if (pattern->type == Language<T>::NULL_LANGUAGE ||
                    pattern->type == Language<T>::EMPTY_LANGUAGE)
                {
                    optimal = &empty;
                    optimal->marker = marker; // This should be unnecessary, unless optimal is &null or &empty
                    *this = empty;
                    return optimal;
                }

                return this;
            }
    }

    assert(false);
    return nullptr;
}

template <typename T>
struct IsDead
{
    IsDead(unsigned int m) : marker(m)
    {
    }

    unsigned int marker;

    bool operator() (const Language<T>* lang) const
    {
        return lang->marker != marker;
    }
};

template <typename A>
Language<char>* terminal(A& allocate, char c)
{
    Language<char>* lit = allocate();
    lit->marker = 0;
    lit->type = Language<char>::TERMINAL_LANGUAGE;
    lit->t = c;
    return lit;
}

template <typename T, typename A>
Language<T>* alternate(A& allocate, Language<T>* left, Language<T>* right)
{
    assert(left);
    assert(right);
    Language<T>* alt = allocate();
    alt->marker = 0;
    alt->memoize = nullptr;
    alt->leastFixedPointFound = false;
    alt->type = Language<T>::ALTERNATE_LANGUAGE;
    alt->left = left;
    alt->right = right;
    return alt;
}

template <typename T, typename A>
Language<T>* sequence(A& allocate, Language<T>* left, Language<T>* right)
{
    assert(left);
    assert(right);
    Language<T>* seq = allocate();
    seq->marker = 0;
    seq->memoize = nullptr;
    seq->leastFixedPointFound = false;
    seq->type = Language<T>::SEQUENCE_LANGUAGE;
    seq->left = left;
    seq->right = right;
    return seq;
}

template <typename A>
Language<char>* sequence(A& allocate, const std::string& str)
{
    if (str.empty()) return &Language<char>::empty;

    Language<char>* lang = terminal(allocate, str.back());
    for (auto i = str.crbegin() + 1; i != str.crend(); ++i)
    {
        lang = sequence(allocate, terminal(allocate, *i), lang);
        // We know it's not nullable, so we can just set that now
        lang->leastFixedPointFound = true;
        lang->nullable = false;
    }

    return lang;
}

template <typename T, typename A>
Language<T>* repetition(A& allocate, Language<T>* pattern)
{
    assert(pattern);
    Language<T>* rep = allocate();
    rep->marker = 0;
    rep->memoize = nullptr;
    rep->type = Language<T>::REPETITION_LANGUAGE;
    rep->pattern = pattern;
    return rep;
}

template <typename A>
Language<char>* anyOf(A& allocate, const std::string& str)
{
    if (str.empty()) return &Language<char>::empty;

    Language<char>* lang = terminal(allocate, str[0]);
    for (auto i = str.cbegin() + 1; i != str.cend(); ++i)
    {
        lang = alternate(allocate, lang, terminal(allocate, *i));
        // We know it's not nullable, so we can just set that now
        lang->leastFixedPointFound = true;
        lang->nullable = false;
    }

    return lang;
}

} // namespace priv

} // namespace derp

#endif
