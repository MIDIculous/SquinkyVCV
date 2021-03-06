#pragma once

#include "GateTrigger.h"
#include "MidiTrack.h"
#include "MidiVoice.h"
#include "MidiVoiceAssigner.h"

#include <memory>

class IMidiPlayerHost4;
class MidiSong4;
class MidiTrack;

// #define _MLOG

/**
 * This awful hack is so that both the real plugin and
 * the unit tests can pass this "Output" struct around
 */
#ifdef __PLUGIN
namespace rack {
namespace engine {
struct Input;
struct Param;
}
}  // namespace rack
#else
#include "TestComposite.h"
#endif

/**
 * input port usage
 * 
 * 0 = gate: goto next section
 * 1 = gate: goto prev section
 * 2 = cv: set section number
 * 3 = transpose
 * 4 =play clip 2x/3x/4x... faster (CV)
 */

class MidiTrackPlayer {
public:
#ifdef __PLUGIN
    using Param = rack::engine::Param;
    using Input = rack::engine::Input;
#else
    using Input = ::Input;
    using Param = ::Param;
#endif

    MidiTrackPlayer(std::shared_ptr<IMidiPlayerHost4> host, int trackIndex, std::shared_ptr<MidiSong4> song);
    void setSong(std::shared_ptr<MidiSong4> newSong, int trackIndex);
    void resetAllVoices(bool clearGates);

    /**
     * play the next event, if possible.
     * return true if event played.
     */
    bool playOnce(double metricTime, float quantizeInterval);

    void reset(bool resetSectionIndex);
    void setNumVoices(int numVoices);
    void setSampleCountForRetrigger(int);
    void updateSampleCount(int numElapsed);
    std::shared_ptr<MidiSong4> getSong();

    /**
     * For all these API, the section numbers are 1..4
     * for "next section" that totally makes sense, as 0 means "no request".
     * for getSection() I don't know what it's that way...
     */
    int getSection() const;
    void setNextSection(int section);

    /**
     * @param section is a new requested section (0, 1..4)
     * @returns valid section request (0,1..4) 
     *      If section exists, will return section
     *      otherwise will search forward for one to play.
     *      Will return 0 if there are no playable sections.
     */
    int validateSectionRequest(int section) const;
    int getNextSection() const;
    void setRunningStatus(bool running) {
        isPlaying = running;
    }

    void setPorts(Input* cvInput, Param* triggerImmediate) {
        input = cvInput;
        immediateParam = triggerImmediate;
    }

    /**
     * Returns the count in counting up the repeats.
     */
    int getCurrentRepetition();

private:
    std::shared_ptr<MidiSong4> song;
    std::shared_ptr<MidiTrack> curTrack;  // need something like array for song4??
    const int trackIndex = 0;


    /**
     * Variables around voice state
     */
    int numVoices = 1;  // up to 16
    static const int maxVoices = 16;
    MidiVoice voices[maxVoices];
    MidiVoiceAssigner voiceAssigner;

    /**
     * VCV Input port for the CV input for track
     */
    Input* input = nullptr;
    
    /**
     * Schmidt triggers for various CV input channels
     */
    GateTrigger nextSectionTrigger;
    GateTrigger prevSectionTrigger;

    Param* immediateParam = nullptr;        // not imp yet

    /************************************************************************************
     * variables for playing a track
     * 
     * Q: In general, when/how should we init the playback vars? Esp curEvent
     * and curSectionIndex?
     * ATM it's a little schizophrenic. we mostly change them when we finish playing
     * a section. But we also init from from reset and setSong().
     * 
     * Maybe we need a flag to tell us when to do setup? we sort of use reset() for that now,
     * but it's not a flag. We could do something like
     * bool needsReset (or even put it in event Q!!!), as well as
     * bool needsPlaySetup
     *      if true, then play calls will check it first. If set, clear and do one-time setup
     *      set it from reset.
     * 
     * Do we want to get away from requiring a reset call to set up playback? Could 
     * we use isPlaying instead? Sure, we can do that.
     * But - is there any difference between a reset and a change in playing status? I guess it's
     * fine to have two API's (setRunningStatus and reset()) to set it, but it's only one thing,
     * and it should probably live.... in the event Q.
     * 
     * can we use event Q for all requests to go to different sections, even the normal playback transitions?
     * I think so, if we are careful to set the requests before we process them. Oh, but when we are playing back we 
     * can easily blow through multiple events, so the section changes need to be more lock-step.
     * Now I'm thinking we can't we need functions to advance it. But they should be very clearly playback functions.
     * 
     * In the future, we may not want reset to change sections. but "hard reset" should.
     * 
     * How many resets are there:
     *      setup to play from curent location (edit conflict reset)
     *      setup to play from the very start (hard reset / reset CV)
     *      setup to lay from very start (first playback after new song)???
     * 
     * If we never hard reset (from cv) when will we ever setup to play?
     * 
     * 
     * Plan:
     *  rename findFirstTrackSection to something like setupTrackSectionForPlayback.
     *  move reset requests into the queue
     */

    /**
     * abs metric time of start of current section's current loop.
     * Do we still uses this? we've changed how looping works..
     */
    double currentLoopIterationStart = 0;

    /**
     * event iterator that playback uses. Advances each
     * time an event is played from the track.
     * We also set it on set song, but maybe that should be queued also?
     */
    MidiTrack::const_iterator curEvent;

    /**
     * cur section index is 0..3, and is the direct index into the
     * song4 sections array.
     * This variable should not be directly manipulated by UI.
     * It is typically set by playback code when a measure changes.
     * It is also set when we set the song, etc... but that's probably a mistake. We should probably 
     * only queue a change when we set song.
     */
    int curSectionIndex = 0;

    /**
     * This counter counts down. when if gets to zero
     * the section is done.
     */
    int sectionLoopCounter = 1;
    int totalRepeatCount = 1;  // what repeat was set to at the start of the section

    /**
     * Sometimes we need to know if we are playing.
     * This started as an experiment - is it still used?
     */
    bool isPlaying = false;    

    bool pollForNoteOff(double metricTime);
    void findFirstTrackSection();

    /**
     * will set curSectionIndex, and sectionLoopCounter
     * to play the next valid section after curSectionIndex
     */
    void setupToPlayNextSection();

    /**
     * As above, will set CurSelectionIndex, and sectionLoopCounter.
     * @param section is the section + 1 we wish to go to.
     */
    void setupToPlayDifferentSection(int section);
    void setupToPlayCommon();
    void onEndOfTrack();
    void pollForCVChange();

    /**
     * This is not an event queue at all.
     * It's a collection of flags and vaues that are queued up.
     */
    class EventQ {
    public:
        /**
         * next section index is different. it is 1..4, where 
         * 0 means "no request". APIs to get and set this
         * use the save 1..4 offset index.
         */
        int nextSectionIndex = 0;

        /**
         * If false, we wait for next loop iteration to apply queue.
         * If true, we do "immediately";
         */
        bool eventsHappenImmediately = false;
    };

    EventQ eventQ;
    GateTrigger cv0Trigger;
    GateTrigger cv1Trigger;
};
