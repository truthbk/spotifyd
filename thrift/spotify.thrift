#!/opt/local/bin/thrift --gen cpp

include "spotify_types.thrift"

service Spotify {
        bool is_service_ready();
        spotify_types.SpotifyMode server_mode();
        spotify_types.SpotifyCredential check_in(1: spotify_types.SpotifyCredential cred);
        bool check_out(1: spotify_types.SpotifyCredential cred);
        bool loginSession(1: spotify_types.SpotifyCredential cred);
        bool isLoggedIn(1: spotify_types.SpotifyCredential cred);
        i64 getStateTS(1: spotify_types.SpotifyCredential cred);
        i64 getSessionStateTS(1: spotify_types.SpotifyCredential cred);
        oneway void logoutSession(1: spotify_types.SpotifyCredential cred);

        oneway void sendCommand(1: spotify_types.SpotifyCredential cred, 2: spotify_types.SpotifyCmd cmd);
        spotify_types.SpotifyPlaylist search(1: spotify_types.SpotifyCredential cred, 2: spotify_types.SpotifySearch criteria);

        spotify_types.SpotifyPlaylistList getPlaylists(1: spotify_types.SpotifyCredential cred);
        spotify_types.SpotifyPlaylist getPlaylist(1: spotify_types.SpotifyCredential cred, 2: i32 plist_id);
        spotify_types.SpotifyPlaylist getPlaylistByName(1: spotify_types.SpotifyCredential cred, 2: string name);

        oneway void selectPlaylist(1: spotify_types.SpotifyCredential cred, 2: string playlist);
        oneway void selectPlaylistById(1: spotify_types.SpotifyCredential cred, 2: i32 plist_id);
        oneway void selectTrack(1: spotify_types.SpotifyCredential cred, 2: string track);
        oneway void selectTrackById(1: spotify_types.SpotifyCredential cred, 2: i32 track_id);

        bool merge2playlist(1: spotify_types.SpotifyCredential cred, 2: string pl, 3: spotify_types.SpotifyPlaylist tracks); //MERGE multiple tracks to PL
        bool add2playlist(1: spotify_types.SpotifyCredential cred, 2: string pl, 3: spotify_types.SpotifyTrack track); //MERGE single track to PL

        spotify_types.SpotifyTrack whats_playing(1: spotify_types.SpotifyCredential cred);

}
