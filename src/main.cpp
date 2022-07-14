#define THINGER_SERIAL_DEBUG

#include <ThingerESP8266.h>
#include "arduino_secrets.h"

const char *plant_tags[] = {"th-1-fougere", "th-2-rose", "th-3-draco", "th-4-cactus"};
const char *water_tags[] = {"w-1-fougere", "w-2-rose", "w-3-draco", "w-4-cactus"};
const char *pump_tags[] = {"p-1-fougere", "p-2-rose", "p-3-draco", "p-4-cactus"};
int thresholds[] = {610, 470, 600, 540};
int water_time[] = {5, 10, 5, 5};

ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

const int PIN_VCC = D0;
const int PIN_A = D1;
const int PIN_B = D2;
const int PIN_C = D3;

const int PIN_PUMP[] = {D5, D6, D7, D8};

const int PIN_IN = A0;

int read_sensor(int channel)
{
  if (channel < 4)
  {
    digitalWrite(PIN_C, (channel >> 2) & 1);
    digitalWrite(PIN_B, (channel >> 1) & 1);
    digitalWrite(PIN_A, channel & 1);

    int value = analogRead(PIN_IN);

    return value;
  }
  else
  {
    return -1;
  }
}

void setup()
{
  // open serial for monitoring
  Serial.begin(115200);

  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_VCC, OUTPUT);

  digitalWrite(PIN_VCC, LOW);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  digitalWrite(PIN_B, LOW);

  for (int channel = 0; channel < 4; channel++)
  {
    pinMode(PIN_PUMP[channel], OUTPUT);
    digitalWrite(PIN_PUMP[channel], HIGH);
  }

  pinMode(PIN_IN, INPUT);

  // add WiFi credentials
  thing.add_wifi(SSID, SSID_PASSWORD);

  thing["thresholds"] << [](pson &in)
  {
    if (in.is_empty())
    {
      for (int i = 0; i < 4; i++)
      {
        in[plant_tags[i]] = thresholds[i];
      }
    }
    else
    {
      for (int i = 0; i < 4; i++)
      {
        thresholds[i] = in[plant_tags[i]];
      }
    }
  };

  thing["timing"] << [](pson &in)
  {
    if (in.is_empty())
    {
      for (int i = 0; i < 4; i++)
      {
        in[plant_tags[i]] = water_time[i];
      }
    }
    else
    {
      for (int i = 0; i < 4; i++)
      {
        water_time[i] = in[plant_tags[i]];
      }
    }
  };

  thing["watering"] >> [](pson &out)
  {
    digitalWrite(PIN_VCC, HIGH);

    for (int channel = 0; channel < 4; channel++)
    {
      int water = read_sensor(channel);
      bool pump = false;
      if (water > thresholds[channel])
      {
        digitalWrite(PIN_PUMP[channel], LOW);
        delay(1000 * water_time[channel]);
        digitalWrite(PIN_PUMP[channel], HIGH);
        pump = true;
      }
      out[water_tags[channel]] = water;
      out[pump_tags[channel]] = pump;
    }
  };

  digitalWrite(PIN_VCC, LOW);
}

void loop()
{
  thing.handle();
}