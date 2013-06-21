#include <cstdlib>
#include "xplodify_audio.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

void XplodifyAudio::initialize() {
    return;
}

void XplodifyAudio::run() {
    audio_fifo_t *af = aux;
    audio_fifo_data_t *afd;
    unsigned int frame = 0;
    ALCdevice *device = NULL;
    ALCcontext *context = NULL;
    ALuint buffers[NUM_BUFFERS];
    ALuint source;
    ALint processed;
    ALenum error;
    ALint rate;
    ALint channels;


    device = alcOpenDevice(NULL); /* Use the default device */
    if (!device) error_exit("failed to open device");
    context = alcCreateContext(device, NULL);
    alcMakeContextCurrent(context);
    alListenerf(AL_GAIN, 1.0f);
    alDistanceModel(AL_NONE);
    alGenBuffers((ALsizei)NUM_BUFFERS, buffers);
    alGenSources(1, &source);

    /* First prebuffer some audio */
    queue_buffer(source, af, buffers[0]);
    queue_buffer(source, af, buffers[1]);
    queue_buffer(source, af, buffers[2]);
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
            afd = audio_get(af);
            alGetBufferi(buffers[frame % 3], AL_FREQUENCY, &rate);
            alGetBufferi(buffers[frame % 3], AL_CHANNELS, &channels);
            if (afd->rate != rate || afd->channels != channels) {
                printf("rate or channel count changed, resetting\n");
                free(afd);
                break;
            }
            alBufferData(buffers[frame % 3], 
                    afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
                    afd->samples, 
                    afd->nsamples * afd->channels * sizeof(short), 
                    afd->rate);
            free(afd);
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
                afd->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, 
                afd->samples, 
                afd->nsamples * afd->channels * sizeof(short), 
                afd->rate);

        alSourceQueueBuffers(source, 1, &buffers[0]);
        queue_buffer(source, af, buffers[1]);
        queue_buffer(source, af, buffers[2]);
        frame = 0;
    }

    return;
}

void XplodifyAudio::enqueue(audio_data * d) {
    return;
}

void XplodifyAudio::dequeue() {
    return;
}
