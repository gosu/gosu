#ifndef GOSUIMPL_GRAPHICS_TRANSFORMSTACK_HPP
#define GOSUIMPL_GRAPHICS_TRANSFORMSTACK_HPP

#include "Common.hpp"
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
        Transforms::const_iterator currentIterator;
        
        void makeCurrent(const Transform& transform)
        {
            currentIterator =
                std::find(absolute.begin(), absolute.end(), transform);
            if (currentIterator == absolute.end())
                currentIterator = absolute.insert(absolute.end(), transform);
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
            currentIterator = absolute.begin();
        }
        
        TransformStack(const TransformStack& other)
        {
            *this = other;
        }
        
        // Custom assignment to ensure valid currentIterator
        TransformStack& operator=(const TransformStack &other)
        {
            individual = other.individual;
            absolute = other.absolute;
            
            // Reset our currentIterator to point to the respective element
            // in our own 'absolute' transforms by iterating both lists up to
            // the other lists' current iterator
            currentIterator = absolute.begin();
            Transforms::const_iterator otherIterator = other.absolute.begin();
            while (otherIterator != other.currentIterator)
                ++currentIterator, ++otherIterator;
            
            return *this;
        }
        
        void setBaseTransform(const Transform& baseTransform)
        {
            assert (individual.size() == 1);
            assert (absolute.size() == 1);
            
            individual.front() = absolute.front() = baseTransform;
        }
        
        const Transform& current()
        {
            return *currentIterator;
        }
        
        void push(const Transform& transform)
        {
            individual.push_back(transform);
            Transform result = multiply(transform, current());
            makeCurrent(result);
        }
        
        void pop()
        {
            assert (individual.size() > 1);
            
            individual.pop_back();
            Transform result = scale(1);
            for (Transforms::reverse_iterator it = individual.rbegin(),
                    end = individual.rend(); it != end; ++it)
                result = multiply(result, *it);
            makeCurrent(result);
        }
    };
}

#endif
