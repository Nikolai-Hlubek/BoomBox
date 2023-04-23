#include <unity.h>
#include "BoomBox.h"

#include <vs1053_SdFat.h>

// Stub mp3 player
void vs1053::stopTrack() {}
uint8_t vs1053::playMP3(char* filename, uint32_t timecode) {return 0;}
void delay(uint32_t ms) {}
int freeMemory() {return 0;}


Playlist playlist {
    .pDirMusic = "/Music1", 
    .currentTrackNo = 1,
    .maxTrackNo = 3,
    .playbackState = PlaybackState::Stopped,
    .playbackMode = PlaybackMode::One,
};

void setUp(void) {
    // set stuff up here
}


void tearDown(void) {
    // clean stuff up here
}

void test_next_track() {
    vs1053 MP3player;

    play(playlist, MP3player);
    TEST_ASSERT_EQUAL(playlist.currentTrackNo, 2);
    TEST_ASSERT_EQUAL(playlist.playbackState, PlaybackState::Playing);
}

void test_rollover_current_track_is_max_track() {
    playlist.currentTrackNo = 2;

    vs1053 MP3player;

    play(playlist, MP3player);
    play(playlist, MP3player);
    TEST_ASSERT_EQUAL(playlist.currentTrackNo, 1);
}

void test_rollover_stop_playback() {
    playlist.currentTrackNo = 3;
    playlist.playbackState = PlaybackState::Playing;

    vs1053 MP3player;

    play(playlist, MP3player);
    TEST_ASSERT_EQUAL(playlist.playbackState, PlaybackState::Stopped);
    TEST_ASSERT_EQUAL(playlist.currentTrackNo, 0);
}

int main() { // int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_next_track);
    RUN_TEST(test_rollover_current_track_is_max_track);
    UNITY_END();
}


