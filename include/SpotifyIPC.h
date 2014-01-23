/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef SpotifyIPC_H
#define SpotifyIPC_H

#include <thrift/TDispatchProcessor.h>
#include "spotify_ipc_types.h"



class SpotifyIPCIf {
 public:
  virtual ~SpotifyIPCIf() {}
  virtual bool set_master() = 0;
  virtual bool set_slave() = 0;
  virtual void check_in(SpotifyIPCCredential& _return, const SpotifyIPCCredential& cred) = 0;
  virtual bool check_out() = 0;
  virtual bool login(const SpotifyIPCCredential& cred) = 0;
  virtual bool is_logged() = 0;
  virtual void logout() = 0;
  virtual void selectPlaylist(const std::string& playlist) = 0;
  virtual void selectPlaylistById(const int32_t plist_id) = 0;
  virtual void selectTrack(const std::string& track) = 0;
  virtual void selectTrackById(const int32_t track_id) = 0;
  virtual void play() = 0;
  virtual void stop() = 0;
  virtual void terminate_proc() = 0;
};

class SpotifyIPCIfFactory {
 public:
  typedef SpotifyIPCIf Handler;

  virtual ~SpotifyIPCIfFactory() {}

  virtual SpotifyIPCIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(SpotifyIPCIf* /* handler */) = 0;
};

class SpotifyIPCIfSingletonFactory : virtual public SpotifyIPCIfFactory {
 public:
  SpotifyIPCIfSingletonFactory(const boost::shared_ptr<SpotifyIPCIf>& iface) : iface_(iface) {}
  virtual ~SpotifyIPCIfSingletonFactory() {}

  virtual SpotifyIPCIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(SpotifyIPCIf* /* handler */) {}

 protected:
  boost::shared_ptr<SpotifyIPCIf> iface_;
};

class SpotifyIPCNull : virtual public SpotifyIPCIf {
 public:
  virtual ~SpotifyIPCNull() {}
  bool set_master() {
    bool _return = false;
    return _return;
  }
  bool set_slave() {
    bool _return = false;
    return _return;
  }
  void check_in(SpotifyIPCCredential& /* _return */, const SpotifyIPCCredential& /* cred */) {
    return;
  }
  bool check_out() {
    bool _return = false;
    return _return;
  }
  bool login(const SpotifyIPCCredential& /* cred */) {
    bool _return = false;
    return _return;
  }
  bool is_logged() {
    bool _return = false;
    return _return;
  }
  void logout() {
    return;
  }
  void selectPlaylist(const std::string& /* playlist */) {
    return;
  }
  void selectPlaylistById(const int32_t /* plist_id */) {
    return;
  }
  void selectTrack(const std::string& /* track */) {
    return;
  }
  void selectTrackById(const int32_t /* track_id */) {
    return;
  }
  void play() {
    return;
  }
  void stop() {
    return;
  }
  void terminate_proc() {
    return;
  }
};


class SpotifyIPC_set_master_args {
 public:

  SpotifyIPC_set_master_args() {
  }

  virtual ~SpotifyIPC_set_master_args() throw() {}


