#define N 1024

int relay[8] = {3, 4, 5, 6, 7, 2, 8, 9};
int control = 0, prev_control = 0;

float kwh[4] = {0};
uint8_t sense_pin[4] = {A0, A1, A2, A3};


unsigned long dt = 0, present_ms = 0, last_ms = 0, moist_ms = 0, update_time = 5000;

float voltage[4] = {0};
long offset = 512;
float division = 66;
float rms_current[4] = {0}, rms_voltage = 230;
float current[4] = {0};
float real_power[4] = {0};

float temp = 0;

void setup() {
  // put your setup code here, to run once:
  init_sensors();
  init_relay();
  Serial.begin(115200);
}

void loop()
{
  present_ms = millis();
  cal_kwh();
  check_serial();
  if (millis() - last_ms > update_time)
  {
    send_power_data();
    last_ms = millis();
    //send_energy_data();
  }
  // Serial.println(millis() - present_ms);
}

void check_serial()
{
  if (Serial.available() > 0)
  {
    String temp = Serial.readStringUntil('\n');
    control = temp.toInt();
  }
  if (prev_control != control)
  {
    int temp = control;
    for (int i = 0; i < 8; i++)
    {
      if (temp & 0x01)
      {
        digitalWrite(relay[i], LOW);
      }
      else
      {
        digitalWrite(relay[i], HIGH);
      }
      temp = temp >> 1;
    }
  }
  prev_control = control;
}

void init_relay()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], HIGH);
  }
}

void init_sensors()
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(sense_pin[i], INPUT);
  }

}
void cal_kwh()
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      voltage[j] = analogRead(sense_pin[j]);
      voltage[j] = (voltage[j] - offset);
      voltage[j] = voltage[j] * 5 / 1023;
      voltage[j] = voltage[j] * 1000 / division;
      temp = voltage[j];
      current[j] += temp;
      rms_current[j] += temp * temp;
    }
  }
  for (int i = 0; i < 4; i++)
  {
    rms_current[i] = sqrt(rms_current[i] / N);
    current[i] = current[i] / N;
    real_power[i] = rms_current[i] * rms_voltage;
  }
}

void send_power_data()
{
  for (int i = 0; i < 4; i++)
  {
    Serial.print("P" + String(i) + " ");
    Serial.println((int)real_power[i]);
  }
}

void send_energy_data()
{
  for (int i = 0; i < 4; i++)
  {
    Serial.print("E" + String(i) + " ");
    Serial.println((int)kwh[i]);
  }
}
