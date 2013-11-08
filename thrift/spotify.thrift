#!/opt/local/bin/thrift --gen cpp

struct SpotifyIPCCredential {
    1: required string _username;
    2: required string _passwd;
    3: optional string _uuid; //This could potentially be a i32 token as well...
}

//TODO: We must implement exceptions.

//map of uuid SessionId for client and saved credentials.
typedef map<string, SpotifyIPCCredential> SpotifySessions

service SpotifyIPC {
        bool set_master(); //Change role to master. Master can reproduce music.
        bool set_slave(); //Change role to slave. Slave cannot reproduce music.
        bool login(1: SpotifyIPCCredential cred);

        oneway void select_playlist(1: SpotifyIPCCredential cred, 2: i32 plist_id);
        oneway void select_track(1: SpotifyIPCCredential cred, 2: i32 track_id);
	oneway void play();

}
