#include <errno.h>
#include <libgen.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <libspotify/api.h>

#include "audio.h"


/* --- Data --- */
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

static audio_fifo_t g_audiofifo;
static pthread_mutex_t g_notify_mutex;
static pthread_cond_t g_notify_cond;
static int g_notify_do;
static int g_playback_done;
static sp_session *g_sess;
static sp_playlist *g_jukeboxlist;
const char *g_listname;
static int g_remove_tracks = 0;
static sp_track *g_currenttrack;
static int g_track_index;

static short verbose = 0;

static void try_jukebox_start(void)
{
        sp_track *t;
        if (!g_jukeboxlist)
                return;

        if (!sp_playlist_num_tracks(g_jukeboxlist))
        {
                fprintf(stderr, "jukebox: No tracks in playlist. Waiting\n");
                return;
        }

        if (sp_playlist_num_tracks(g_jukeboxlist) < g_track_index)
        {
                fprintf(stderr, "jukebox: No more tracks in playlist. Waiting\n");
                return;
        }

        t = sp_playlist_track(g_jukeboxlist, g_track_index);

        if (g_currenttrack && t != g_currenttrack) 
        {
                /* Someone changed the current track */
                audio_fifo_flush(&g_audiofifo);
                sp_session_player_unload(g_sess);
                g_currenttrack = NULL;
        }

        if (!t)
        {
                return;
        }

        if (sp_track_error(t) != SP_ERROR_OK)
        {
                return;
        }

        if (g_currenttrack == t)
        {
            return;
        }

        g_currenttrack = t;

        printf("jukebox: Now playing \"%s\"...\n", sp_track_name(t));
        fflush(stdout);

        sp_session_player_load(g_sess, t);
        sp_session_player_play(g_sess, 1);
}

/* --------------------------  PLAYLIST CALLBACKS  ------------------------- */
static void tracks_added(sp_playlist *pl, sp_track * const *tracks,
                                 int num_tracks, int position, void *userdata)
{
        if (pl != g_jukeboxlist)
        {
                return;
        }
 
        printf("jukebox: %d tracks were added\n", num_tracks);
        fflush(stdout);
        try_jukebox_start();
}

static void tracks_removed(sp_playlist *pl, const int *tracks,
                                   int num_tracks, void *userdata)
{
        int i, k = 0;

        if (pl != g_jukeboxlist)
        {
            return;
        }

        for (i = 0; i < num_tracks; ++i)
        {
                if (tracks[i] < g_track_index)
                {
                        ++k;
                }
        }

        g_track_index -= k;

        printf("jukebox: %d tracks were removed\n", num_tracks);
        fflush(stdout);
        try_jukebox_start();
}

static void tracks_moved(sp_playlist *pl, const int *tracks,
                                 int num_tracks, int new_position, void *userdata)
{
        if (pl != g_jukeboxlist)
        {
                return;
        }

        printf("jukebox: %d tracks were moved around\n", num_tracks);
        fflush(stdout);
        
        try_jukebox_start();
}

static void playlist_renamed(sp_playlist *pl, void *userdata)
{
        const char *name = sp_playlist_name(pl);

        
        if (!strcasecmp(name, g_listname)) 
        {
                g_jukeboxlist = pl;
                g_track_index = 0;
                try_jukebox_start();
        
        } else if (g_jukeboxlist == pl) {
                printf("jukebox: current playlist renamed to \"%s\".\n", name);
                g_jukeboxlist = NULL;
                g_currenttrack = NULL;
                sp_session_player_unload(g_sess);
        }
}

static sp_playlist_callbacks pl_callbacks = {
        .tracks_added = &tracks_added,
        .tracks_removed = &tracks_removed,
        .tracks_moved = &tracks_moved,
        .playlist_renamed = &playlist_renamed,
};


/* --------------------  PLAYLIST CONTAINER CALLBACKS  --------------------- */
static void playlist_added(sp_playlistcontainer *pc, sp_playlist *pl,
                                   int position, void *userdata)
{
        sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

        if (!strcasecmp(sp_playlist_name(pl), g_listname))
        {
                g_jukeboxlist = pl;
                try_jukebox_start();
        }
}

static void playlist_removed(sp_playlistcontainer *pc, sp_playlist *pl,
                                     int position, void *userdata)
{
        sp_playlist_remove_callbacks(pl, &pl_callbacks, NULL);
}


static void container_loaded(sp_playlistcontainer *pc, void *userdata)
{
        fprintf(stderr, "jukebox: Rootlist synchronized (%d playlists)\n",
        sp_playlistcontainer_num_playlists(pc));
}


