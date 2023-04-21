#pragma once

#include <stdint.h>

class vs1053;

enum class PlaybackMode : uint8_t
{
    One = 1,
    All
};

enum class PlaybackState : uint8_t
{
    Stopped = 1,
    Playing,
    Paused
};

typedef struct Playlist {
  const char* pDirMusic;
  uint8_t currentTrackNo;
  uint8_t maxTrackNo;
  PlaybackState playbackState;
  PlaybackMode playbackMode;
} Playlist;

void play(Playlist& playlist, vs1053& MP3player);
void setup_BoomBox();
void loop_BoomBox();
