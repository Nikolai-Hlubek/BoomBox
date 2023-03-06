//Add the SdFat Libraries
//#include <SdFat.h>
//#include <SdFatUtil.h>
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
#define BUTTON_DEBOUNCE_PERIOD 20 // ms

static const uint16_t buttons[6] = {BUTTON_PIN_1, BUTTON_PIN_2, BUTTON_PIN_3, BUTTON_PIN_4, BUTTON_PIN_5, BUTTON_PIN_6};
static const uint16_t numberOfButtons = sizeof(buttons) / sizeof(buttons[0]);

SdFat sd;
//SdFile file;
vs1053 MP3player;

Bounce bNext  = Bounce();
Bounce bStop  = Bounce();
Bounce bPlay  = Bounce();

// Variable für das Lesen des Verzeichnisses
File sdDir;
File sdFile;

char rootDir[2] = "/";


void printSDRootContent() {
  char filename[50];
  sdDir.open(rootDir);
  uint16_t count = 1;

  while (sdFile.openNext(&sdDir, O_READ)) {
    sdFile.getName(filename, sizeof(filename));
    if ( isFnMusic(filename) ) {
     
      char result[100];
      snprintf(result, sizeof(result), "%s%s", rootDir, filename);
      Serial.println(result);
//      MP3player.playMP3(&result[0]);
    }
    sdFile.close();
  }
  sdDir.close();
}

void setup() {
  Serial.begin(9600);

  for (int i=0; i<numberOfButtons; i++) {
      pinMode(buttons[i], INPUT_PULLUP);
  }

  bNext.attach(BUTTON_PIN_1);
  bNext.interval(BUTTON_DEBOUNCE_PERIOD);
  bPause.attach(BUTTON_PIN_2);
  bPause.interval(BUTTON_DEBOUNCE_PERIOD);
  bPlay.attach(BUTTON_PIN_3);
  bPlay.interval(BUTTON_DEBOUNCE_PERIOD);

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
    Serial.println("Player erfolgreich gestartet");
  }

  MP3player.setVolume(1,1);

  printSDRootContent();
  sdDir.open(rootDir);
}


void loop() {
  uint8_t buttonStates[numberOfButtons] = {2};
  for (int i=0; i<numberOfButtons; i++) {
    buttonStates[i] = digitalRead(buttons[i]);
  }

  sdDir.open(rootDir);

  if (bNext.update()) {
    if (bNext.read() == LOW) {

      Serial.println("Playing next track.");
      char filename[50];
      sdFile.openNext(&sdDir, O_READ);
      sdFile.getName(filename, sizeof(filename));
      Serial.println(filename);
      if ( isFnMusic(filename) ) {
        MP3player.playMP3(filename);
              snprintf(result, sizeof(result), "%s%s", dirname, filename);
      Serial.println(result);
//      MP3player.playMP3(&result[0]);
      }
    sdFile.close();      
    }
  }

  if (bPause.update()) {
    if (bPause.read() == LOW) {
      MP3player.SendSingleMIDInote();  // play short "Beep"

      Serial.println("Pausing current track.");
      if MP3player.isPlaying() {
        MP3player.pauseMusic();
      } else {
        MP3player.resumeMusic();
      }
    }
  }



/*
  for (int i=0; i<numberOfButtons; i++) {
    Serial.print(buttonStates[i]);
    Serial.print(" ");
  }
  Serial.println("foo");
*/
//  delay(100);
}
