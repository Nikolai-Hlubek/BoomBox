#include <Arduino.h>

#include <string.h>

#include <SPI.h>
#include <SdFat.h>
#include <vs1053_SdFat.h>
#include <Bounce2.h>


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

SdFat sd;
File sdDir;  // open folder
File sdFile;  // open file
vs1053 MP3player;

Bounce bPause  = Bounce();
Bounce bStop  = Bounce();
Bounce bVolume  = Bounce();
Bounce bMusic1  = Bounce();
Bounce bMusic2  = Bounce();
Bounce bMusic3  = Bounce();

//
// Used in calculating free memory.
//
extern unsigned int __bss_end;
extern void *__brkval;

//
// Returns the current amount of free memory in bytes.
//
int freeMemory() {
	int free_memory;
	if ((int) __brkval) {
		return ((int) &free_memory) - ((int) __brkval);
  }
	return ((int) &free_memory) - ((int) &__bss_end);
}

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
  bStop.attach(BUTTON_PIN_2);
  bStop.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bVolume.attach(BUTTON_PIN_3);
  bVolume.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic1.attach(BUTTON_PIN_4);
  bMusic1.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic2.attach(BUTTON_PIN_5);
  bMusic2.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic3.attach(BUTTON_PIN_6);
  bMusic3.interval(BUTTON_DEBOUNCE_PERIOD_MS);  

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
  
  setPlaybackMode();
}


void play(Playlist& playlist) {
  char filepath[] = "/Music1/track001.mp3";

  // generate filename of file to play
  sprintf(filepath, "%s/track%03d.mp3", playlist.pDirMusic, playlist.currentTrackNo);

  Serial.println(filepath);
  ++playlist.currentTrackNo;
  if ((playlist.currentTrackNo) > 0 && (playlist.currentTrackNo > playlist.maxTrackNo)) {
    playlist.currentTrackNo = 1;
  }

  MP3player.stopTrack();
  delay(100);
  MP3player.playMP3(filepath);
  playlist.playbackState = PlaybackState::Playing;
  delay(100);

  Serial.println(F("Free RAM in bytes"));
  Serial.println(freeMemory());  // print how much RAM is available in bytes.
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

  if (bStop.update()) {
    if (bStop.read() == LOW) {
      MP3player.SendSingleMIDInote();  // play short "Beep"
      stop();
    }
  }

  if (bMusic1.update()) {
    if (bMusic1.read() == LOW) {
      stop();
      play(playlist1);
    }
  }

  if (bMusic2.update()) {
    if (bMusic2.read() == LOW) {
      stop();
      play(playlist2);
    }
  }

  if (bMusic3.update()) {
    if (bMusic3.read() == LOW) {
      stop();
      play(playlist3);
    }
  }

  // -------------------------------------------------------------------------
  // if track is finished and playback mode is "all" then play next track
  // -------------------------------------------------------------------------
  if ((playlist1.playbackMode == PlaybackMode::All) && (playlist1.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist1);  // play next song
    }
  }
  if ((playlist2.playbackMode == PlaybackMode::All) && (playlist2.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist2);  // play next song
    }
  }
  if ((playlist3.playbackMode == PlaybackMode::All) && (playlist3.playbackState == PlaybackState::Playing)) {
    if (!MP3player.isPlaying()) {  // track finished
      play(playlist3);  // play next song
    }
  }

  // -------------------------------------------------------------------------
  // wait so we don't overload the CPU and bus
  // -------------------------------------------------------------------------
  delay(100);  // delay in ms
}
