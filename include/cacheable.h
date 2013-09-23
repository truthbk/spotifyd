#ifndef _CACHEABLE_H
#define _CACHEABLE_H

class Cacheable {
    public:
        Cacheable() 
            : m_cached(false) {
        }
        virtual void cache(void) = 0;
        virtual void uncache(void) = 0;
        virtual bool is_cached(void) = 0;
    protected:
        bool m_cached;
};

#endif /* _CACHEABLE_H*/

