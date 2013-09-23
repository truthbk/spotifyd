#ifndef _CACHEABLE_H
#define _CACHEABLE_H

class Cacheable {
    public:
        virtual void cache(void) = 0;
        virtual void uncache(void) = 0;
        virtual bool is_cached(void) = 0;
    private:
        bool cached;
};

#endif /* _CACHEABLE_H*/

