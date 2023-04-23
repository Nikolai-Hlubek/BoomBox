#include "BoomBox.h"

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <vs1053_SdFat.h>
#include <Bounce2.h>

#include <string.h>

#define BUTTON_PIN_1 17
#define BUTTON_PIN_2 18
#define BUTTON_PIN_3 19
#define BUTTON_PIN_4 14
#define BUTTON_PIN_5 15
#define BUTTON_PIN_6 16
#define BUTTON_DEBOUNCE_PERIOD_MS 20  // ms

static const uint16_t buttons[6] = {BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4, BUTTON_PIN_5, BUTTON_PIN_6};
static const uint16_t numberOfButtons = sizeof(buttons) / sizeof(buttons[0]);
static const uint16_t volumePresets[] = {50, 40, 30, 20, 10};
static const uint8_t volumePresetsLen = sizeof(volumePresets)/sizeof(volumePresets[0]);
static uint8_t volumePresetsIdx = 3;
static uint8_t isPaused = 0;


Playlist playlist1 {
  .pDirMusic = "/Music1", 
  .currentTrackNo = 1,
  .maxTrackNo = 1,
  .playbackState = PlaybackState::Stopped,
  .playbackMode = PlaybackMode::One,
};
Playlist playlist2 {
  .pDirMusic = "/Music2", 
  .currentTrackNo = 1,
  .maxTrackNo = 1,
  .playbackState = PlaybackState::Stopped,
  .playbackMode = PlaybackMode::One,
};
Playlist playlist3 {
  .pDirMusic = "/Music3", 
  .currentTrackNo = 1,
  .maxTrackNo = 1,
  .playbackState = PlaybackState::Stopped,
  .playbackMode = PlaybackMode::All,
};
Playlist playlist4 {
  .pDirMusic = "/Music4", 
  .currentTrackNo = 1,
  .maxTrackNo = 1,
  .playbackState = PlaybackState::Stopped,
  .playbackMode = PlaybackMode::All,
};


SdFat sd;
File sdDir;  // open folder
File sdFile;  // open file
vs1053 MP3player;

Bounce bPause  = Bounce();
Bounce bVolume  = Bounce();
Bounce bMusic1  = Bounce();
Bounce bMusic2  = Bounce();
Bounce bMusic3  = Bounce();
Bounce bMusic4  = Bounce();


uint8_t countFilesInDirectory(const char* const pDirMusic) {
  uint8_t count = 0;
  sdDir.open(pDirMusic);
  while (true) {
    File entry = sdDir.openNextFile();
    if (!entry) {  // no more files
      break;
    }

    if (!entry.isDirectory()) {
      count++;
    }
    entry.close();
  }
  sdDir.close();
  return count;
}


PlaybackMode readPlaybackMode(const char* const line) {
  if (strncmp(line, "one", 3) == 0) {
    return PlaybackMode::One;
  } else if (strncmp(line, "One", 3) == 0) {
    return PlaybackMode::One;
  } else if (strncmp(line, "ONE", 3) == 0) {
    return PlaybackMode::One;
  } else if (strncmp(line, "all", 3) == 0) {
    return PlaybackMode::All;
  } else if (strncmp(line, "All", 3) == 0) {
    return PlaybackMode::All;
  } else if (strncmp(line, "ALL", 3) == 0) {
    return PlaybackMode::All;
  }
  return PlaybackMode::One;
}

void setPlaybackMode() {
  char line[14];
  int n;

  // open test file
  SdFile file("/playbackMode.txt", O_READ);
  if (!file.isOpen()) {
    Serial.println(F("Opening playbackMode file failed."));
    return;
  }
 
  // read lines from the file
  while ((n = file.fgets(line, sizeof(line))) > 0) {
    if (strncmp(line, "Music1", 6) == 0) {
      playlist1.playbackMode = readPlaybackMode(line+7);
//      Serial.println(static_cast<uint8_t>(playlist1.playbackMode));
    } else if (strncmp(line, "Music2", 6) == 0) {
      playlist2.playbackMode = readPlaybackMode(line+7);
//      Serial.println(static_cast<uint8_t>(playlist2.playbackMode));
    } else if (strncmp(line, "Music3", 6) == 0) {
      playlist3.playbackMode = readPlaybackMode(line+7);
//      Serial.println(static_cast<uint8_t>(playlist3.playbackMode));
    } else if (strncmp(line, "Music4", 6) == 0) {
      playlist4.playbackMode = readPlaybackMode(line+7);
//      Serial.println(static_cast<uint8_t>(playlist3.playbackMode));
    }
  }

  file.close();
}


