#!/opt/local/bin/thrift --gen cpp

struct SpotifyTrack {
    1: required i32 _id;
    2: required string _name;
    3: required string _artist;
    4: optional byte _minutes;
    5: optional byte _seconds;
    6: optional byte _popularity;
    7: optional string _genre;
    8: optional bool _starred;
}

struct SpotifySearch {
    1: optional string artist;
    2: optional string album;
    3: optional string track;
}

enum SpotifyCmd {
    PLAY = 1,
    PAUSE,
    NEXT,
    PREV,
    RAND,
    LINEAR,
    REPEAT_ONE,
    REPEAT
}

struct SpotifyCredential {
    1: required string _username;
    2: required string _passwd;
    3: optional string _uid; //This could potentially be a i32 token as well...
}

//TODO: We must implement exceptions.

typedef set<string> SpotifyPlaylistList
typedef set<SpotifyTrack> SpotifyPlaylist
typedef map<string, SpotifyPlaylist> SpotifyLibrary
//map of uuid SessionId for client and saved credentials.
typedef map<string, SpotifyCredential> SpotifySessions

service Spotify {
        SpotifyCredential loginSession(1: SpotifyCredential cred);
        oneway void logoutSession(1: SpotifyCredential cred);

        oneway void sendCommand(1: SpotifyCredential cred, 2: SpotifyCmd cmd);
        SpotifyPlaylist search(1: SpotifyCredential cred, 2: SpotifySearch criteria);

        SpotifyPlaylistList getPlaylists(1: SpotifyCredential cred);
        SpotifyPlaylist getPlaylist(1: SpotifyCredential cred, 2: i32 plist_id);
        SpotifyPlaylist getPlaylistByName(1: SpotifyCredential cred, 2: string name);

        oneway void selectPlaylist(1: SpotifyCredential cred, 2: string playlist);
        oneway void selectPlaylistById(1: SpotifyCredential cred, 2: i32 plist_id);
        bool merge2playlist(1: SpotifyCredential cred, 2: string pl, 3: SpotifyPlaylist tracks); //MERGE multiple tracks to PL
        bool add2playlist(1: SpotifyCredential cred, 2: string pl, 3: SpotifyTrack track); //MERGE single track to PL

        SpotifyTrack whats_playing();

}
