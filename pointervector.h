#ifndef POINTER_VECTOR_H
#define POINTER_VECTOR_H

#include <vector>

template <typename T>
class PointerVector : public std::vector<T*>
{
public:
    ~PointerVector()
    {
        typename std::vector<T*>::iterator it;
        for(it = this->begin(); it != this->end(); ++it)
        {
            delete *it;
        }
        this->clear();
    }
};

#endif
