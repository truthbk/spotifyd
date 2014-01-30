#!/opt/local/bin/thrift --gen cpp

//DEFINITIONS FOR SERVICE TYPES

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
    SINGLE,
    LINEAR,
    REPEAT_ONE,
    REPEAT
}

struct SpotifyCredential {
    1: required string _username;
    2: required string _passwd;
    3: optional string _uuid; //This could potentially be a i32 token as well...
}

//TODO: We must implement exceptions.

typedef set<string> SpotifyPlaylistList
typedef list<SpotifyTrack> SpotifyPlaylist
typedef map<string, SpotifyPlaylist> SpotifyLibrary
//map of uuid SessionId for client and saved credentials.
typedef map<string, SpotifyCredential> SpotifySessions
