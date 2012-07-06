// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Spotify.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

class SpotifyHandler : virtual public SpotifyIf {
 public:
  SpotifyHandler() {
    // Your initialization goes here
  }

  void loginSession(SpotifyCredential& _return, const SpotifyCredential& cred) {
    // Your implementation goes here
    printf("loginSession\n");
  }

  void logoutSession(const SpotifyCredential& cred) {
    // Your implementation goes here
    printf("logoutSession\n");
  }

  void sendCommand(const SpotifyCredential& cred, const SpotifyCmd::type cmd) {
    // Your implementation goes here
    printf("sendCommand\n");
  }

  void search(SpotifyPlaylist& _return, const SpotifyCredential& cred, const SpotifySearch& criteria) {
    // Your implementation goes here
    printf("search\n");
  }

  void getPlaylists(SpotifyPlaylistList& _return, const SpotifyCredential& cred) {
    // Your implementation goes here
    printf("getPlaylists\n");
  }

  void getPlaylist(SpotifyPlaylist& _return, const SpotifyCredential& cred, const int32_t plist_id) {
    // Your implementation goes here
    printf("getPlaylist\n");
  }

  void getPlaylistByName(SpotifyPlaylist& _return, const SpotifyCredential& cred, const std::string& name) {
    // Your implementation goes here
    printf("getPlaylistByName\n");
  }

  void selectPlaylist(const SpotifyCredential& cred, const std::string& playlist) {
    // Your implementation goes here
    printf("selectPlaylist\n");
  }

  void selectPlaylistById(const SpotifyCredential& cred, const int32_t plist_id) {
    // Your implementation goes here
    printf("selectPlaylistById\n");
  }

  bool merge2playlist(const SpotifyCredential& cred, const std::string& pl, const SpotifyPlaylist& tracks) {
    // Your implementation goes here
    printf("merge2playlist\n");
  }

  bool add2playlist(const SpotifyCredential& cred, const std::string& pl, const SpotifyTrack& track) {
    // Your implementation goes here
    printf("add2playlist\n");
  }

  void whats_playing(SpotifyTrack& _return) {
    // Your implementation goes here
    printf("whats_playing\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<SpotifyHandler> handler(new SpotifyHandler());
  shared_ptr<TProcessor> processor(new SpotifyProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

