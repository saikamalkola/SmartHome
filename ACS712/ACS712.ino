#define N 1024

float kwh1 = 0, kwh2 = 0;
uint8_t sense_pin[2] = {A0,A1};


unsigned long dt = 0,present_ms = 0, last_ms = 0, moist_ms = 0, update_time = 5000;

float voltage[2] = {0,0};
long offset = 510;
float division = 66;
float rms_current[2] = {0,0},rms_voltage = 220;
float current[2] = {0,0};
float real_power[2] = {0,0};

float temp = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(sense_pin, INPUT);
  Serial.begin(9600);
}

void loop() {
  
  cal_kwh_par();
  send_data();
  delay(1000);
}

void cal_kwh_par()
{
  for (int i = 0; i < N; i++)
  {
    for(int j = 0; j < 2; j++)
    {
    voltage[j]= analogRead(sense_pin[j]);
    
    voltage[j] = (voltage[j] - offset);
    voltage[j] = voltage[j] * 5 / 1023;
    voltage[j] = voltage[j] * 1000 / division;
    temp = voltage[j];
    current[j] += temp;
    rms_current[j] += temp * temp;
  }
  }

  rms_current[0] = sqrt(rms_current[0] / N);
  rms_current[1] = sqrt(rms_current[1] / N);
  current[0] = current[0] / N;
  current[1] = current[1] / N;
  real_power[0] = rms_current[0] * rms_voltage;
  real_power[1] = rms_current[1] * rms_voltage;
}

void send_data()
{
  Serial.print("#");
  Serial.print(real_power[0]);
  Serial.print("#");
  Serial.print(real_power[1]);
  for (int i = 0; i < 6; i++)
  {
    Serial.print("#");
    Serial.print(analogRead(A3) / 10);
  }
  Serial.print("\n");
}
