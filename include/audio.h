/*
 * Copyright (c) 2006-2009 Spotify Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 * Audio output driver.
 *
 * This file is part of the libspotify examples suite.
 */
#ifndef _JUKEBOX_AUDIO_H_
#define _JUKEBOX_AUDIO_H_

#include <pthread.h>
#include <stdint.h>
#include "queue.h"


/* --- Types --- */
typedef struct audio_fifo_data {
    TAILQ_ENTRY(audio_fifo_data) link;
    int channels;
    int rate;
    int nsamples;
    int16_t samples[0];
} audio_fifo_data_t;

typedef struct audio_fifo {
    TAILQ_HEAD(, audio_fifo_data) q;
    int qlen;
    int prev_qlen;
    int reset;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} audio_fifo_t;



/* --- Functions --- */

typedef void (* audio_init_ptr)(audio_fifo_t *);

enum audio_arch {
    DUMMY = 1,
#ifdef _LINUX
    ALSA,
    OPENAL_ARCH,
#elif _OSX
    OPENAL_ARCH,
    AUDIOTOOLKIT,
#endif
};

extern audio_init_ptr audio_init;
#ifdef _LINUX
#ifdef HAS_ALSA
void alsa_audio_init(audio_fifo_t *af);
#endif
#ifdef HAS_OPENAL
void openal_audio_init(audio_fifo_t *af);
#endif
void dummy_audio_init(audio_fifo_t *af);
#elif _OSX
#ifdef HAS_OPENAL
void openal_audio_init(audio_fifo_t *af);
#elif HAS_AUDIOTOOLKIT
void osx_audio_init(audio_fifo_t *af);
#endif
#endif


extern void audio_fifo_flush(audio_fifo_t *af);
void set_audio_init( audio_init_ptr ptr );
audio_fifo_data_t* audio_get(audio_fifo_t *af);
void audio_fifo_set_reset(audio_fifo_t * af, int r);
int audio_fifo_get_reset(audio_fifo_t * af);

int set_audio(enum audio_arch arch);

#endif /* _JUKEBOX_AUDIO_H_ */
