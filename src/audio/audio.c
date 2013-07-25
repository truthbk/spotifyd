/*
 * Copyright (c) 2010 Spotify Ltd
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
 * Audio helper functions.
 *
 * This file is part of the libspotify examples suite.
 */

#include "audio.h"
#include <stdlib.h>
#include <stdio.h>

audio_init_ptr audio_init;

audio_fifo_data_t* audio_get(audio_fifo_t *af)
{
    audio_fifo_data_t *afd;
    pthread_mutex_lock(&af->mutex);
  
    while (!(afd = TAILQ_FIRST(&af->q)))
	pthread_cond_wait(&af->cond, &af->mutex);
  
    TAILQ_REMOVE(&af->q, afd, link);
    af->qlen -= afd->nsamples;
  
    pthread_mutex_unlock(&af->mutex);
    return afd;
}

void audio_fifo_set_reset(audio_fifo_t * af, int r)
{
    pthread_mutex_lock(&af->mutex);
    af->reset = r;
    pthread_mutex_unlock(&af->mutex);
}

int audio_fifo_get_reset(audio_fifo_t * af)
{
    int res;
    pthread_mutex_lock(&af->mutex);
    res = af->reset; 
    pthread_mutex_unlock(&af->mutex);

    return res;
}

void audio_fifo_flush(audio_fifo_t *af)
{
    audio_fifo_data_t *afd;


    pthread_mutex_lock(&af->mutex);

    while((afd = TAILQ_FIRST(&af->q))) {
	TAILQ_REMOVE(&af->q, afd, link);
	free(afd);
    }

    af->qlen = 0;
    pthread_mutex_unlock(&af->mutex);
#ifdef _DEBUG
    fprintf(stdout, "FIFO Flushed\n");
#endif
}

void set_audio_init( audio_init_ptr ptr ) {
    if(!ptr)
    {
#ifdef _LINUX
        audio_init = dummy_audio_init;
#elif _OSX
#ifdef HAS_OPENAL
        audio_init = openal_audio_init;
#elif HAS_AUDIOTOOLKIT
        audio_init = osx_audio_init;
#endif
#endif
        return;
    }
    audio_init = ptr;
}

int set_audio(enum audio_arch arch) 
{
    int ret = 0;

    switch(arch)
    {
#ifdef _LINUX
#ifdef HAS_ALSA
	case ALSA:
	    set_audio_init(alsa_audio_init);
	    break;
#endif
#ifdef HAS_OPENAL
	case OPENAL_ARCH:
	    set_audio_init(openal_audio_init);
	    break;
#endif
	default:
	    set_audio_init(dummy_audio_init);
	    break;
#endif
#ifdef _OSX
#ifdef HAS_AUDIOTOOLKIT
	case AUDIOTOOLKIT:
	    set_audio_init(osx_audio_init);
	    break;
#endif
#ifdef HAS_OPENAL
	case OPENAL_ARCH:
	    set_audio_init(openal_audio_init);
#endif
	default:
	    break;
#endif

    }

    return ret;
}
