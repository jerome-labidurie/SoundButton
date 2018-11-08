// this example will play a random track from all on the sd 
//
// it expects the sd card to contain some mp3 files

#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

// implement a notification class,
// its member methods will get called 
//
class Mp3Notify
{
public:
  static void OnError(uint16_t errorCode)
  {
    // see DfMp3_Error for code meaning
    Serial.println();
    Serial.print("Com Error ");
    Serial.println(errorCode);
  }
  static void OnPlayFinished(uint16_t track)
  {
    Serial.print("Play finished for #");
    Serial.println(track);  
  }
  static void OnCardOnline(uint16_t code)
  {
    Serial.println("Card online ");
  }
  static void OnCardInserted(uint16_t code)
  {
    Serial.println("Card inserted ");
  }
  static void OnCardRemoved(uint16_t code)
  {
    Serial.println("Card removed ");
  }
};

// Some arduino boards only have one hardware serial port, so a software serial port is needed instead.
// comment out the above definition and uncomment these lines
SoftwareSerial secondarySerial(10, 11); // RX, TX
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(secondarySerial);

uint16_t count = 0; ///< number of mp3 tracks
uint16_t play = 0; ///< track to play
unsigned long lastRead = 0; ///< last button read time
unsigned long curRead = 0; ///< currentr button read time

/** number of buttons, must be equal or less than nb of tracks
 *  plug them to pins 2..NB_BTN+1
 */
#define NB_BTN 2

void setup() 
{
  
  Serial.begin(115200);

  Serial.println("initializing...");

  for (uint8_t i = 2; i <= NB_BTN+1; i++) {
    pinMode(i, INPUT_PULLUP);
  }
 
  mp3.begin();
  mp3.reset(); 
  
  // show some properties and set the volume
  uint16_t volume = mp3.getVolume();
  Serial.print("volume ");
  Serial.println(volume);
  mp3.setVolume(16);
  
  count = mp3.getTotalTrackCount();
  Serial.print("files ");
  Serial.println(count);

  uint16_t mode = mp3.getPlaybackMode();
  Serial.print("playback mode ");
  Serial.println(mode);
  
  Serial.println("starting...");

//  mp3.playRandomTrackFromAll(); // random of all folders on sd
}

void waitMilliseconds(uint16_t msWait)
{
  uint32_t start = millis();
  
  while ((millis() - start) < msWait)
  {
    // calling mp3.loop() periodically allows for notifications 
    // to be handled without interrupts
    mp3.loop(); 
    delay(1);
  }
}

void loop() 
{
  curRead = millis();
  if (curRead - lastRead > 200) {
    for (uint8_t i = 2; i<=NB_BTN+1; i++) {
      if (digitalRead(i) == LOW) {
        play = i-1;
        break;
      }
    }
    lastRead = curRead;
  }
  if (play > 0) {
    mp3.playGlobalTrack (play);
    play = 0;
  }
  waitMilliseconds(50);
  //Serial.print(play);
  //Serial.println (current);
}

