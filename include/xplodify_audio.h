#ifndef _XPLODIFY_AUDIO_H
#ifndef _XPLODIFY_AUDIO_H

#include <queue>
#include <cstdint>

#include "lockable.h"
#include "runnable.h"

struct audio_data {
    public:
        uint32_t channels;
        uint32_t rate;
        uint32_t channels;
        std::vector<int16_t> samples;
}

//This will probably end up being a superclass
class XplodifyAudio 
         : public Runnable
         , private Lockable {

    public:
        void enqueue_samples(audio_data * d);
        void dequeue();
    protected:
        void initialize();
        // implemeting runnable
        void run();
    private:
        std::queue<audio_data> audio_queue;
}

#endif //_XPLODIFY_AUDIO_HH
