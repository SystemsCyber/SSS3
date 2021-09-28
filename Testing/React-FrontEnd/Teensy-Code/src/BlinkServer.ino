//#include <WiFi.h>
#include <SPI.h>
#include <Ethernet.h>
#include "aWOT.h"
#include "StaticFiles.h"
#include "SSS3_defines.h"
#include "SSS3_board_defs_rev_1.h"
#include "SSS3_functions.h"
#include <ArduinoJson.h>
#include <StreamUtils.h>
#define LED_BUILTIN 2

byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
EthernetServer server(80);

Application app;
DynamicJsonDocument doc(2048);
ReadLoggingStream loggingStream(Serial, Serial);

bool ledOn;
uint8_t buff[2048];
char buff_c[2048];
char *userFilter[3] = {"name", "password", NULL};
// aJsonObject* user = aJson.parse("{\"name\":\"John Smith\",\"password\":\"secret\"}");
void read_keySw(Request &req, Response &res)
{
  Serial.print("Got GET Request for LED returned: ");
  Serial.println(ignitionCtlState);
  res.print(ignitionCtlState);
}

void update_keySw(Request &req, Response &res)
{

  // JsonObject& config = jb.parseObject( &req);
  Serial.print("Got POST Request for LED: ");
  req.body(buff, sizeof(buff));
  if (!parse_response(buff))
  {
    res.print("Not a valid Json Format");
  }
  else
  {
    ledOn = doc["ledOn"];
    Serial.println(ledOn);
    ignitionCtlState = doc["ledOn"];
    redLEDstate = doc["ledOn"];
    digitalWrite(redLEDpin, redLEDstate); // Remove Later
    commandPrefix = "50";
    if (ignitionCtlState)
      commandString = "1";
    else
      commandString = "0";
    fastSetSetting();
    return read_keySw(req, res);
  }
}

// PWM Section

int map_duty_cycle(float duty_cycle)
{
  Serial.print("map_duty_cycle input : ");
  Serial.println(duty_cycle);

  // if (duty_cycle < 0)
  //   duty_cycle = 0;
  // if (duty_cycle > 100)
  //   duty_cycle = 100;
  // int output = map(duty_cycle, 0, 100, 0, 256);
  int output = duty_cycle;
  return output;
}

void read_PWM(Request &req, Response &res)
{
  DynamicJsonDocument response(2048);
  char json[2048];
  Serial.print("Got GET Request for PWM returned: ");

  response["pwm1"]["duty"]["value"] = pwmValue[0];
  response["pwm1"]["freq"]["value"] = pwmFrequency[0];
  response["pwm1"]["switch"]["value"] = PWM1Out;
  response["pwm1"]["switch"]["meta"] = "U41-D5";

  response["pwm2"]["duty"]["value"] = pwmValue[1];
  response["pwm2"]["freq"]["value"] = pwmFrequency[1];
  response["pwm2"]["switch"]["value"] = PWM2Out;
  response["pwm2"]["switch"]["meta"] = "U42-D6";

  response["pwm3"]["duty"]["value"] = pwmValue[2];
  response["pwm3"]["freq"]["value"] = pwmFrequency[2];
  response["pwm3"]["switch"]["value"] = PWM3Out;
  response["pwm3"]["switch"]["meta"] = "U43-D1";

  response["pwm4"]["duty"]["value"] = pwmValue[3];
  response["pwm4"]["freq"]["value"] = pwmFrequency[3];
  response["pwm4"]["switch"]["value"] = PWM4Out;
  response["pwm4"]["switch"]["meta"] = "U44-D2";

  response["pwm5"]["duty"]["value"] = pwmValue[4];
  response["pwm5"]["freq"]["value"] = pwmFrequency[4];
  response["pwm5"]["switch"]["value"] = PWM5Out;
  response["pwm5"]["switch"]["meta"] = "U45-D7";

  response["pwm6"]["duty"]["value"] = pwmValue[5];
  response["pwm6"]["freq"]["value"] = pwmFrequency[5];
  response["pwm6"]["switch"]["value"] = PWM6Out;
  response["pwm6"]["switch"]["meta"] = "U46-D8";

  // serializeJson(response, json);
  serializeJsonPretty(response, json);
  Serial.println(json);
  res.print(json);
}
void update_PWM(Request &req, Response &res)
{
  // Need to Remove after testing Replace with responsible function
  // uint8_t PWMSettings = uint8_t(true | true << 1 | true << 2 | true << 3 |
  //                               true << 4 | true << 5 | true << 6 | true << 7);
  // digitalWrite(CSconfigBPin, LOW);
  // SPI1.transfer(PWMSettings);
  // digitalWrite(CSconfigBPin, HIGH);

  // JsonObject& config = jb.parseObject( &req);
  Serial.print("Got POST Request for PWM: ");
  req.body(buff, sizeof(buff));
  if (!parse_response(buff))
  {
    res.print("Not a valid Json Format");
  }
  else
  {
    if (doc["pwm1"] or doc["pwm1"] > -1)
    {
      float pwm = doc["pwm1"]["duty"]["value"];
      bool sw = doc["pwm1"]["switch"]["value"];
      float pwm_freq = doc["pwm1"]["freq"]["value"];

      if (sw)
        PWM1Out = true;
      else
        PWM1Out = false;
      Serial.print("PWM1 Switch : ");
      Serial.println(PWM1Out);

      commandPrefix = "33";
      commandString = String(map_duty_cycle(pwm));
      fastSetSetting();
      if (pwm_freq != pwmFrequency[0])
      {
        commandPrefix = "81";
        commandString = String(pwm_freq);
        fastSetSetting();
      }
    }

    if (doc["pwm2"])
    {
      float pwm = doc["pwm2"]["duty"]["value"];
      bool sw = doc["pwm2"]["switch"]["value"];
      float pwm_freq = doc["pwm2"]["freq"]["value"];
      if (sw)
        PWM2Out = true;
      else
        PWM2Out = false;
      commandPrefix = "34";
      commandString = String(map_duty_cycle(pwm));
      fastSetSetting();
      if (pwm_freq != pwmFrequency[1]) // check with PWM1 as both are tied to same Timer
      {
        commandPrefix = "82";
        commandString = String( pwmFrequency[1]);
        fastSetSetting();
      }
    }
    if (doc["pwm3"])
    {
      float pwm = doc["pwm3"]["duty"]["value"];
      bool sw = doc["pwm3"]["switch"]["value"];
      float pwm_freq = doc["pwm3"]["freq"]["value"];
      if (sw)
        PWM3Out = true;
      else
        PWM3Out = false;
      commandPrefix = "35";
      commandString = String(map_duty_cycle(pwm));
      fastSetSetting();
      if (pwm_freq != pwmFrequency[2])
      {
        commandPrefix = "83";
        commandString = String(pwm_freq);
        fastSetSetting();
      }
    }
    if (doc["pwm4"])
    {
      float pwm = doc["pwm4"]["duty"]["value"];
      bool sw = doc["pwm4"]["switch"]["value"];
      float pwm_freq = doc["pwm4"]["freq"]["value"];

      if (sw)
        PWM4Out = true;
      else
        PWM4Out = false;
      commandPrefix = "36";
      commandString = String(map_duty_cycle(pwm));
      fastSetSetting();
      if (pwm_freq != pwmFrequency[3])
      {
        commandPrefix = "84";
        commandString = String(pwm_freq);
        fastSetSetting();
      }
    }
    if (doc["pwm5"])
    {
      float pwm = doc["pwm5"]["duty"]["value"];
      bool sw = doc["pwm5"]["switch"]["value"];
      float pwm_freq = doc["pwm5"]["freq"]["value"];

      if (sw)
        PWM5Out = true;
      else
        PWM5Out = false;
      commandPrefix = "87";
      commandString = String(map_duty_cycle(pwm));
      fastSetSetting();
    }
    if (doc["pwm6"])
    {
      float pwm6 = doc["pwm6"]["duty"]["value"];
      bool sw = doc["pwm6"]["switch"]["value"];
      float pwm_freq = doc["pwm6"]["freq"]["value"];

      if (sw)
        PWM6Out = true;
      else
        PWM6Out = false;
      commandPrefix = "88";
      commandString = String(map_duty_cycle(pwm6));
      fastSetSetting();
    }
    setPWMSwitches();
    return read_PWM(req, res);
  }
}

