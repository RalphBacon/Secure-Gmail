/*
 * This demo sketch will fail at the Gmail login unless your Google account has
 * set the following option:
 *
 *     Allow less secure apps: OFF
 *
 * which can be found in your Google account under Security. You will get an email
 * warning from Google that the universe will implode if you allow this.
 *
 * see https://www.base64encode.org/ for encoding / decoding
 */

#include <base64.h>
#include <ESP8266WiFi.h>
#include "Config.h"

/*
 * const char* _ssid = "Greypuss";
 * const char* _password = "mypassword";
 * const char* _GMailServer = "smtp.gmail.com";
 * const char* _mailUser = "mygmailaddress@gmail.com";
 * const char* _mailPassword = "my Google password";
*/

const char* ssid = _ssid;
const char* fingerprint = "289509731da223e5218031c38108dc5d014e829b"; // For smtp.gmail.com

WiFiClientSecure client;

// Forward declarations of functions (only required in Eclipse IDE)
byte response();
byte sendEmail();

void setup() {
  Serial.begin(74880);
  delay(10);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(_ssid, _password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("My IP address: ");
  Serial.println(WiFi.localIP());

  delay(1000);
  sendEmail();
}

void loop() {
//Nothing to do here. Never send email in a loop. You will get blacklisted.
}

// Function send a secure email via Gmail
byte sendEmail()
{
  //client.setFingerprint(fingerprint); // not available in axTLS::WiFiClientSecure 4.2.2
  // port 465=SSL 567=TLS; 587 not available with library 4.2.2
  // this all needs Google security downgrading:
  // https://myaccount.google.com/lesssecureapps?utm_source=google-account&utm_medium=web
  
 /*
 * Gmail exposes port 465 for SMTP over SSL and port 587 for SMTP with STARTTLS.
 * The difference between these two is that SMTP over SSL first establishes a secure 
 * SSL/TLS connection and conducts SMTP over that connection, and SMTP with STARTTLS 
 * starts with unencrypted SMTP and then switches to SSL/TLS. 
 * See https://stackoverflow.com/questions/17281669/using-smtp-gmail-and-starttls
 */  
  Serial.println("Attempting to connect to GMAIL server");
  if (client.connect(_GMailServer, 465) == 1) {
    Serial.println(F("Connected"));
  } else {
    Serial.print(F("Connection failed:"));
    return 0;
  }
  if (!response())
    return 0;

  Serial.println(F("Sending Extended Hello"));
  client.println("EHLO gmail.com");
  if (!response())
    return 0;

  // We're not using port 567 in this demo
  //Serial.println(F("STARTTLS"));
  //if (!response())
  //  return 0;
  //Serial.println(F("Sending EHLO"));
  //client.println("EHLO gmail.com");
  //if (!response())
  //  return 0;
  
  Serial.println(F("Sending auth login"));
  client.println("auth login");
  if (!response())
    return 0;

  Serial.println(F("Sending User"));
  // Change to your base64, ASCII encoded user
  client.println(base64::encode(_mailUser));
  if (!response())
    return 0;

  Serial.println(F("Sending Password"));
  // change to your base64, ASCII encoded password
  client.println(base64::encode(_mailPassword));
  if (!response())
    return 0;

  Serial.println(F("Sending From"));
  // your email address (sender) - MUST include angle brackets
  client.println(F("MAIL FROM: <arduinitepower@gmail.com>"));
  if (!response())
    return 0;

  // change to recipient address - MUST include angle brackets
  Serial.println(F("Sending To"));
  client.println(F("RCPT To: <ralph@gmail.com>"));
  // Repeat above line for EACH recipient
  if (!response())
    return 0;

  Serial.println(F("Sending DATA"));
  client.println(F("DATA"));
  if (!response())
    return 0;

  Serial.println(F("Sending email"));
  // recipient address (include option display name if you want)
  client.println(F("To: Home Alone Group<totally@made.up>"));

  // change to your address
  client.println(F("From: HomeAlone@gmail.com"));
  client.println(F("Subject: Your Arduino\r\n"));
  client.println(F("This email was sent securely via an encrypted mail link.\n"));
  client.println(F("In the last hour there was: 8 activities detected. Please check all is well."));
  client.println(F("This email will NOT be repeated for this hour.\n"));
  client.println(F("This email was sent from an unmonitored email account - please do not reply."));
  client.println(F("Love and kisses from Dougle and Benny. They wrote this sketch."));

  // IMPORTANT you must send a complete line containing just a "." to end the conversation
  // So the PREVIOUS line to this one must be a prinln not just a print
  client.println(F("."));
  if (!response())
    return 0;

  Serial.println(F("Sending QUIT"));
  client.println(F("QUIT"));
  if (!response())
    return 0;

  client.stop();
  Serial.println(F("Disconnected"));
  return 1;
}

// Check response from SMTP server
byte response()
{
  // Wait for a response for up to X seconds
  int loopCount = 0;
  while (!client.available()) {
    delay(1);
    loopCount++;
    // if nothing received for 10 seconds, timeout
    if (loopCount > 10000) {
      client.stop();
      Serial.println(F("\r\nTimeout"));
      return 0;
    }
  }

  // Take a snapshot of the response code
  byte respCode = client.peek();
  while (client.available())
  {
    Serial.write(client.read());
  }

  if (respCode >= '4')
  {
    Serial.print("Failed in eRcv with response: ");
    Serial.print(respCode);
    return 0;
  }
  return 1;
}