static sp_playlistcontainer_callbacks pc_callbacks = {
        .playlist_added = &playlist_added,
        .playlist_removed = &playlist_removed,
        .container_loaded = &container_loaded,
};


/* ---------------------------  SESSION CALLBACKS  ------------------------- */
static void logged_in(sp_session *sess, sp_error error)
{
        sp_playlistcontainer *pc = sp_session_playlistcontainer(sess);
        int i;

        if (SP_ERROR_OK != error) 
        {
                    fprintf(stderr, "jukebox: Login failed: %s\n",
                                                sp_error_message(error));
                            exit(2);
        }

        printf("jukebox: Looking at %d playlists\n", sp_playlistcontainer_num_playlists(pc));

        for (i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i)
        {
                sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);
                sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);

                if (!strcasecmp(sp_playlist_name(pl), g_listname))
                {
                    g_jukeboxlist = pl;
                    try_jukebox_start();
                }
        }

        if (!g_jukeboxlist)
        {
                printf("jukebox: No such playlist. Waiting for one to pop up...\n");
                fflush(stdout);
        }
}

static void notify_main_thread(sp_session *sess)
{
        pthread_mutex_lock(&g_notify_mutex);
        g_notify_do = 1;
        pthread_cond_signal(&g_notify_cond);
        pthread_mutex_unlock(&g_notify_mutex);
}

static int music_delivery(sp_session *sess, const sp_audioformat *format,
                                  const void *frames, int num_frames)
{
        audio_fifo_t *af = &g_audiofifo;
        audio_fifo_data_t *afd;
        size_t s;

        if (num_frames == 0)
        {
                return 0; // Audio discontinuity, do nothing
        }

        pthread_mutex_lock(&af->mutex);

        /* Buffer one second of audio */
        if (af->qlen > format->sample_rate)
        {
                pthread_mutex_unlock(&af->mutex);
                return 0;
        }

        s = num_frames * sizeof(int16_t) * format->channels;

        afd = malloc(sizeof(audio_fifo_data_t) + s);
        memcpy(afd->samples, frames, s);

        afd->nsamples = num_frames;
        afd->rate = format->sample_rate;
        afd->channels = format->channels;

        TAILQ_INSERT_TAIL(&af->q, afd, link);
        af->qlen += num_frames;

        pthread_cond_signal(&af->cond);
        pthread_mutex_unlock(&af->mutex);

        return num_frames;
}


static void end_of_track(sp_session *sess)
{
        pthread_mutex_lock(&g_notify_mutex);
        g_playback_done = 1;
        pthread_cond_signal(&g_notify_cond);
        pthread_mutex_unlock(&g_notify_mutex);
}


static void metadata_updated(sp_session *sess)
{
        try_jukebox_start();
}

static void play_token_lost(sp_session *sess)
{
        audio_fifo_flush(&g_audiofifo);

        if (g_currenttrack != NULL)
        {
                sp_session_player_unload(g_sess);
                g_currenttrack = NULL;
        }
}

static sp_session_callbacks session_callbacks = {
        .logged_in = &logged_in,
        .notify_main_thread = &notify_main_thread,
        .music_delivery = &music_delivery,
        .metadata_updated = &metadata_updated,
        .play_token_lost = &play_token_lost,
        .log_message = NULL,
        .end_of_track = &end_of_track,
};

static sp_session_config spconfig = {
        .api_version = SPOTIFY_API_VERSION,
        .cache_location = "tmp",
        .settings_location = "tmp",
        .application_key = g_appkey,
        .application_key_size = 0, // Set in main()
        .user_agent = "spotifyd",
        .callbacks = &session_callbacks,
        NULL,
};
/* -------------------------  END SESSION CALLBACKS  ----------------------- */


static void track_ended(void)
{
        int tracks = 0;

        if (g_currenttrack)
        {
                g_currenttrack = NULL;
                sp_session_player_unload(g_sess);
                if (g_remove_tracks)
                {
                        sp_playlist_remove_tracks(g_jukeboxlist, &tracks, 1);
                } else {
                        ++g_track_index;
                        try_jukebox_start();
                }
        }
}

/* Daemon signal handler...  */
static void signal_handler(int sig) 
{
    switch(sig) {
        case SIGHUP:
                syslog(LOG_WARNING, "Received SIGHUP signal.");
                break;
        case SIGTERM:
                syslog(LOG_WARNING, "Received SIGTERM signal.");
                break;
        default:
                syslog(LOG_WARNING, "Unhandled signal (%d) %s", strsignal(sig));
                break;
    }
}

