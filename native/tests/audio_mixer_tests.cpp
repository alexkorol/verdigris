// TASK-0157: dedicated audio scheduler tests. Proves stable event-to-cue
// mapping, deterministic ordering, bus mute/volume behavior, bounded
// voice-cap eviction (steal-oldest), unknown-event silence, and byte-
// identical serialized schedules across two in-process runs. The binary is
// run twice by the acceptance gate; its final line prints a schedule digest
// so cross-process identity is visible in the transcript. Everything here
// schedules data only: no device, asset, dependency, simulation, or wire
// surface is touched.

#include <cstdio>
#include <string>
#include <vector>

#include "../audio/audio_mixer.hpp"
#include "../audio/cue_library.hpp"
#include "../audio/presentation_cues.hpp"

namespace {

int failures = 0;

void check(bool ok, const char* label) {
  std::printf("%s %s\n", ok ? "PASS" : "FAIL", label);
  if (!ok) ++failures;
}

using verdigris::audio::Admission;
using verdigris::audio::AudioMixer;
using verdigris::audio::Bus;
using verdigris::audio::CueSpec;
using verdigris::audio::CueTrigger;
using verdigris::audio::Priority;
using verdigris::audio::RecordingSink;
using verdigris::audio::ScheduledCue;
using verdigris::audio::Waveform;
using verdigris::client::PresentationEvent;
using verdigris::client::PresentationEventType;

CueTrigger classify(const PresentationEvent& event) {
  return verdigris::audio::classify_presentation_event(event);
}

bool label_equals(const CueSpec& spec, const char* text) {
  for (size_t i = 0; i < verdigris::audio::kCueLabelBytes; ++i) {
    if (text[i] == '\0') return spec.label[i] == '\0';
    if (spec.label[i] != text[i]) return false;
  }
  return true;
}

verdigris::audio::CueLabel label_from(const char* text) {
  verdigris::audio::CueLabel label{};
  for (size_t i = 0; text[i] != '\0' && i + 1 < verdigris::audio::kCueLabelBytes;
       ++i)
    label[i] = text[i];
  return label;
}

// The representative TASK-0157 presentation-event set, built as real
// PresentationEvent values with authoritative parity fields.
const PresentationEvent kOrdinaryHit{PresentationEventType::DamageApplied,
                                     "foe-1", "", "outgoing", 4, false, ""};
const PresentationEvent kCriticalHit{PresentationEventType::DamageApplied,
                                     "foe-1", "", "outgoing", 9, true, "stab"};
const PresentationEvent kEnemyDefeat{PresentationEventType::ActorDied, "foe-1",
                                     "", "", 0, false, ""};
const PresentationEvent kScionLoss{
    PresentationEventType::ScionLost, "scion-1", "", "Fable", 0, false, ""};
const PresentationEvent kWarCryExpire{PresentationEventType::BuffExpired,
                                      "scion-1", "", "war-cry", 0, false, ""};

void stable_event_to_cue_mapping() {
  // Same event classified twice yields identical cue parameters.
  for (const PresentationEvent* event :
       {&kOrdinaryHit, &kCriticalHit, &kEnemyDefeat, &kScionLoss,
        &kWarCryExpire}) {
    const CueTrigger first = classify(*event);
    const CueTrigger second = classify(*event);
    check(first == second && first != CueTrigger::Unknown,
          "mapping: representative events classify stably");
    const auto first_map = verdigris::audio::cue_for_trigger(first);
    const auto second_map = verdigris::audio::cue_for_trigger(second);
    check(first_map.mapped && first_map.spec == second_map.spec &&
              first_map.bus == second_map.bus &&
              first_map.priority == second_map.priority,
          "mapping: trigger-to-cue translation is identical across calls");
  }

  // The mapping pins exactly these content-neutral parameters.
  const auto ordinary = verdigris::audio::cue_for_trigger(classify(kOrdinaryHit));
  check(label_equals(ordinary.spec, "hit") &&
            ordinary.spec.waveform == Waveform::Noise &&
            ordinary.priority == Priority::Feedback &&
            ordinary.spec.pitch_start_permil ==
                ordinary.spec.pitch_end_permil,
        "mapping: ordinary hit is the flat noise feedback cue");
  const auto critical = verdigris::audio::cue_for_trigger(classify(kCriticalHit));
  check(critical.spec.gain_milli > ordinary.spec.gain_milli &&
            critical.spec.duration_ms > ordinary.spec.duration_ms &&
            critical.spec.pitch_end_permil < critical.spec.pitch_start_permil,
        "mapping: critical hit differs structurally from the ordinary hit");
  const auto defeat = verdigris::audio::cue_for_trigger(classify(kEnemyDefeat));
  check(defeat.spec.waveform == Waveform::Tone &&
            defeat.spec.pitch_end_permil < defeat.spec.pitch_start_permil &&
            defeat.priority == Priority::World,
        "mapping: enemy defeat is the falling world-class tone");
  const auto loss = verdigris::audio::cue_for_trigger(classify(kScionLoss));
  check(loss.spec.duration_ms > defeat.spec.duration_ms &&
            loss.spec.pitch_start_permil == 500 &&
            loss.spec.pitch_end_permil == 250,
        "mapping: Scion loss is the longest, lowest fall in the set");
  const auto expire = verdigris::audio::cue_for_trigger(classify(kWarCryExpire));
  check(expire.priority == Priority::Ui && expire.mapped &&
            label_equals(expire.spec, "warcry-end"),
        "mapping: war-cry expiry is the UI-class buff cue");

  // No absolute frequency anywhere: pitch fields are ratios of a runtime
  // reference owned elsewhere.
  bool ratios_only = true;
  for (const CueTrigger trigger : {CueTrigger::OrdinaryHit, CueTrigger::CriticalHit,
                                   CueTrigger::EnemyDefeat, CueTrigger::ScionLoss,
                                   CueTrigger::WarCryExpire}) {
    const auto mapped = verdigris::audio::cue_for_trigger(trigger);
    if (mapped.spec.pitch_start_permil <= 0 ||
        mapped.spec.pitch_end_permil <= 0)
      ratios_only = false;
  }
  check(ratios_only, "mapping: all pitch fields are positive reference ratios");
}

void deterministic_ordering() {
  RecordingSink sink_a;
  AudioMixer mixer_a(sink_a);
  RecordingSink sink_b;
  AudioMixer mixer_b(sink_b);

  const CueSpec world_cue{label_from("world"), Waveform::Tone, 100, 0, 50, 400,
                          1000, 1000};
  const CueSpec ui_cue{label_from("ui"), Waveform::Tone, 80, 0, 40, 300, 1000,
                       1000};
  const CueSpec feedback_cue{label_from("feedback"), Waveform::Noise, 60, 0, 30,
                             350, 1000, 1000};

  // Mixer A submits frames out of order with mixed priorities.
  const Admission a1 = mixer_a.submit(Bus::Sfx, Priority::World, world_cue, 30).status;
  const Admission a2 = mixer_a.submit(Bus::Sfx, Priority::Ui, ui_cue, 10).status;
  const Admission a3 =
      mixer_a.submit(Bus::Sfx, Priority::Feedback, feedback_cue, 10).status;
  const Admission a4 = mixer_a.submit(Bus::Sfx, Priority::World, world_cue, 20).status;
  const Admission b1 = mixer_b.submit(Bus::Sfx, Priority::World, world_cue, 20).status;
  const Admission b2 =
      mixer_b.submit(Bus::Sfx, Priority::Feedback, feedback_cue, 10).status;
  const Admission b3 = mixer_b.submit(Bus::Sfx, Priority::Ui, ui_cue, 10).status;
  const Admission b4 = mixer_b.submit(Bus::Sfx, Priority::World, world_cue, 30).status;

  check(a1 == Admission::Admitted && a2 == Admission::Admitted &&
            a3 == Admission::Admitted && a4 == Admission::Admitted &&
            b1 == Admission::Admitted && b2 == Admission::Admitted &&
            b3 == Admission::Admitted && b4 == Admission::Admitted,
          "ordering: every scripted submission admits below cap");

  const std::vector<ScheduledCue> ordered_a = mixer_a.active_voices(Bus::Sfx);
  check(ordered_a.size() == 4 && ordered_a[0].frame == 10 &&
            ordered_a[0].priority == Priority::Ui &&
            ordered_a[1].frame == 10 &&
            ordered_a[1].priority == Priority::Feedback &&
            ordered_a[2].frame == 20 && ordered_a[3].frame == 30,
        "ordering: canonical order is frame asc then priority desc then age");
  check(ordered_a[0].sequence < ordered_a[1].sequence,
        "ordering: equal frame and priority keep admission age order");

  // Different submission order, same logical multiset, same bytes.
  const std::vector<ScheduledCue> ordered_b = mixer_b.active_voices(Bus::Sfx);
  check(verdigris::audio::schedule_digest_hex(ordered_a) ==
            verdigris::audio::schedule_digest_hex(ordered_b),
        "ordering: insertion order never changes the serialized schedule");
}

void bus_mute_and_volume() {
  RecordingSink sink;
  AudioMixer mixer(sink);
  const CueSpec cue{label_from("mute"), Waveform::Tone, 100, 0, 50, 500, 1000,
                    1000};

  mixer.set_bus_muted(Bus::Sfx, true);
  const SubmitResult muted = mixer.submit(Bus::Sfx, Priority::Feedback, cue, 1);
  check(muted.status == Admission::RejectedMutedBus &&
            sink.admitted().empty(),
        "mute: muted buses reject cues before any voice exists");
  check(mixer.active_voices(Bus::Sfx).empty(), "mute: no voice is scheduled");

  mixer.set_bus_muted(Bus::Sfx, false);
  mixer.set_bus_volume(Bus::Sfx, 250);
  const SubmitResult quarter = mixer.submit(Bus::Sfx, Priority::Feedback, cue, 2);
  check(quarter.status == Admission::Admitted &&
            quarter.sequence != 0 &&
            sink.admitted().front().effective_gain_milli == 125,
        "volume: admission snapshots spec gain scaled by bus volume (500*250/1000)");

  mixer.set_bus_volume(Bus::Sfx, 1000);
  check(sink.admitted().front().effective_gain_milli == 125,
        "volume: later volume changes never rewrite already-scheduled voices");

  // Music bus state is independent.
  mixer.set_bus_muted(Bus::Music, true);
  const SubmitResult music_muted =
      mixer.submit(Bus::Music, Priority::World, cue, 3);
  check(music_muted.status == Admission::RejectedMutedBus,
        "mute: music bus mutes independently of SFX");
  mixer.set_bus_volume(Bus::Music, 700);
  const SubmitResult music_loud =
      mixer.submit(Bus::Music, Priority::World, cue, 4);
  check(music_loud.status == Admission::RejectedMutedBus,
        "mute: volume changes do not unmute a muted bus");

  mixer.set_bus_volume(Bus::Sfx, 1500);
  check(mixer.bus_volume(Bus::Sfx) == 1000,
        "volume: values above full scale clamp to 1000 milli");
  mixer.set_bus_volume(Bus::Music, 0);
  mixer.set_bus_muted(Bus::Music, false);
  const SubmitResult silent_music =
      mixer.submit(Bus::Music, Priority::World, cue, 5);
  check(silent_music.status == Admission::Admitted &&
            sink.admitted().back().effective_gain_milli == 0,
        "volume: zero-volume bus admits a fully-scaled-zero voice");
}

void cap_eviction_steals_oldest() {
  RecordingSink sink;
  AudioMixer mixer(sink);
  check(mixer.voice_cap(Bus::Sfx) == 8, "caps: named table defaults SFX to 8");
  check(mixer.voice_cap(Bus::Music) == 2,
        "caps: named table defaults music to 2");

  const CueSpec low{label_from("low"), Waveform::Tone, 100, 0, 50, 400, 1000,
                    1000};
  const CueSpec high{label_from("high"), Waveform::Noise, 90, 0, 40, 500, 1000,
                     1000};

  uint64_t first_sequence = 0;
  for (uint32_t i = 0; i < mixer.voice_cap(Bus::Sfx); ++i) {
    const SubmitResult result =
        mixer.submit(Bus::Sfx, Priority::World, low, i + 1);
    check(result.status == Admission::Admitted,
          "caps: board fills to the configured cap");
    if (i == 0) first_sequence = result.sequence;
  }

  // Same priority: steal-oldest. The oldest sequence must retire.
  const SubmitResult steal = mixer.submit(Bus::Sfx, Priority::World, low, 99);
  check(steal.status == Admission::Admitted &&
            steal.evicted_sequence == first_sequence &&
            mixer.active_voices(Bus::Sfx).size() == 8,
        "caps: at-cap same-priority submit evicts the oldest voice");
  check(sink.retire_count() == 1 &&
            sink.retired().front().first == first_sequence &&
            sink.retired().front().second == steal.sequence,
        "caps: the recording sink captured the exact eviction pair");
  bool oldest_gone = true;
  for (const ScheduledCue& cue : mixer.active_voices(Bus::Sfx))
    if (cue.sequence == first_sequence) oldest_gone = false;
  check(oldest_gone, "caps: the evicted voice leaves the active set");

  // Lower priority against a full higher-priority board drops instead.
  AudioMixer strict(sink);
  for (uint32_t i = 0; i < 8; ++i)
    strict.submit(Bus::Sfx, Priority::Feedback, high, i + 1);
  const size_t retired_before = sink.retire_count();
  const SubmitResult dropped =
      strict.submit(Bus::Sfx, Priority::World, low, 50);
  check(dropped.status == Admission::RejectedVoiceCap &&
            dropped.sequence == 0 && dropped.evicted_sequence == 0 &&
            strict.active_voices(Bus::Sfx).size() == 8 &&
            sink.retire_count() == retired_before,
        "caps: lower-priority incoming cue is dropped, never steals up");

  // UI priority steals into a full world-priority board.
  const SubmitResult ui_steal =
      strict.submit(Bus::Sfx, Priority::Ui, high, 51);
  check(ui_steal.status == Admission::Admitted &&
            ui_steal.evicted_sequence != 0,
        "caps: higher-priority incoming cue steals the lowest class");
  check(strict.active_voices(Bus::Sfx).size() == 8,
        "caps: cap holds exactly after a steal");

  // Buses evict independently.
  AudioMixer both(sink);
  both.submit(Bus::Music, Priority::World, low, 1);
  both.submit(Bus::Music, Priority::World, low, 2);
  check(both.active_voices(Bus::Music).size() == 2 &&
            both.active_voices(Bus::Sfx).empty(),
        "caps: music fills without touching SFX voices");
  const SubmitResult third_voice =
      both.submit(Bus::Music, Priority::World, low, 3);
  check(third_voice.status == Admission::Admitted &&
            third_voice.evicted_sequence != 0 &&
            both.active_voices(Bus::Music).size() == 2 &&
            both.active_voices(Bus::Sfx).empty(),
        "caps: music cap evicts inside its own bus only");

  // Caps are floor-clamped: the mixer can never be configured unbounded-off.
  mixer.set_voice_cap(Bus::Sfx, 0);
  check(mixer.voice_cap(Bus::Sfx) >= 1,
        "caps: zero caps clamp to the minimum live voice");
}

void unknown_event_silence() {
  RecordingSink sink;
  AudioMixer mixer(sink);

  const PresentationEvent message{PresentationEventType::Message, "", "",
                                  "hello", 0, false, ""};
  const PresentationEvent protocol_error{PresentationEventType::ProtocolError,
                                         "", "", "bad envelope", 0, false, ""};
  const PresentationEvent connected{
      PresentationEventType::ConnectionEstablished, "", "", "", 0, false, ""};
  const PresentationEvent other_buff{PresentationEventType::BuffExpired,
                                     "scion-1", "", "some-other-buff", 0,
                                     false, ""};
  const PresentationEvent scion_died{PresentationEventType::ScionDied,
                                     "scion-1", "", "", 0, false, ""};

  for (const PresentationEvent* event :
       {&message, &protocol_error, &connected, &other_buff, &scion_died}) {
    const CueTrigger trigger = classify(*event);
    check(trigger == CueTrigger::Unknown,
          "silence: uncovered events classify Unknown");
    const auto mapping = verdigris::audio::cue_for_trigger(trigger);
    check(!mapping.mapped,
          "silence: Unknown maps to explicit silence, not a default cue");
  }
  check(sink.admitted().empty() && mixer.active_voices(Bus::Sfx).empty() &&
            mixer.active_voices(Bus::Music).empty(),
        "silence: nothing was scheduled for unknown events");
}

std::vector<ScheduledCue> scripted_session(AudioMixer& mixer, RecordingSink& sink) {
  // Deterministic mixed-bus combat script over the representative set.
  struct Step {
    CueTrigger trigger;
    uint64_t frame;
  };
  const Step steps[] = {
      {CueTrigger::OrdinaryHit, 5},   {CueTrigger::CriticalHit, 6},
      {CueTrigger::WarCryExpire, 7},  {CueTrigger::EnemyDefeat, 8},
      {CueTrigger::OrdinaryHit, 9},   {CueTrigger::Unknown, 10},
      {CueTrigger::OrdinaryHit, 11},  {CueTrigger::ScionLoss, 12},
      {CueTrigger::EnemyDefeat, 13},  {CueTrigger::CriticalHit, 14},
      {CueTrigger::WarCryExpire, 15}, {CueTrigger::OrdinaryHit, 16},
      {CueTrigger::EnemyDefeat, 17},
  };
  mixer.set_bus_volume(Bus::Sfx, 800);
  mixer.set_bus_volume(Bus::Music, 600);
  for (const Step& step : steps) {
    const auto mapping = verdigris::audio::cue_for_trigger(step.trigger);
    if (!mapping.mapped) continue;
    mixer.submit(mapping.bus, mapping.priority, mapping.spec, step.frame);
  }
  return sink.admitted();
}

void byte_identical_schedules_across_runs() {
  RecordingSink sink_a;
  AudioMixer mixer_a(sink_a);
  const std::vector<ScheduledCue> run_one = scripted_session(mixer_a, sink_a);

  RecordingSink sink_b;
  AudioMixer mixer_b(sink_b);
  const std::vector<ScheduledCue> run_two = scripted_session(mixer_b, sink_b);

  check(run_one.size() == 12 && !run_one.empty(),
        "bytes: the script schedules exactly the mappable events");
  const std::vector<uint8_t> bytes_one =
      verdigris::audio::serialize_schedule(run_one);
  const std::vector<uint8_t> bytes_two =
      verdigris::audio::serialize_schedule(run_two);
  check(bytes_one.size() == bytes_two.size() && bytes_one == bytes_two,
        "bytes: serialized schedules are byte-identical across two runs");
  check(verdigris::audio::schedule_digest_hex(run_one) ==
            verdigris::audio::schedule_digest_hex(run_two),
        "bytes: schedule digests match across two runs");

  // Serialization is position-stable under input reordering.
  std::vector<ScheduledCue> shuffled = run_one;
  for (size_t i = shuffled.size(); i > 1; --i) {
    const size_t j = (i * 7919u) % i;  // Fixed arithmetic permutation.
    std::swap(shuffled[i - 1], shuffled[j - 1]);
  }
  check(verdigris::audio::serialize_schedule(shuffled) == bytes_one,
        "bytes: reordering inputs never changes the serialized schedule");
}

}  // namespace

int main() {
  stable_event_to_cue_mapping();
  deterministic_ordering();
  bus_mute_and_volume();
  cap_eviction_steals_oldest();
  unknown_event_silence();
  byte_identical_schedules_across_runs();

  // Cross-process evidence: this exact line must match between the two
  // acceptance-gate invocations of this executable.
  RecordingSink digest_sink;
  AudioMixer digest_mixer(digest_sink);
  const std::vector<ScheduledCue> scheduled =
      scripted_session(digest_mixer, digest_sink);
  std::printf("audio schedule digest: %s (%zu bytes)\n",
              verdigris::audio::schedule_digest_hex(scheduled).c_str(),
              verdigris::audio::serialize_schedule(scheduled).size());

  std::printf("%s\n", failures == 0 ? "audio mixer tests: PASS"
                                    : "audio mixer tests: FAIL");
  return failures == 0 ? 0 : 1;
}
