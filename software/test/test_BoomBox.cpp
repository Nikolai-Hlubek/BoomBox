#include <unity.h>
#include "BoomBox.h"

// Run native 
// unity cannot mock classes, so it is kind of useless
// Use google test for the mocking

class vs1053 {
    public:
        void stopTrack() {
            // do nothing
        }

        void playMP3(char* filepath) {
            // do nothing
        }
};


void setUp(void) {
    // set stuff up here
}


void tearDown(void) {
    // clean stuff up here
}


void test_play() {
    Playlist playlist {
        .pDirMusic = "/Music1", 
        .currentTrackNo = 1,
        .maxTrackNo = 3,
        .playbackState = PlaybackState::Stopped,
        .playbackMode = PlaybackMode::One,
    };

    vs1053 MP3player;

    play(playlist, MP3player);
    TEST_ASSERT_EQUAL(2, playlist.currentTrackNo);
}



int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_play);

    UNITY_END();
}
