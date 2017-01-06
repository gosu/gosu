#pragma once

#include "GraphicsImpl.hpp"
#include <cassert>
#include <algorithm>

namespace Gosu
{
    class TransformStack
    {
        // All the matrices that are pushed right now.
        Transforms individual;
        // All the absolute matrices that have been created since last reset.
        Transforms absolute;
        // Points to one absolute transform.
        Transforms::const_iterator current_iterator;
        
        void make_current(const Transform& transform)
        {
            current_iterator = std::find(absolute.begin(), absolute.end(), transform);
            if (current_iterator == absolute.end()) {
                current_iterator = absolute.insert(absolute.end(), transform);
            }
        }
        
    public:
        TransformStack()
        {
            reset();
            individual.front() = absolute.front() = scale(1);
        }

        void reset()
        {
            // Every queue has a base transform that is always the current transform.
            // This keeps the code a bit more uniform, and allows the window to
            // set a base transform in the main rendering queue.
            individual.resize(1);
            absolute.resize(1);
            current_iterator = absolute.begin();
        }
        
        TransformStack(const TransformStack& other)
        {
            *this = other;
        }
        
        // Custom assignment to ensure valid current_iterator
        TransformStack& operator=(const TransformStack& other)
        {
            individual = other.individual;
            absolute = other.absolute;
            
            // Reset our current_iterator to point to the respective element
            // in our own 'absolute' transforms by iterating both lists up to
            // the other lists' current iterator
            current_iterator = absolute.begin();
            Transforms::const_iterator other_iterator = other.absolute.begin();
            while (other_iterator != other.current_iterator) {
                ++current_iterator;
                ++other_iterator;
            }
            
            return *this;
        }
        
        void set_base_transform(const Transform& base_transform)
        {
            assert (individual.size() == 1);
            assert (absolute.size() == 1);
            
            individual.front() = absolute.front() = base_transform;
        }
        
        const Transform& current()
        {
            return *current_iterator;
        }
        
        void push(const Transform& transform)
        {
            individual.push_back(transform);
            Transform result = concat(transform, current());
            make_current(result);
        }
        
        void pop()
        {
            assert (individual.size() > 1);
            
            individual.pop_back();
            Transform result = scale(1);
            for (Transforms::reverse_iterator it = individual.rbegin(),
                    end = individual.rend(); it != end; ++it)
                result = concat(result, *it);
            make_current(result);
        }
    };
}
