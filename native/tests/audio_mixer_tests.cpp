// TASK-0157: dedicated acceptance tests for the backend-neutral procedural
// audio scheduler foundation. Proves stable event-to-cue mapping,
// deterministic ordering, bus mute/volume state, bounded voice-cap eviction
// (steal-oldest), unknown-event silence, and byte-identical serialized
// schedules across two independent runs. The suite is fully headless: the
// only sink is the in-process recorder, so a pass here claims scheduled DATA
// only, never audible playback.

#include <cstdio>
#include <string>
#include <vector>

#include "audio_mixer.hpp"
#include "cue_spec.hpp"
#include "event_cues.hpp"

namespace {

int failures = 0;

void check(bool ok, const char* label) {
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
  if (!ok) ++failures;
}

using verdigris::audio::AudioMixer;
using verdigris::audio::Bus;
using verdigris::audio::CueParams;
using verdigris::audio::CueSpec;
using verdigris::audio::PriorityClass;
using verdigris::audio::RecordingSink;
using verdigris::audio::Waveform;
using verdigris::client::PresentationEvent;
using verdigris::client::PresentationEventType;

PresentationEvent make_event(PresentationEventType type, bool critical,
                             const char* text) {
  PresentationEvent event;
  event.type = type;
  event.actor_id = "actor-1";
  event.text = text;
  event.critical = critical;
  return event;
}

CueSpec make_cue(const char* id, Bus bus, PriorityClass priority,
                 std::uint64_t tick) {
  CueSpec spec;
  spec.cue_id = id;
  spec.bus = bus;
  spec.priority = priority;
  spec.scheduled_tick = tick;
  spec.params = CueParams{Waveform::Sine, 220, 110, 90, 480};
  return spec;
}

std::vector<CueSpec> scripted_schedule(RecordingSink* sink) {
  AudioMixer mixer(*sink);
  check(mixer.ingest(make_event(PresentationEventType::DamageApplied, false,
                                "outgoing"),
                     10),
        "byte-identical: ordinary hit ingests");
  check(mixer.ingest(make_event(PresentationEventType::DamageApplied, true,
                                "outgoing"),
                     10),
        "byte-identical: critical hit ingests");
  check(mixer.ingest(make_event(PresentationEventType::ActorDied, false, ""),
                     5),
        "byte-identical: enemy defeat ingests");
  check(mixer.ingest(make_event(PresentationEventType::ScionLost, false, ""),
                     5),
        "byte-identical: Scion loss ingests");
  check(mixer.ingest(
            make_event(PresentationEventType::BuffExpired, false, "war-cry"),
            7),
        "byte-identical: war-cry expiry ingests");
  CueSpec menu_loop = make_cue("menu-loop", Bus::Music, PriorityClass::Ui, 12);
  menu_loop.params = CueParams{Waveform::Sine, 262, 262, 1000, 300};
  mixer.submit(menu_loop);
  return mixer.drain_scheduled();
}

void stable_event_to_cue_mapping() {
  struct Expected {
    PresentationEventType type;
    bool critical;
    const char* text;
    const char* cue_id;
    Bus bus;
    PriorityClass priority;
    Waveform waveform;
    int start_hz;
    int end_hz;
    int duration_ms;
    int gain_permille;
  };
  const Expected table[] = {
      {PresentationEventType::DamageApplied, false, "outgoing", "hit",
       Bus::Sfx, PriorityClass::PlayerFeedback, Waveform::Sine, 220, 110, 90,
       480},
      {PresentationEventType::DamageApplied, true, "outgoing", "crit",
       Bus::Sfx, PriorityClass::PlayerFeedback, Waveform::Square, 440, 110,
       150, 640},
      {PresentationEventType::ActorDied, false, "", "kill", Bus::Sfx,
       PriorityClass::World, Waveform::Sawtooth, 196, 49, 240, 560},
      {PresentationEventType::ScionLost, false, "", "scion-lost", Bus::Sfx,
       PriorityClass::PlayerFeedback, Waveform::Sine, 165, 41, 900, 700},
      {PresentationEventType::BuffExpired, false, "war-cry", "warcry-expire",
       Bus::Sfx, PriorityClass::PlayerFeedback, Waveform::Sine, 392, 262, 300,
       420},
  };
  for (const Expected& row : table) {
    CueSpec a;
    CueSpec b;
    const bool mapped_a =
        verdigris::audio::cue_for_event(make_event(row.type, row.critical,
                                                   row.text),
                                        &a);
    const bool mapped_b =
        verdigris::audio::cue_for_event(make_event(row.type, row.critical,
                                                   row.text),
                                        &b);
    const bool stable = mapped_a && mapped_b &&
                        verdigris::audio::serialize_schedule({a}) ==
                            verdigris::audio::serialize_schedule({b});
    check(stable, "mapping: repeated translation is stable");
    const bool shape =
        a.cue_id == row.cue_id && a.bus == row.bus &&
        a.priority == row.priority &&
        a.params.waveform == row.waveform &&
        a.params.start_hz == row.start_hz && a.params.end_hz == row.end_hz &&
        a.params.duration_ms == row.duration_ms &&
        a.params.gain_permille == row.gain_permille;
    check(shape, "mapping: representative cue shape matches the pinned table");
  }
}

void unknown_events_are_silent() {
  RecordingSink sink;
  AudioMixer mixer(sink);
  const PresentationEventType unknown[] = {
      PresentationEventType::ConnectionEstablished,
      PresentationEventType::ConnectionLost,
      PresentationEventType::SessionReady,
      PresentationEventType::AttackStarted,
      PresentationEventType::ItemDropped,
      PresentationEventType::ItemPickedUp,
      PresentationEventType::ItemEquipped,
      PresentationEventType::ExtractionCompleted,
      PresentationEventType::ScionDied,
      PresentationEventType::Telegraph,
      PresentationEventType::Message,
      PresentationEventType::ProtocolError,
  };
  bool any_mapped = false;
  for (PresentationEventType type : unknown) {
    any_mapped |= mixer.ingest(make_event(type, false, ""), 1);
  }
  // Known type, unknown text discriminator: still silence.
  any_mapped |= mixer.ingest(
      make_event(PresentationEventType::BuffExpired, false, "shield"), 1);
  check(!any_mapped, "silence: no unknown event schedules a cue");
  const std::vector<CueSpec> voiced = mixer.drain_scheduled();
  check(voiced.empty(), "silence: drained schedule is empty");
  check(sink.cues().empty(), "silence: recording sink received nothing");
}

void deterministic_ordering() {
  RecordingSink sink;
  AudioMixer mixer(sink);
  // Interleave ticks so arrival order differs from schedule order.
  mixer.ingest(make_event(PresentationEventType::DamageApplied, false, ""), 20);
  mixer.ingest(make_event(PresentationEventType::ActorDied, false, ""), 5);
  mixer.ingest(make_event(PresentationEventType::DamageApplied, true, ""), 20);
  mixer.ingest(make_event(PresentationEventType::ScionLost, false, ""), 5);
  mixer.ingest(
      make_event(PresentationEventType::BuffExpired, false, "war-cry"), 11);
  const std::vector<CueSpec> voiced = mixer.drain_scheduled();
  check(voiced.size() == 5, "ordering: every known cue is voiced once");
  bool ordered = voiced.size() == 5;
  for (std::size_t i = 1; ordered && i < voiced.size(); ++i) {
    ordered = voiced[i - 1].scheduled_tick <= voiced[i].scheduled_tick;
  }
  check(ordered, "ordering: voiced cues ascend by scheduled tick");
  check(voiced.size() == 5 && voiced[0].sequence == 2 && voiced[1].sequence == 4,
        "ordering: ties resolve by mixer arrival sequence");
}

void bus_mute_and_volume() {
  RecordingSink sink;
  AudioMixer mixer(sink);
  check(mixer.bus_volume(Bus::Sfx) == 1000 && !mixer.bus_muted(Bus::Sfx),
        "bus: default is full volume, unmuted");
  mixer.set_bus_volume(Bus::Sfx, 500);
  check(mixer.bus_volume(Bus::Sfx) == 500, "bus: volume setter stores permille");
  mixer.submit(make_cue("hit", Bus::Sfx, PriorityClass::PlayerFeedback, 1));
  std::vector<CueSpec> voiced = mixer.drain_scheduled();
  check(voiced.size() == 1 && voiced[0].effective_gain_permille == 240,
        "bus: effective gain scales authored gain exactly");
  mixer.set_bus_muted(Bus::Music, true);
  mixer.submit(make_cue("hit", Bus::Sfx, PriorityClass::PlayerFeedback, 2));
  mixer.submit(make_cue("menu-loop", Bus::Music, PriorityClass::Ui, 2));
  voiced = mixer.drain_scheduled();
  check(voiced.size() == 1 && voiced[0].bus == Bus::Sfx,
        "bus: muted bus is silenced while the other still voices");
  mixer.set_bus_muted(Bus::Music, false);
  mixer.set_bus_volume(Bus::Music, 0);
  mixer.submit(make_cue("menu-loop", Bus::Music, PriorityClass::Ui, 3));
  voiced = mixer.drain_scheduled();
  check(voiced.empty(), "bus: zero volume silences without mute");
  mixer.set_bus_volume(Bus::Music, 5000);
  check(mixer.bus_volume(Bus::Music) == 1000, "bus: volume clamps above full");
  mixer.set_bus_volume(Bus::Music, -7);
  check(mixer.bus_volume(Bus::Music) == 0, "bus: volume clamps below zero");
}

void voice_cap_steals_oldest() {
  RecordingSink sink;
  AudioMixer mixer(sink, 3, 2);
  for (int i = 1; i <= 4; ++i) {
    mixer.ingest(make_event(PresentationEventType::DamageApplied, false, ""),
                 static_cast<std::uint64_t>(i));
  }
  std::vector<CueSpec> voiced = mixer.drain_scheduled();
  check(voiced.size() == 3, "cap: voice cap bounds the drained schedule");
  check(voiced.size() == 3 && voiced[0].sequence == 2 && voiced[1].sequence == 3,
        "cap: steal-oldest evicts the earliest same-priority voice");

  RecordingSink priority_sink;
  AudioMixer priority_mixer(priority_sink, 2, 2);
  priority_mixer.submit(
      make_cue("world-a", Bus::Sfx, PriorityClass::World, 1));
  priority_mixer.submit(
      make_cue("world-b", Bus::Sfx, PriorityClass::World, 2));
  priority_mixer.submit(make_cue("ui-c", Bus::Sfx, PriorityClass::Ui, 3));
  voiced = priority_mixer.drain_scheduled();
  bool kept_priority = voiced.size() == 2;
  for (const CueSpec& cue : voiced) {
    if (cue.cue_id == "world-a") kept_priority = false;
  }
  check(kept_priority, "cap: higher-priority newcomer steals the oldest lower-priority voice");

  RecordingSink protect_sink;
  AudioMixer protect_mixer(protect_sink, 2, 2);
  protect_mixer.submit(make_cue("ui-a", Bus::Sfx, PriorityClass::Ui, 1));
  protect_mixer.submit(make_cue("ui-b", Bus::Sfx, PriorityClass::Ui, 2));
  protect_mixer.submit(make_cue("world-c", Bus::Sfx, PriorityClass::World, 3));
  voiced = protect_mixer.drain_scheduled();
  check(voiced.size() == 2 && voiced[0].cue_id == "ui-a" &&
            voiced[1].cue_id == "ui-b",
        "cap: a lower-priority newcomer never steals a higher-priority voice");

  RecordingSink music_sink;
  AudioMixer music_mixer(music_sink, 8, 2);
  for (int i = 1; i <= 3; ++i) {
    music_mixer.submit(make_cue("menu-loop", Bus::Music, PriorityClass::Ui,
                                static_cast<std::uint64_t>(i)));
  }
  music_mixer.submit(make_cue("hit", Bus::Sfx, PriorityClass::PlayerFeedback, 4));
  voiced = music_mixer.drain_scheduled();
  check(voiced.size() == 3 && voiced.back().cue_id == "hit",
        "cap: music bus cap is enforced independently of the sfx bus");
}

void byte_identical_serialization_across_runs() {
  RecordingSink run_a;
  const std::vector<CueSpec> voiced_a = scripted_schedule(&run_a);
  RecordingSink run_b;
  const std::vector<CueSpec> voiced_b = scripted_schedule(&run_b);

  const std::string bytes_a = verdigris::audio::serialize_schedule(voiced_a);
  const std::string bytes_b = verdigris::audio::serialize_schedule(voiced_b);
  check(bytes_a == bytes_b,
        "serialization: two independent runs are byte-identical");

  static const char* kExpected[] = {
      "cue[000003] tick=5 bus=sfx prio=world id=kill wave=sawtooth "
      "196->49Hz 240ms gain=560 effective=560\n",
      "cue[000004] tick=5 bus=sfx prio=player id=scion-lost wave=sine "
      "165->41Hz 900ms gain=700 effective=700\n",
      "cue[000005] tick=7 bus=sfx prio=player id=warcry-expire wave=sine "
      "392->262Hz 300ms gain=420 effective=420\n",
      "cue[000001] tick=10 bus=sfx prio=player id=hit wave=sine 220->110Hz "
      "90ms gain=480 effective=480\n",
      "cue[000002] tick=10 bus=sfx prio=player id=crit wave=square 440->110Hz "
      "150ms gain=640 effective=640\n",
      "cue[000006] tick=12 bus=music prio=ui id=menu-loop wave=sine "
      "262->262Hz 1000ms gain=300 effective=300\n",
  };
  std::string expected;
  for (const char* line : kExpected) expected += line;
  check(bytes_a == expected,
        "serialization: schedule matches the pinned canonical form");

  check(run_a.cues().size() == voiced_a.size(),
        "serialization: sink recorded exactly the voiced schedule");
  std::printf("--- begin canonical schedule ---\n%s--- end canonical "
              "schedule ---\n",
              bytes_a.c_str());
}

}  // namespace

int main() {
  stable_event_to_cue_mapping();
  unknown_events_are_silent();
  deterministic_ordering();
  bus_mute_and_volume();
  voice_cap_steals_oldest();
  byte_identical_serialization_across_runs();
  if (failures != 0) {
    std::printf("%d check(s) FAILED\n", failures);
    return 1;
  }
  std::printf("all audio mixer checks passed\n");
  return 0;
}