void setup() {
  Serial.begin(9600);

  // use pullup of arduino to make hardware cheaper
  for (uint16_t i=0; i<numberOfButtons; ++i) {
      pinMode(buttons[i], INPUT_PULLUP);
  }

  bPause.attach(BUTTON_PIN_1);
  bPause.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bVolume.attach(BUTTON_PIN_3);
  bVolume.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic1.attach(BUTTON_PIN_4);
  bMusic1.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic2.attach(BUTTON_PIN_5);
  bMusic2.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic3.attach(BUTTON_PIN_6);
  bMusic3.interval(BUTTON_DEBOUNCE_PERIOD_MS);  
  bMusic4.attach(BUTTON_PIN_2);
  bMusic4.interval(BUTTON_DEBOUNCE_PERIOD_MS);

  if (!sd.begin(9, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
    Serial.println(F("SD Init failed."));
  }
  if (!sd.chdir("/")) {
    sd.errorHalt(F("sd.chdir"));
    Serial.println(F("SD directory change failed."));
  }

  uint8_t  status = 0;
  status = MP3player.begin();  // Loads patch if present
  if (status != 0) {
    Serial.println(F("MP3 player init failed with code:"));
    Serial.println(status);
  }
  // Hoehen: erlaubte Werte: -8 bis 7
  MP3player.setTrebleAmplitude(4);
  // Baesse: erlaubte Werte 0 bis 15
  MP3player.setBassAmplitude(7);
  // State of the player
  if (MP3player.getState() == 1) {
    Serial.println(F("MP3 player started successfully."));
  }

  int16_t vol = volumePresets[volumePresetsIdx];
  MP3player.setVolume(vol,vol);  // smaller numbers are louder

  playlist1.maxTrackNo = countFilesInDirectory(playlist1.pDirMusic);
  playlist2.maxTrackNo = countFilesInDirectory(playlist2.pDirMusic);
  playlist3.maxTrackNo = countFilesInDirectory(playlist3.pDirMusic);
  playlist4.maxTrackNo = countFilesInDirectory(playlist4.pDirMusic);
  
  setPlaybackMode();
}


void volume() {
  Serial.println(F("Change volume."));

  ++volumePresetsIdx;
  if (volumePresetsIdx >= volumePresetsLen) {
    volumePresetsIdx = 0;
  }

  int16_t vol = volumePresets[volumePresetsIdx];
  
  MP3player.setVolume(vol,vol);  // smaller numbers are louder
}

void pause() {
  MP3player.SendSingleMIDInote();  // play short "Beep"

  if (isPaused) {
    Serial.println(F("Resume current track."));
    MP3player.resumeMusic();
    isPaused = 0;
  } else {
    Serial.println(F("Pausing current track."));
    MP3player.pauseMusic();
    isPaused = 1;
  }
}

void stop() {
  playlist1.playbackState = PlaybackState::Stopped;
  playlist2.playbackState = PlaybackState::Stopped;
  playlist3.playbackState = PlaybackState::Stopped;
  playlist4.playbackState = PlaybackState::Stopped;

  if (MP3player.isPlaying()) {
    MP3player.stopTrack();
  }
}

void loop() {

  // -------------------------------------------------------------------------
  // input handling
  // -------------------------------------------------------------------------
  if (bVolume.update()) {
    if (bVolume.read() == LOW) {
      volume();
    }
  }

  if (bPause.update()) {
    if (bPause.read() == LOW) {
      pause();
    }
  }

  if (bMusic1.update()) {
    if (bMusic1.read() == LOW) {
      stop();
      play(playlist1, MP3player);
    }
  }

  if (bMusic2.update()) {
    if (bMusic2.read() == LOW) {
      stop();
      play(playlist2, MP3player);
    }
  }

  if (bMusic3.update()) {
    if (bMusic3.read() == LOW) {
      stop();
      play(playlist3, MP3player);
    }
  }

  if (bMusic4.update()) {
    if (bMusic4.read() == LOW) {
      stop();
      play(playlist4, MP3player);
    }
  }

  // -------------------------------------------------------------------------
  // if track is finished and playback mode is "all" then play next track
  // -------------------------------------------------------------------------
  if ((playlist1.playbackMode == PlaybackMode::All) && (playlist1.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist1, MP3player);  // play next song
    }
  }
  if ((playlist2.playbackMode == PlaybackMode::All) && (playlist2.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist2, MP3player);  // play next song
    }
  }
  if ((playlist3.playbackMode == PlaybackMode::All) && (playlist3.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist3, MP3player);  // play next song
    }
  }
  if ((playlist4.playbackMode == PlaybackMode::All) && (playlist4.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist4, MP3player);  // play next song
    }
  }

  // -------------------------------------------------------------------------
  // wait so we don't overload the CPU and bus
  // -------------------------------------------------------------------------
  delay(100);  // delay in ms
}