static void usage(const char *progname)
{
        fprintf(stderr, "usage: %s -u <username> -p <password> -l <listname> [-d]\n", progname);
        fprintf(stderr, "warning: -d will delete the tracks played from the list!\n");
}

int main(int argc, char **argv)
{
        sp_session *sp;
        sp_error err;

        short fg = 0;
        int next_timeout = 0;
        const char *username = NULL;
        const char *password = NULL;
        int opt;

        pid_t pid, ppid;

        while ((opt = getopt(argc, argv, "u:p:l:dfv")) != EOF)
        {
                switch (opt) 
                {
                        case 'u':
                                username = optarg;
                                break;

                        case 'p':
                                password = optarg;
                                break;

                        case 'l':
                                g_listname = optarg;
                                break;

                        case 'd':
                                g_remove_tracks = 1;
                                break;

                        case 'f':
                                fg = 1;
                                break;

                        case 'v':
                                verbose = 1;
                                break;

                        default:
                                usage(basename(argv[0]));
                                exit(1);

                }
        }
    // Setup syslog logging - see SETLOGMASK(3)

#if defined(DEBUG)
        setlogmask(LOG_UPTO(LOG_DEBUG));
        openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
#else
        setlogmask(LOG_UPTO(LOG_INFO));
        openlog(DAEMON_NAME, LOG_CONS, LOG_USER);
#endif


        if (!username || !password || !g_listname)
        {
                usage(basename(argv[0]));
                exit(1);
        }

        audio_init(&g_audiofifo);

        /* Create session */
        spconfig.application_key_size = g_appkey_size;
        err = sp_session_create(&spconfig, &sp);

        if (SP_ERROR_OK != err)
        {
                fprintf(stderr, "Unable to create session: %s\n",
                sp_error_message(err));
                exit(1);
        }

        g_sess = sp;

        pthread_mutex_init(&g_notify_mutex, NULL);
        pthread_cond_init(&g_notify_cond, NULL);

        sp_playlistcontainer_add_callbacks(
                sp_session_playlistcontainer(g_sess),
                &pc_callbacks,
                NULL);

        sp_session_login(sp, username, password, 0);
        pthread_mutex_lock(&g_notify_mutex);

        if(!fg)
        {
                pid = fork();

                if(pid < 0)
                {
                        exit(EXIT_FAILURE);
                }

                if(pid > 0)
                {
                        exit(EXIT_SUCCESS);
                }

                umask(0);

                sid = setsid();
                if (sid < 0)
                {
                        /* Log the failure */
                        exit(EXIT_FAILURE);
                }


                /* Change the current working directory */
                if ((chdir("/")) < 0)
                {
                        /* Log the failure */
                        exit(EXIT_FAILURE);
                }

                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
        }

        /* So... here comes the fun... */
        for (;;) {
                /*
                 *
                 * We listen, using thrift/libevent for incoming requests (ie. change
                 * playlist, play, pause, next, previous, search,...).
                 *
                 * We need a dedicated thread for reproduction.
                 *
                 * We need behavior to be Async, as request may take variable amounts of time
                 * to complete.
                 *
                 * One session per daemon, maybe we can eventually change this if we eventually
                 * stream music. But ATM, it doesnt make much sense to manage multiple sessions.
                 *
                 *
                 * */

                if (next_timeout == 0) {
                        while(!g_notify_do && !g_playback_done)
                        {
                                pthread_cond_wait(&g_notify_cond, &g_notify_mutex);
                        }
                } else {
                        struct timespec ts;
#if _POSIX_TIMERS > 0
                        clock_gettime(CLOCK_REALTIME, &ts);
#else
                        struct timeval tv;
                        gettimeofday(&tv, NULL);
                        TIMEVAL_TO_TIMESPEC(&tv, &ts);
#endif
                        ts.tv_sec += next_timeout / 1000;
                        ts.tv_nsec += (next_timeout % 1000) * 1000000;
                        pthread_cond_timedwait(&g_notify_cond, &g_notify_mutex, &ts);
                }

                g_notify_do = 0;
                pthread_mutex_unlock(&g_notify_mutex);

                if (g_playback_done) 
                {
                        track_ended();
                        g_playback_done = 0;
                }

                do {
                        sp_session_process_events(sp, &next_timeout);
                } while (next_timeout == 0);

                pthread_mutex_lock(&g_notify_mutex);
        }

        return 0;
}

