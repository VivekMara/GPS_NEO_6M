#include <SoftwareSerial.h>

// using software serial since arduino uno has only one hardware uart
SoftwareSerial gpsSerial(4, 3);  // RX=4, TX=3
String data = "";
unsigned long last_print_time = 0; // track last valid print

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600); // Neo-6M default baud
}

void loop() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (c == '\n') {  // end of sentence
      if (data.startsWith("$GPGGA")) {
        unsigned long now = millis();

        // print only once per second
        if (now - last_print_time >= 1000) {
          parse_GPGA(data);
          last_print_time = now;
        }
      }
      data = ""; // reset string
    } else {
      data += c;
    }
  }

  delay(10);
}

// convert UTC to IST
String convert_to_IST(String utc) {
  if (utc.length() < 6) return "Invalid";

  int hh = utc.substring(0, 2).toInt();
  int mm = utc.substring(2, 4).toInt();
  int ss = utc.substring(4, 6).toInt();

  // add 5 hours 30 minutes
  hh += 5;
  mm += 30;

  // handle minute overflow
  if (mm >= 60) {
    mm -= 60;
    hh++;
  }

  // handle hour overflow
  if (hh >= 24) {
    hh -= 24; // wrap around next day
  }

  char buffer[9];
  sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
  return String(buffer);
}

void parse_GPGA(String sentence) {
  int start = 0;
  int end   = sentence.indexOf(',');

  int fieldIndex = 0;
  while (end != -1) {
    String sub_data = sentence.substring(start, end);

    if (fieldIndex == 1) Serial.println("Time: " + convert_to_IST(sub_data));
    if (fieldIndex == 2) Serial.println("Latitude: " + sub_data);
    if (fieldIndex == 3) Serial.println("N/S: " + sub_data);
    if (fieldIndex == 4) Serial.println("Longitude: " + sub_data);
    if (fieldIndex == 5) Serial.println("E/W: " + sub_data);

    start = end + 1;
    end = sentence.indexOf(',', start);
    fieldIndex++;
  }

  Serial.println("Full data: " + sentence);
  Serial.println("-------------------------");
}
