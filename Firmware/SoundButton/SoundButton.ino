/** SoundButton
 *  https://github.com/jerome-labidurie/SoundButton
 */
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>
#include <SerialCommands.h>

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
uint16_t play = 0; ///< track to play (bitmap)
unsigned long lastRead = 0; ///< last button read time
unsigned long curRead = 0; ///< currentr button read time

/** number of buttons, must be equal or less than nb of tracks
 *  plug them to pins 2..NB_BTN+1
 */
#define NB_BTN 3

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

//This is the default handler, and gets called when no other command matches. 
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
  sender->GetSerial()->println("+/- : volume");
  sender->GetSerial()->println("p N : play track N");
}

void cmd_vol(SerialCommands* sender, const char* cmd) {
  uint16_t volume = mp3.getVolume();
  if (*cmd == '+') {
    volume++;
  } else {
    volume--;
  }
  mp3.setVolume(volume);
  Serial.println(volume);
}

void cmd_play(SerialCommands* sender) {
  char* track = sender->Next();
  if (track == NULL) {
    Serial.println ("ERR: No track");
    return;
  }
  play = *track - '0';
}
SerialCommand cmd_vol_up("+", cmd_vol);
SerialCommand cmd_vol_dn("-", cmd_vol);
SerialCommand cmd_play_track("p", cmd_play);

void setup() 
{
  
  Serial.begin(115200);

  Serial.println("initializing...");

  for (uint8_t i = 2; i <= NB_BTN+1; i++) {
    pinMode(i, INPUT_PULLUP);
  }

  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_vol_up);
  serial_commands_.AddCommand(&cmd_vol_dn);
  serial_commands_.AddCommand(&cmd_play_track);
 
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
          play |= 1 << (i-2);
          lastRead = curRead;
      }
    }
  }
  if (play > 0) {
    mp3.playGlobalTrack (play);
    Serial.println(play);
    play = 0;
  }
  waitMilliseconds(40);
  serial_commands_.ReadSerial();
  
  //Serial.print(play);
  //Serial.println (current);
}

