#include <TimerOne.h>
#include <SPI.h>

#define PL_pin 10
#define N 200

//PushButton Array Varibles
uint8_t shift = 0;
boolean stat[8] = {0}, prev_stat[8] = {0};

//Energy Measurement Variables
float Kv = 0.755276, Kc = 0.071833;
//Kv = (4.85 * 118) / (1023 * 18 * 0.04115)
//Kc = (4.85 * 1000) / (66 * 1023)
float voltage = 0;
float current[4] = {0};
float offset = 512;
float kwh[4] = {0};

float Vrms = 0;
float Irms[4] = {0};
float real_power[4] = {0};
float app_power[4] = {0};
float power_factor[4] = {0};

int index = 0;

//Relay Control Variables
int relay[8] = {3, 4, 5, 6, 7, 2, 8, 9};
int control = 0, prev_control = 0;

//Timing Variables
unsigned long dt = 0, present_ms = 0, last_ms = 0, energy_ms = 0, update_time = 5000;

void setup()
{
  init_sensors();
  init_SPI();
  init_relay();
  Serial.begin(115200);
  Timer1.initialize(5000);
  Timer1.attachInterrupt(read_data);
  energy_ms = millis();
}

void loop()
{
  check_serial();
  read_spi_data();
  control_relay();
  noInterrupts();
  if (index == N)
  {
    index = 0;
    cal_parameters();
  }
  interrupts();
  if (millis() - last_ms > update_time)
  {
    send_power_data();
    last_ms = millis();
    //send_energy_data();
  }
}

void send_power_data()
{
  for (int i = 0; i < 4; i++)
  {
    Serial.print("P" + String(i) + " ");
    Serial.println((int)app_power[i]);
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

void check_serial()
{
  if (Serial.available() > 0)
  {
    String temp = Serial.readStringUntil('\n');
    control = temp.toInt();
  }
}

void control_relay()
{
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

void cal_parameters()
{
  dt = millis() - energy_ms;
  energy_ms = millis();
  Vrms = sqrt(Vrms / N);
  for (int i = 0; i < 4; i++)
  {
    Irms[i] = sqrt(Irms[i] / N);
    app_power[i] = Vrms * Irms[i];
    real_power[i] = real_power[i] / N;
    kwh[i] += (app_power[i] * dt * 0.000278 / 1000);
//    power_factor[i] = real_power[i] / app_power[i];
    Irms[i] = 0;
    real_power[i] = 0;
    power_factor[i] = 0;
  }
  Vrms = 0;
}

void read_data()
{
  voltage = analogRead(A4);
  voltage = Kv * (voltage - offset);
  Vrms += (voltage * voltage);
  for (int i = 0; i < 4; i++)
  {
    current[i] = analogRead(A0 + i);
    current[i] = Kc * (current[i] - offset);
    Irms[i] += (current[i] * current[i]);
    real_power[i] += (voltage * current[i]);
  }
  index++;
}

void read_spi_data()
{
  digitalWrite(PL_pin, LOW);
  digitalWrite(PL_pin, HIGH);
  shift = 0;
  shift = SPI.transfer(0x00);
  for (int i = 0; i < 8; i++)
  {
    stat[i] = (shift >> i) & 0x01;
    if (prev_stat[i] != stat[i] && stat[i] == LOW)
    {
      control ^= (1 << i);
    }
    prev_stat[i] = stat[i];
  }
}

void init_SPI()
{
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  SPI.begin();
  pinMode(PL_pin, OUTPUT);
  digitalWrite(PL_pin, HIGH);
}

void init_sensors()
{
  for (int i = 0; i < 5; i++)
  {
    pinMode(A0 + i, INPUT);
  }
}

void init_relay()
{
  for (int i = 0; i < 8; i++)
  {
    pinMode(relay[i], OUTPUT);
    digitalWrite(relay[i], HIGH);
  }
}

