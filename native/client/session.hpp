#pragma once

// C3 session seam (D-122, TASK-0060). IClientSession is the ONLY doorway
// between client presentation code and authoritative game state. Client code
// must never reach into verdigris::Simulation directly — LocalCoreSession is
// the single place allowed to know an in-process simulation exists, and
// RemoteProtocolSession is the single place allowed to know a socket exists.
//
// Invariants (docs/rebuild/NATIVE_PRODUCT_CONVERGENCE.md):
//  - remote mode runs NO second authoritative simulation;
//  - remote failure NEVER silently falls back to local play — connection
//    state is explicit and visible;
//  - no gameplay rules in transports, reducers, renderers, or HUD.

#include <string>
#include <vector>

#include "client_model.hpp"
#include "presentation_events.hpp"

namespace verdigris::client {

enum class ConnectionState {
  Idle,              // constructed, start() not called
  Connecting,        // TCP/upgrade in progress
  Connected,         // websocket established, login not yet acknowledged
  Ready,             // login accepted; model is authoritative
  Disconnected,      // orderly or dropped close after a connection existed
  Retrying,          // reconnect attempt in progress (0061+)
  Rejected,          // endpoint refused the connection
  ProtocolMismatch,  // endpoint spoke something other than the game protocol
};

const char* connection_state_label(ConnectionState state);

// Typed player intents. The session translates these into core Commands
// (local) or protocol envelopes (remote); presentation code never builds
// either representation itself.
struct ClientCommand {
  enum class Type {
    Login,          // guest login; `target` = guest identity, value!=0 => quickGuest
    Move,           // dx/dy in {-1,0,1}
    Aim,            // dx/dy direction
    UseAction,      // `target` = action name ("melee", ...)
    PickUp,         // `target` = ground item uuid; empty => underfoot
    Equip,          // `target` = item uuid
    EnterZone,      // `target` = route/node id
    Extract,
    // TASK-0145 Gate-B chronicles intents. The session translates them into
    // the exact accepted envelopes; presentation never builds wire payloads.
    FoundHouse,     // `target` = House display name
    CreateScion,    // `target` = Scion display name
    SelectScion,    // `target` = scion id; value!=0 => mortal oath
    SetOut,         // `target` = scion id (plain admission / road purse)
  };

  Type type = Type::Move;
  int dx = 0;
  int dy = 0;
  int value = 0;
  std::string target;

  static ClientCommand login(std::string guest_id, bool quick_guest);
  static ClientCommand move(int dx, int dy);
  static ClientCommand aim(int dx, int dy);
  static ClientCommand use_action(std::string action);
  static ClientCommand pick_up(std::string item_uuid);
  static ClientCommand equip(std::string item_uuid);
  static ClientCommand enter_zone(std::string node_id);
  static ClientCommand extract();
  static ClientCommand found_house(std::string house_name);
  static ClientCommand create_scion(std::string scion_name);
  static ClientCommand select_scion(std::string scion_id, bool mortal_oath);
  static ClientCommand set_out(std::string scion_id);
};

class IClientSession {
 public:
  virtual ~IClientSession() = default;

  // Establish the session (local: construct the simulation; remote: connect
  // and upgrade). Returns false and fills `error` on hard failure.
  virtual bool start(std::string* error = nullptr) = 0;
  virtual void shutdown() = 0;

  virtual void submit(const ClientCommand& command) = 0;

  // Pump the session: apply queued authoritative updates to the model and
  // stage presentation events. Cheap; call once per frame.
  virtual void poll() = 0;

  virtual ConnectionState connection_state() const = 0;
  virtual const ClientModel& model() const = 0;
  virtual std::vector<PresentationEvent> drain_events() = 0;
  virtual const std::string& last_error() const = 0;
};

}  // namespace verdigris::client
