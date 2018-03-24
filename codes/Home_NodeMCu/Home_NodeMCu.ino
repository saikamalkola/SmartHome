#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "kamal";
const char* password = "kamal123";
const char* mqtt_server = "192.168.43.179";

String response = "";
long power[4] = {20}, energy[4] = {0}, actuation_data = 0, prev_data = 0;
long data_UI = 0;
String cmd = "";
long data = 0;
char msg[50];
long last_ms = 0, update_ms = 5000;
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (Serial.available())
  {
    response = Serial.readStringUntil('\n');
    parse_response();
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (millis() - last_ms > update_ms) {
    last_ms = millis();
    pub_power();
    pub_energy();
  }
  if (prev_data != actuation_data)
  {
    pub_actuation_data();
  }
  prev_data = actuation_data;
}

void parse_response()
{
  int l = response.length(), k = 0;
  int limits[100];
  for (int i = 0; i < l - 1 ; i++)
  {
    if (response[i] == ' ')
    {
      limits[k] = i + 1;
      k++;
    }
  }
  String temp = response.substring(0, limits[0] - 1);
  cmd = temp;
  temp = response.substring(limits[0], limits[1] - 1);
  data = temp.toInt();
  action();

}

void action()
{
  if (cmd == "P0")
  {
    power[0] = data;
  }
  if (cmd == "P1")
  {
    power[1] = data;
  }
  if (cmd == "P2")
  {
    power[2] = data;
  }
  if (cmd == "P3")
  {
    power[3] = data;
  }
  if (cmd == "relay")
  {
    actuation_data = data;
  }
  if (cmd == "E0")
  {
    energy[0] = data;
  }
  if (cmd == "E1")
  {
    energy[1] = data;
  }
  if (cmd == "E2")
  {
    energy[2] = data;
  }
  if (cmd == "E3")
  {
    energy[3] = data;
  }
}

void pub_power()
{
  for (int i = 0; i < 4; i++)
  {
    snprintf (msg, 75, "%ld", power[i]);
    char temp[15] = "device/power/";
    temp[13] = i + '0';
    client.publish(temp, msg);
  }
}

void pub_energy()
{
  for (int i = 0; i < 4; i++)
  {
    snprintf (msg, 75, "%ld", energy[i]);
    char temp[15] = "device/energy/";
    temp[14] = i + '0';
    client.publish(temp, msg);
  }
}

void pub_actuation_data()
{
  snprintf (msg, 75, "%ld", actuation_data);
  char temp[18] = "device/controlFB";
  client.publish(temp, msg);
}

void callback(char* topic, byte* payload, unsigned int length) {
  data_UI = 0;
  for (int i = 0; i < length; i++) {
    data_UI += ((payload[i]-'0') * pow(10,(length - i - 1)));
  }
  Serial.println(data_UI);
}

void reconnect()
{
  while (!client.connected())
  {
    if (client.connect("ESP8266Client")) {
      client.subscribe("devices/control");
    }
    else
    {
      //delay(5000);
    }
  }
}

void setup_wifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
}


