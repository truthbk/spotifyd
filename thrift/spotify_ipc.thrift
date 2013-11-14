#!/opt/local/bin/thrift --gen cpp


struct SpotifyIPCCredential {
    1: required string _username;
    2: required string _passwd;
    3: optional string _uuid; //This could potentially be a i32 token as well...
}

service SpotifyIPC {
	bool set_master();
	bool set_slave();
	oneway void login(1: SpotifyCredential cred);
	bool  logout();
	SpotifyiIPCCredential login(1: SpotifyCredential cred);
	bool is_logged(1: SpotifyCredential cred);

	oneway void selectPlaylist(1: SpotifyCredential cred, 2: string playlist);
	oneway void selectPlaylistById(1: SpotifyCredential cred, 2: i32 plist_id);
	oneway void selectTrack(1: SpotifyCredential cred, 2: string track);
	oneway void selectTrackById(1: SpotifyCredential cred, 2: i32 track_id);

	oneway void play();
	oneway void stop();
}
