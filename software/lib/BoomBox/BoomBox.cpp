#include "BoomBox.h"

#include "freeMem.h"
#include <vs1053_SdFat.h>

#include "Arduino.h"


void play(Playlist& playlist, vs1053& MP3player) {
  char filepath[] = "/MusicX/track00Y.mp3";

  // generate filename of file to play
  sprintf(filepath, "%s/track%03d.mp3", playlist.pDirMusic, playlist.currentTrackNo);

  Serial.println(filepath);
  ++playlist.currentTrackNo;
  // Album finished?
  if ((playlist.currentTrackNo) > 0 && (playlist.currentTrackNo > playlist.maxTrackNo)) {
    playlist.currentTrackNo = 1;

    // Stop here if playlist finished and playback mode is all
    if (playlist.playbackMode == PlaybackMode::All) {
      playlist.playbackState = PlaybackState::Stopped;
      return;
    }
  }

  MP3player.stopTrack();
  delay(100);
  MP3player.playMP3(filepath);
  playlist.playbackState = PlaybackState::Playing;
  delay(100);

  Serial.println(F("Free RAM in bytes"));
  Serial.println(freeMemory());  // print how much RAM is available in bytes.
}


void setup_BoomBox() {

}


void loop_BoomBox() {

}