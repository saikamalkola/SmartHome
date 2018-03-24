#include <TimerOne.h>

#define N 200

int voltage = 0;
int current[4] = {0};
int last_voltage = 0;
int last_current[4] = {0};

long shiftedV = -10000;
long shiftedC[4] = { -10000, -10000, -10000, -10000};

int index = 0;
int offset = 512;

float Kv = 22/(122 * 0.04115), Kc = (5 * 1000)/ (66 * 1023);
long filtered_voltage = 0;
long filtered_current[4] = {0};

float Vrms = 0;
float Irms[4] = {0};
float real_power[4] = {0};
float app_power[4] = {0};
float power_factor[4] = {0};
void setup()
{
  init_sensors();
  Serial.begin(115200);
  Timer1.initialize(5000);
  Timer1.attachInterrupt(read_data);
}

void loop()
{
  noInterrupts();
  if (index == N)
  {
    index = 0;
    cal_parameters();
  }
  interrupts();
}

void cal_parameters()
{
  Vrms = Vrms / N;
  for(int i = 0; i < 4; i++)
  {
    Irms[i] = Irms[i] / N;
    app_power[i] = Vrms * Irms[i];
    power_factor[i] = real_power[i] / app_power[i];
  }
}

void read_data()
{
  voltage = analogRead(A4);
  long shiftedFCL = shiftedV + (long)((voltage - last_voltage) << 8);
  shiftedV = shiftedFCL - (shiftedFCL >> 8);
  filtered_voltage  = (shiftedV + 128) >> 8;
  filtered_voltage = Kv * filtered_voltage;
  Vrms += (filtered_voltage * filtered_voltage);
  last_voltage = voltage;
  for (int i = 0; i < 4; i++)
  {
    current[i] = analogRead(A0 + i);
    long shiftedFCL = shiftedC[i] + (long)((current[i] - last_current[i]) << 8);
    shiftedC[i] = shiftedFCL - (shiftedFCL >> 8);
    filtered_current[i]  = (shiftedC[i] + 128) >> 8;
    filtered_current[i] = Kc * filtered_current[i];
    Irms[i] += (filtered_current[i] * filtered_current[i]);
    real_power[i] += (filtered_voltage * filtered_current[i]);
    last_current[i] = current[i];
    Serial.print(filtered_current[i]);
    Serial.print(" ");
  }
  Serial.println("");
  index++;
}

void init_sensors()
{
  for (int i = 0; i < 5; i++)
  {
    pinMode(A0 + i, INPUT);
  }
}