void readRelay(Request &req, Response &res)
{
  Serial.print("Got GET Request for ignitionCtlState returned: ");
  Serial.println(ignitionCtlState);

  res.print(ignitionCtlState);
}

void updateRelay(Request &req, Response &res)
{
  Serial.print("Got PUT Request for ignitionCtlState: ");
  ignitionCtlState = (req.read() != '0');
  Serial.println(ignitionCtlState);
  digitalWrite(ignitionCtlPin, ignitionCtlState);
  digitalWrite(greenLEDpin, ignitionCtlState);
  return readRelay(req, res);
}

bool parse_response(uint8_t *buffer)
{
  // Serial.print((char *)buffer);
  // Serial.println(" EOF");
  //Serial.println("parse_response");
  DeserializationError error = deserializeJson(doc, buffer);
  serializeJsonPretty(doc, Serial);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return false;
  }

  return true;
}

void getter_method(Request &req, Response &res)
{
  Serial.println("getter_method: ");
  req.body(buff, sizeof(buff));
  if (!parse_response(buff))
  {
    res.print("Not a valid Json Format");
  }
  else
  {
    const char *func_name = doc["func_name"];
    double latitude = doc["func_params"]["duty_cycle"];
    double longitude = doc["func_params"]["freq"];

    // Print values.
    Serial.println((String) "func_name: " + func_name + "\n" + "func_params: " + latitude + " " + longitude);
    Serial.println(" EOF");
    res.print("test");
  }
}

void setup()
{
  SPI.begin();
  SPI1.begin();
  Ethernet.init(10); // Most Arduino shields
  Serial.begin(9600);
  setPinModes();
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Ethernet cable is not connected.");
  }

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  app.get("/led", &read_keySw);
  app.put("/led", &update_keySw);
  app.post("/led", &update_keySw);

  app.get("/relay", &readRelay);
  app.put("/relay", &updateRelay);
  app.post("/pwm", &update_PWM);
  app.get("/pwm", &read_PWM);

  // app.get("/user", &readUser);
  // app.put("/user", &updateUser);
  app.route(staticFiles());

  server.begin();
}

void loop()
{
  EthernetClient client = server.available();
  WriteLoggingStream loggingClient(client, Serial);
  // loggingClient.println("GET / HTTP/1.1");
  // loggingClient.println("User-Agent: Arduino");

  // deserializeJson(doc, loggingStream);
  // loggingStream.write("test");
  if (client.connected())
  {
    app.process(&client);
  }
}