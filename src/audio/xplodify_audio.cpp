#include <cstdlib>
#include "xplodify_audio.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

XplodifyAudio::XplodifyAudio() {
    device = alcOpenDevice(NULL); /* Use the default device */
    if (!device) {
        error_exit("failed to open device");
    }

    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    alListenerf(AL_GAIN, 1.0f);
    alDistanceModel(AL_NONE);
    alGenBuffers((ALsizei)NUM_BUFFERS, buffers);
    alGenSources(1, &source);
}

void XplodifyAudio::initialize() {
    return;
}

int XplodifyAudio::queue_buffer(ALuint src, ALuint buffer) {
    boost::shared_ptr<audio_data> ad = audio_queue.pop_front();

    alBufferData(buffer,
            ad->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
            &(ad->samples[0]), //elements are guaranteed to be contiguous!
            ad->n_samples * ad->channels * sizeof(short),
            ad->rate);
    alSourceQueueBuffers(src, 1, &buffer);
    return 1;
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
            ad = audio_queue.pop_front();
            alGetBufferi(buffers[frame % 3], AL_FREQUENCY, &rate);
            alGetBufferi(buffers[frame % 3], AL_CHANNELS, &channels);
            if (ad->rate != rate || ad->channels != channels) {
                printf("rate or channel count changed, resetting\n");
                free(ad);
                break;
            }
            alBufferData(buffers[frame % 3], 
                    ad->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
                    &(ad->samples[0]), 
                    ad->nsamples * ad->channels * sizeof(short), 
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
                ad->nsamples * ad->channels * sizeof(short), 
                ad->rate);

        alSourceQueueBuffers(source, 1, &buffers[0]);
        queue_buffer(source, buffers[1]);
        queue_buffer(source, buffers[2]);
        frame = 0;
    }

    return;
}

void XplodifyAudio::enqueue(boost::shared_ptr<audio_data>  d) {
    if(!d) {
        return;
    }

    audio_queue.push_back(d);
    n_samples += d->n_samples;
}

void XplodifyAudio::dequeue() {
    return;
}
