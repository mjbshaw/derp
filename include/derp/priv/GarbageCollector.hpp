#ifndef LIB_DERP_PRIV_GARBAGE_COLLECTOR_HPP
#define LIB_DERP_PRIV_GARBAGE_COLLECTOR_HPP

#include <vector>

namespace derp
{

namespace priv
{

template <typename T>
struct GarbageCollector
{
    GarbageCollector() = default;
    GarbageCollector(const GarbageCollector<T>&) = delete;
    GarbageCollector(GarbageCollector<T>&&) = delete;
    GarbageCollector<T>& operator= (const GarbageCollector<T>&) = delete;
    GarbageCollector<T>& operator= (GarbageCollector<T>&&) = delete;

    std::vector<T*> alive;
    std::vector<T*> dead;

    ~GarbageCollector()
    {
        for (T* t : alive)
        {
            delete t;
        }

        for (T* t : dead)
        {
            delete t;
        }
    }

    template <typename C>
    void steal(C& container)
    {
        container.insert(container.end(), alive.begin(), alive.end());
        alive.clear();
    }

    void steal(std::vector<T*>& container)
    {
        if (container.empty())
        {
            alive.swap(container);
        }
        else
        {
            steal<std::vector<T*>>(container);
        }
    }

    template <typename C>
    void give(C& container)
    {
        alive.insert(alive.end(), container.begin(), container.end());
        container.clear();
    }

    template <typename C>
    void give(std::vector<T*>& container)
    {
        if (alive.empty())
        {
            alive.swap(container);
        }
        else
        {
            give<std::vector<T*>>(container);
        }
    }

    template <typename P>
    void collect(P isDead)
    {
        for (std::size_t i = 0; i < alive.size();)
        {
            if (isDead(alive[i]))
            {
                dead.push_back(alive[i]);
                alive[i] = alive.back();
                alive.pop_back();
            }
            else
            {
                ++i;
            }
        }
    }

    void collect()
    {
        if (dead.empty())
        {
            dead.swap(alive);
        }
        else
        {
            dead.insert(dead.end(), alive.begin(), alive.end());
            alive.clear();
        }
    }

    void shrink()
    {
        for (T* t : dead)
        {
            delete t;
        }

        dead.clear();
    }

    T* allocate()
    {
        if (dead.empty())
        {
            alive.push_back(new T);
        }
        else
        {
            alive.push_back(dead.back());
            dead.pop_back();
        }

        return alive.back();
    }

    T* operator() ()
    {
        return allocate();
    }
};

} // namespace priv

} // namespace derp

#endif