  bool operator == (const SpotifyIPC_set_master_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_set_master_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_set_master_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_set_master_pargs {
 public:


  virtual ~SpotifyIPC_set_master_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_set_master_result__isset {
  _SpotifyIPC_set_master_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_set_master_result__isset;

class SpotifyIPC_set_master_result {
 public:

  SpotifyIPC_set_master_result() : success(0) {
  }

  virtual ~SpotifyIPC_set_master_result() throw() {}

  bool success;

  _SpotifyIPC_set_master_result__isset __isset;

  void __set_success(const bool val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_set_master_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_set_master_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_set_master_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_set_master_presult__isset {
  _SpotifyIPC_set_master_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_set_master_presult__isset;

class SpotifyIPC_set_master_presult {
 public:


  virtual ~SpotifyIPC_set_master_presult() throw() {}

  bool* success;

  _SpotifyIPC_set_master_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class SpotifyIPC_set_slave_args {
 public:

  SpotifyIPC_set_slave_args() {
  }

  virtual ~SpotifyIPC_set_slave_args() throw() {}


  bool operator == (const SpotifyIPC_set_slave_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_set_slave_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_set_slave_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_set_slave_pargs {
 public:


  virtual ~SpotifyIPC_set_slave_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_set_slave_result__isset {
  _SpotifyIPC_set_slave_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_set_slave_result__isset;

class SpotifyIPC_set_slave_result {
 public:

  SpotifyIPC_set_slave_result() : success(0) {
  }

  virtual ~SpotifyIPC_set_slave_result() throw() {}

  bool success;

  _SpotifyIPC_set_slave_result__isset __isset;

  void __set_success(const bool val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_set_slave_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_set_slave_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_set_slave_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_set_slave_presult__isset {
  _SpotifyIPC_set_slave_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_set_slave_presult__isset;

class SpotifyIPC_set_slave_presult {
 public:


  virtual ~SpotifyIPC_set_slave_presult() throw() {}

  bool* success;

  _SpotifyIPC_set_slave_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _SpotifyIPC_check_in_args__isset {
  _SpotifyIPC_check_in_args__isset() : cred(false) {}
  bool cred;
} _SpotifyIPC_check_in_args__isset;

class SpotifyIPC_check_in_args {
 public:

  SpotifyIPC_check_in_args() {
  }

  virtual ~SpotifyIPC_check_in_args() throw() {}

  SpotifyIPCCredential cred;

  _SpotifyIPC_check_in_args__isset __isset;

  void __set_cred(const SpotifyIPCCredential& val) {
    cred = val;
  }

  bool operator == (const SpotifyIPC_check_in_args & rhs) const
  {
    if (!(cred == rhs.cred))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_check_in_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_check_in_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_check_in_pargs {
 public:


  virtual ~SpotifyIPC_check_in_pargs() throw() {}

  const SpotifyIPCCredential* cred;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_check_in_result__isset {
  _SpotifyIPC_check_in_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_check_in_result__isset;

class SpotifyIPC_check_in_result {
 public:

  SpotifyIPC_check_in_result() {
  }

  virtual ~SpotifyIPC_check_in_result() throw() {}

  SpotifyIPCCredential success;

  _SpotifyIPC_check_in_result__isset __isset;

  void __set_success(const SpotifyIPCCredential& val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_check_in_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_check_in_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_check_in_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_check_in_presult__isset {
  _SpotifyIPC_check_in_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_check_in_presult__isset;

class SpotifyIPC_check_in_presult {
 public:


  virtual ~SpotifyIPC_check_in_presult() throw() {}

  SpotifyIPCCredential* success;

  _SpotifyIPC_check_in_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class SpotifyIPC_check_out_args {
 public:

  SpotifyIPC_check_out_args() {
  }

  virtual ~SpotifyIPC_check_out_args() throw() {}


  bool operator == (const SpotifyIPC_check_out_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_check_out_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_check_out_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_check_out_pargs {
 public:


  virtual ~SpotifyIPC_check_out_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_check_out_result__isset {
  _SpotifyIPC_check_out_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_check_out_result__isset;

class SpotifyIPC_check_out_result {
 public:

  SpotifyIPC_check_out_result() : success(0) {
  }

  virtual ~SpotifyIPC_check_out_result() throw() {}

  bool success;

  _SpotifyIPC_check_out_result__isset __isset;

  void __set_success(const bool val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_check_out_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_check_out_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_check_out_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_check_out_presult__isset {
  _SpotifyIPC_check_out_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_check_out_presult__isset;

class SpotifyIPC_check_out_presult {
 public:


  virtual ~SpotifyIPC_check_out_presult() throw() {}

  bool* success;

  _SpotifyIPC_check_out_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

typedef struct _SpotifyIPC_login_args__isset {
  _SpotifyIPC_login_args__isset() : cred(false) {}
  bool cred;
} _SpotifyIPC_login_args__isset;

class SpotifyIPC_login_args {
 public:

  SpotifyIPC_login_args() {
  }

  virtual ~SpotifyIPC_login_args() throw() {}

  SpotifyIPCCredential cred;

  _SpotifyIPC_login_args__isset __isset;

  void __set_cred(const SpotifyIPCCredential& val) {
    cred = val;
  }

  bool operator == (const SpotifyIPC_login_args & rhs) const
  {
    if (!(cred == rhs.cred))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_login_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_login_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_login_pargs {
 public:


  virtual ~SpotifyIPC_login_pargs() throw() {}

  const SpotifyIPCCredential* cred;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_login_result__isset {
  _SpotifyIPC_login_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_login_result__isset;

class SpotifyIPC_login_result {
 public:

  SpotifyIPC_login_result() : success(0) {
  }

  virtual ~SpotifyIPC_login_result() throw() {}

  bool success;

  _SpotifyIPC_login_result__isset __isset;

  void __set_success(const bool val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_login_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_login_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_login_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_login_presult__isset {
  _SpotifyIPC_login_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_login_presult__isset;

class SpotifyIPC_login_presult {
 public:


  virtual ~SpotifyIPC_login_presult() throw() {}

  bool* success;

  _SpotifyIPC_login_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class SpotifyIPC_is_logged_args {
 public:

  SpotifyIPC_is_logged_args() {
  }

  virtual ~SpotifyIPC_is_logged_args() throw() {}


  bool operator == (const SpotifyIPC_is_logged_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_is_logged_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_is_logged_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_is_logged_pargs {
 public:


  virtual ~SpotifyIPC_is_logged_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_is_logged_result__isset {
  _SpotifyIPC_is_logged_result__isset() : success(false) {}
  bool success;
} _SpotifyIPC_is_logged_result__isset;

class SpotifyIPC_is_logged_result {
 public:

  SpotifyIPC_is_logged_result() : success(0) {
  }

  virtual ~SpotifyIPC_is_logged_result() throw() {}

  bool success;

  _SpotifyIPC_is_logged_result__isset __isset;

  void __set_success(const bool val) {
    success = val;
  }

  bool operator == (const SpotifyIPC_is_logged_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_is_logged_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_is_logged_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_is_logged_presult__isset {
  _SpotifyIPC_is_logged_presult__isset() : success(false) {}
  bool success;
} _SpotifyIPC_is_logged_presult__isset;

class SpotifyIPC_is_logged_presult {
 public:


  virtual ~SpotifyIPC_is_logged_presult() throw() {}

  bool* success;

  _SpotifyIPC_is_logged_presult__isset __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};


class SpotifyIPC_logout_args {
 public:

  SpotifyIPC_logout_args() {
  }

  virtual ~SpotifyIPC_logout_args() throw() {}


  bool operator == (const SpotifyIPC_logout_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_logout_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_logout_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_logout_pargs {
 public:


  virtual ~SpotifyIPC_logout_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_selectPlaylist_args__isset {
  _SpotifyIPC_selectPlaylist_args__isset() : playlist(false) {}
  bool playlist;
} _SpotifyIPC_selectPlaylist_args__isset;

class SpotifyIPC_selectPlaylist_args {
 public:

  SpotifyIPC_selectPlaylist_args() : playlist() {
  }

  virtual ~SpotifyIPC_selectPlaylist_args() throw() {}

  std::string playlist;

  _SpotifyIPC_selectPlaylist_args__isset __isset;

  void __set_playlist(const std::string& val) {
    playlist = val;
  }

  bool operator == (const SpotifyIPC_selectPlaylist_args & rhs) const
  {
    if (!(playlist == rhs.playlist))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_selectPlaylist_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_selectPlaylist_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_selectPlaylist_pargs {
 public:


  virtual ~SpotifyIPC_selectPlaylist_pargs() throw() {}

  const std::string* playlist;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_selectPlaylistById_args__isset {
  _SpotifyIPC_selectPlaylistById_args__isset() : plist_id(false) {}
  bool plist_id;
} _SpotifyIPC_selectPlaylistById_args__isset;

class SpotifyIPC_selectPlaylistById_args {
 public:

  SpotifyIPC_selectPlaylistById_args() : plist_id(0) {
  }

  virtual ~SpotifyIPC_selectPlaylistById_args() throw() {}

  int32_t plist_id;

  _SpotifyIPC_selectPlaylistById_args__isset __isset;

  void __set_plist_id(const int32_t val) {
    plist_id = val;
  }

  bool operator == (const SpotifyIPC_selectPlaylistById_args & rhs) const
  {
    if (!(plist_id == rhs.plist_id))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_selectPlaylistById_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_selectPlaylistById_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_selectPlaylistById_pargs {
 public:


  virtual ~SpotifyIPC_selectPlaylistById_pargs() throw() {}

  const int32_t* plist_id;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_selectTrack_args__isset {
  _SpotifyIPC_selectTrack_args__isset() : track(false) {}
  bool track;
} _SpotifyIPC_selectTrack_args__isset;

class SpotifyIPC_selectTrack_args {
 public:

  SpotifyIPC_selectTrack_args() : track() {
  }

  virtual ~SpotifyIPC_selectTrack_args() throw() {}

  std::string track;

  _SpotifyIPC_selectTrack_args__isset __isset;

  void __set_track(const std::string& val) {
    track = val;
  }

  bool operator == (const SpotifyIPC_selectTrack_args & rhs) const
  {
    if (!(track == rhs.track))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_selectTrack_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_selectTrack_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_selectTrack_pargs {
 public:


  virtual ~SpotifyIPC_selectTrack_pargs() throw() {}

  const std::string* track;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _SpotifyIPC_selectTrackById_args__isset {
  _SpotifyIPC_selectTrackById_args__isset() : track_id(false) {}
  bool track_id;
} _SpotifyIPC_selectTrackById_args__isset;

class SpotifyIPC_selectTrackById_args {
 public:

  SpotifyIPC_selectTrackById_args() : track_id(0) {
  }

  virtual ~SpotifyIPC_selectTrackById_args() throw() {}

  int32_t track_id;

  _SpotifyIPC_selectTrackById_args__isset __isset;

  void __set_track_id(const int32_t val) {
    track_id = val;
  }

  bool operator == (const SpotifyIPC_selectTrackById_args & rhs) const
  {
    if (!(track_id == rhs.track_id))
      return false;
    return true;
  }
  bool operator != (const SpotifyIPC_selectTrackById_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_selectTrackById_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_selectTrackById_pargs {
 public:


  virtual ~SpotifyIPC_selectTrackById_pargs() throw() {}

  const int32_t* track_id;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_play_args {
 public:

  SpotifyIPC_play_args() {
  }

  virtual ~SpotifyIPC_play_args() throw() {}


  bool operator == (const SpotifyIPC_play_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_play_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_play_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_play_pargs {
 public:


  virtual ~SpotifyIPC_play_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_stop_args {
 public:

  SpotifyIPC_stop_args() {
  }

  virtual ~SpotifyIPC_stop_args() throw() {}


  bool operator == (const SpotifyIPC_stop_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_stop_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_stop_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_stop_pargs {
 public:


  virtual ~SpotifyIPC_stop_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_terminate_proc_args {
 public:

  SpotifyIPC_terminate_proc_args() {
  }

  virtual ~SpotifyIPC_terminate_proc_args() throw() {}


  bool operator == (const SpotifyIPC_terminate_proc_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const SpotifyIPC_terminate_proc_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SpotifyIPC_terminate_proc_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};


class SpotifyIPC_terminate_proc_pargs {
 public:


  virtual ~SpotifyIPC_terminate_proc_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class SpotifyIPCClient : virtual public SpotifyIPCIf {
 public:
  SpotifyIPCClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  SpotifyIPCClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  bool set_master();
  void send_set_master();
  bool recv_set_master();
  bool set_slave();
  void send_set_slave();
  bool recv_set_slave();
  void check_in(SpotifyIPCCredential& _return, const SpotifyIPCCredential& cred);
  void send_check_in(const SpotifyIPCCredential& cred);
  void recv_check_in(SpotifyIPCCredential& _return);
  bool check_out();
  void send_check_out();
  bool recv_check_out();
  bool login(const SpotifyIPCCredential& cred);
  void send_login(const SpotifyIPCCredential& cred);
  bool recv_login();
  bool is_logged();
  void send_is_logged();
  bool recv_is_logged();
  void logout();
  void send_logout();
  void selectPlaylist(const std::string& playlist);
  void send_selectPlaylist(const std::string& playlist);
  void selectPlaylistById(const int32_t plist_id);
  void send_selectPlaylistById(const int32_t plist_id);
  void selectTrack(const std::string& track);
  void send_selectTrack(const std::string& track);
  void selectTrackById(const int32_t track_id);
  void send_selectTrackById(const int32_t track_id);
  void play();
  void send_play();
  void stop();
  void send_stop();
  void terminate_proc();
  void send_terminate_proc();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class SpotifyIPCProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<SpotifyIPCIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (SpotifyIPCProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_set_master(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_set_slave(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_check_in(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_check_out(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_login(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_is_logged(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_logout(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_selectPlaylist(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_selectPlaylistById(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_selectTrack(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_selectTrackById(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_play(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_stop(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_terminate_proc(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  SpotifyIPCProcessor(boost::shared_ptr<SpotifyIPCIf> iface) :
    iface_(iface) {
    processMap_["set_master"] = &SpotifyIPCProcessor::process_set_master;
    processMap_["set_slave"] = &SpotifyIPCProcessor::process_set_slave;
    processMap_["check_in"] = &SpotifyIPCProcessor::process_check_in;
    processMap_["check_out"] = &SpotifyIPCProcessor::process_check_out;
    processMap_["login"] = &SpotifyIPCProcessor::process_login;
    processMap_["is_logged"] = &SpotifyIPCProcessor::process_is_logged;
    processMap_["logout"] = &SpotifyIPCProcessor::process_logout;
    processMap_["selectPlaylist"] = &SpotifyIPCProcessor::process_selectPlaylist;
    processMap_["selectPlaylistById"] = &SpotifyIPCProcessor::process_selectPlaylistById;
    processMap_["selectTrack"] = &SpotifyIPCProcessor::process_selectTrack;
    processMap_["selectTrackById"] = &SpotifyIPCProcessor::process_selectTrackById;
    processMap_["play"] = &SpotifyIPCProcessor::process_play;
    processMap_["stop"] = &SpotifyIPCProcessor::process_stop;
    processMap_["terminate_proc"] = &SpotifyIPCProcessor::process_terminate_proc;
  }

  virtual ~SpotifyIPCProcessor() {}
};

class SpotifyIPCProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  SpotifyIPCProcessorFactory(const ::boost::shared_ptr< SpotifyIPCIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< SpotifyIPCIfFactory > handlerFactory_;
};

class SpotifyIPCMultiface : virtual public SpotifyIPCIf {
 public:
  SpotifyIPCMultiface(std::vector<boost::shared_ptr<SpotifyIPCIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~SpotifyIPCMultiface() {}
 protected:
  std::vector<boost::shared_ptr<SpotifyIPCIf> > ifaces_;
  SpotifyIPCMultiface() {}
  void add(boost::shared_ptr<SpotifyIPCIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  bool set_master() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->set_master();
    }
    return ifaces_[i]->set_master();
  }

  bool set_slave() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->set_slave();
    }
    return ifaces_[i]->set_slave();
  }

  void check_in(SpotifyIPCCredential& _return, const SpotifyIPCCredential& cred) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->check_in(_return, cred);
    }
    ifaces_[i]->check_in(_return, cred);
    return;
  }

  bool check_out() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->check_out();
    }
    return ifaces_[i]->check_out();
  }

  bool login(const SpotifyIPCCredential& cred) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->login(cred);
    }
    return ifaces_[i]->login(cred);
  }

  bool is_logged() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->is_logged();
    }
    return ifaces_[i]->is_logged();
  }

  void logout() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->logout();
    }
    ifaces_[i]->logout();
  }

  void selectPlaylist(const std::string& playlist) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->selectPlaylist(playlist);
    }
    ifaces_[i]->selectPlaylist(playlist);
  }

  void selectPlaylistById(const int32_t plist_id) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->selectPlaylistById(plist_id);
    }
    ifaces_[i]->selectPlaylistById(plist_id);
  }

  void selectTrack(const std::string& track) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->selectTrack(track);
    }
    ifaces_[i]->selectTrack(track);
  }

  void selectTrackById(const int32_t track_id) {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->selectTrackById(track_id);
    }
    ifaces_[i]->selectTrackById(track_id);
  }

  void play() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->play();
    }
    ifaces_[i]->play();
  }

  void stop() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->stop();
    }
    ifaces_[i]->stop();
  }

  void terminate_proc() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->terminate_proc();
    }
    ifaces_[i]->terminate_proc();
  }

};



#endif
