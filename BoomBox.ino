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
#define BUTTON_DEBOUNCE_PERIOD_MS 20 // ms

static const uint16_t buttons[6] = {BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4, BUTTON_PIN_5, BUTTON_PIN_6};
static const uint16_t numberOfButtons = sizeof(buttons) / sizeof(buttons[0]);

static const char* pDirRoot = "/";
static const char* pDirMusic1 = "/Music1";
static const char* pDirMusic2 = "/Music2";
static const char* pDirMusic3 = "/Music3";


SdFat sd;
File sdDir;  // open folder
vs1053 MP3player;

Bounce bPlay  = Bounce();
Bounce bPause  = Bounce();
Bounce bStop  = Bounce();
Bounce bMusic1  = Bounce();
Bounce bMusic2  = Bounce();
Bounce bMusic3  = Bounce();

void printSDRootContent() {
  char filename[50];
  File sdFile;  // open file

  sdDir.open(pDirRoot);
  uint16_t count = 1;

  while (sdFile.openNext(&sdDir, O_READ)) {
    sdFile.getName(filename, sizeof(filename));
     
    char result[52];
    snprintf(result, sizeof(result), "%s%s", pDirRoot, filename);
    Serial.println(result);

    sdFile.close();
  }
  sdDir.close();
}

void setup() {
  Serial.begin(9600);

  // use pullup of arduino to make hardware cheaper
  for (int i=0; i<numberOfButtons; i++) {
      pinMode(buttons[i], INPUT_PULLUP);
  }

  bPlay.attach(BUTTON_PIN_1);
  bPlay.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bPause.attach(BUTTON_PIN_2);
  bPause.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bStop.attach(BUTTON_PIN_3);
  bStop.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic1.attach(BUTTON_PIN_4);
  bMusic1.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic2.attach(BUTTON_PIN_5);
  bMusic2.interval(BUTTON_DEBOUNCE_PERIOD_MS);
  bMusic3.attach(BUTTON_PIN_6);
  bMusic3.interval(BUTTON_DEBOUNCE_PERIOD_MS);  

  if (!sd.begin(9, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
    Serial.println("SD Init failed.");
  }
  if (!sd.chdir("/")) {
    sd.errorHalt("sd.chdir");
    Serial.println("SD directory change failed.");
  }

  uint8_t  status = 0;
  status = MP3player.begin();
  if (status != 0) {
    Serial.println("MP3 player init failed with code:");
    Serial.println(status);
  }
  // Höhen: erlaubte Werte: -8 bis 7
  MP3player.setTrebleAmplitude(4);
  // Bässe: erlaubte Werte 0 bis 15
  MP3player.setBassAmplitude(7);
  // State of the player
  if (MP3player.getState() == 1) {
    Serial.println("MP3 player started successfully.");
  }

  MP3player.setVolume(10,10);  // smaller numbers are louder

  printSDRootContent();
}

void next(const char* pDirMusic) {
  char filename[50] = "";
  char filepath[100] = "";
  File sdFile;  // open file

  bool filesRemaining = true;

  if (!sdDir.isOpen()) {
    Serial.println("opening");
    sdDir.open(pDirMusic);
  }
  filesRemaining = sdFile.openNext(&sdDir, O_READ);
  sdFile.getName(filename, sizeof(filename));
  Serial.println(filename);

  if (filesRemaining==false) {
    Serial.println("looping");
    sdDir.close();
    sdDir.open(pDirMusic);          
  }

  if ( isFnMusic(filename) ) {
    Serial.println("Starting playback.");
    const char* pSep = "/";
    strncat(filepath, pDirMusic, strlen(pDirMusic));  // Concatenate up to n characters from string2 to string1
    strncat(filepath, pSep, strlen(pSep));
    strncat(filepath, filename, strlen(filename));
    Serial.println("Starting playback of file:");
    Serial.println(filepath);
    MP3player.stopTrack();
    MP3player.playMP3(filepath);
  }
  sdFile.close();
}

void play() {
  Serial.println("Resume current track.");
  if (!MP3player.isPlaying()) {
    MP3player.resumeMusic();
  }
}

void pause() {
  MP3player.SendSingleMIDInote();  // play short "Beep"

  Serial.println("Pausing current track.");
  if (MP3player.isPlaying()) {
    MP3player.pauseMusic();
  } else {
    MP3player.resumeMusic();
  }
}

void stop() {
  MP3player.SendSingleMIDInote();  // play short "Beep"

  Serial.println("Stop current track.");
  if (MP3player.isPlaying()) {
    MP3player.stopTrack();
  }
}

void loop() {

  if (bPlay.update()) {
    if (bPlay.read() == LOW) {
      play();
    }
  }

  if (bPause.update()) {
    if (bPause.read() == LOW) {
      pause();
    }
  }

  if (bStop.update()) {
    if (bStop.read() == LOW) {
      stop();
    }
  }

  if (bMusic1.update()) {
    if (bMusic1.read() == LOW) {
      next(pDirMusic1);
    }
  }

  if (bMusic2.update()) {
    if (bMusic2.read() == LOW) {
      next(pDirMusic2);
    }
  }

  if (bMusic3.update()) {
    if (bMusic3.read() == LOW) {
      next(pDirMusic3);
    }
  }

//  delay(100);
}
