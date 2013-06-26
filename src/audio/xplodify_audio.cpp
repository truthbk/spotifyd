#include <cstdlib>
#include "xplodify_audio.h"


XplodifyAudio::XplodifyAudio() 
    : Runnable()
    , Lockable() {
    device = alcOpenDevice(NULL); /* Use the default device */
    if (!device) {
        //TODO: exit cleanly.
        exit(1);
    }

    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    alListenerf(AL_GAIN, 1.0f);
    alDistanceModel(AL_NONE);
    alGenBuffers((ALsizei)NUM_BUFFERS, buffers);
    alGenSources(1, &source);
}

void XplodifyAudio::queue_buffer(ALuint src, ALuint buffer) {
    boost::shared_ptr<audio_data> ad = get_samples();

    alBufferData(buffer,
            ad->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            &(ad->samples[0]), //elements are guaranteed to be contiguous!
            ad->n_samples * ad->channels * sizeof(short),
            ad->rate);
    alSourceQueueBuffers(src, 1, &buffer);

    return;
}

void XplodifyAudio::flush_queue() {
     lock();
     while(!audio_queue.empty()) {
         audio_queue.pop();
     }
     qlen = 0;
     unlock();
 }

void XplodifyAudio::run() {

    boost::shared_ptr<audio_data> ad;

    unsigned int frame = 0;


    /* First prebuffer some audio */
    queue_buffer(source, buffers[0]);
    queue_buffer(source, buffers[1]);
    queue_buffer(source, buffers[2]);
    for (;;) {

        alSourcePlay(source);
        for (;;) {
            /* Wait for some audio to play */
            do {
                alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
                usleep(100);
            } while (!processed);

            /* Remove old audio from the queue.. */
            alSourceUnqueueBuffers(source, 1, &buffers[frame % 3]);

            /* and queue some more audio */
            ad = get_samples();
            alGetBufferi(buffers[frame % 3], AL_FREQUENCY, &rate);
            alGetBufferi(buffers[frame % 3], AL_CHANNELS, &channels);
            if (ad->rate != rate || ad->channels != channels) {
                printf("rate or channel count changed, resetting\n");
                break;
            }
            alBufferData(buffers[frame % 3], 
                    ad->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
                    &(ad->samples[0]), 
                    ad->n_samples * ad->channels * sizeof(short), 
                    ad->rate);

            alSourceQueueBuffers(source, 1, &buffers[frame % 3]);

            if ((error = alcGetError(device)) != AL_NO_ERROR) {
                printf("openal al error: %d\n", error);
                exit(1);
            }
            frame++;
        }
        /* Format or rate changed, so we need to reset all buffers */
        alSourcei(source, AL_BUFFER, 0);
        alSourceStop(source);

        /* Make sure we don't lose the audio packet that caused the change */
        alBufferData(buffers[0], 
                ad->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
                &(ad->samples[0]), 
                ad->n_samples * ad->channels * sizeof(short), 
                ad->rate);

        alSourceQueueBuffers(source, 1, &buffers[0]);
        queue_buffer(source, buffers[1]);
        queue_buffer(source, buffers[2]);
        frame = 0;
    }

    return;
}

void XplodifyAudio::enqueue_samples(boost::shared_ptr<audio_data> d) {
    if(!d) {
        return;
    }

    lock();
    audio_queue.push(d);
    qlen += d->n_samples;
    unlock();
}

boost::shared_ptr<audio_data> XplodifyAudio::get_samples() {
    lock();
    while(audio_queue.empty()) 
        cond_wait();

    boost::shared_ptr<audio_data> ad = audio_queue.front();
    audio_queue.pop();
    qlen -= ad->n_samples;
    unlock();

    return ad;
}
