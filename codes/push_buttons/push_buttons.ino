#include <SPI.h>

#define PL_pin 10

uint8_t control = 0;
uint8_t shift = 0;
boolean stat[8] = {0}, prev_stat[8] = {0};
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  init_SPI();
}

void loop() {
  // put your main code here, to run repeatedly:
  read_data();
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

void read_data()
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
  Serial.println(control);
}
