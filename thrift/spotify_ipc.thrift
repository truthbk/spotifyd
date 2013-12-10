#!/opt/local/bin/thrift --gen cpp


struct SpotifyIPCCredential {
    1: required string _username;
    2: required string _passwd;
    3: optional string _uuid; //This could potentially be a i32 token as well...
}

service SpotifyIPC {
	bool set_master();
	bool set_slave();
	SpotifyIPCCredential check_in(1: SpotifyIPCCredential cred);
	bool check_out();
	bool login(1: SpotifyIPCCredential cred);
	bool is_logged();
	oneway void logout();

	oneway void selectPlaylist(1: string playlist);
	oneway void selectPlaylistById(1: i32 plist_id);
	oneway void selectTrack(1: string track);
	oneway void selectTrackById(1: i32 track_id);

	oneway void play();
	oneway void stop();
	oneway void terminate_proc();
}
