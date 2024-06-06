Project Name: CarDash
Author: Gabriele Parietti
Version: V4.0
Date: 06/06/2024
 
Description:
CarDash is an innovative device based on the ESP32 microcontroller, designed to offer information and entertainment while on the go. 
Mounted on your car's dashboard and projecting data directly onto your windshield, CarDash offers quick and secure access to a host of useful features. 
With an intuitive and customizable interface, CarDash allows users to select and view the information they want, such as today's horoscope, local weather forecast, date and time, jokes to cheer up the journey, important alerts and 
latest news. 
Working via WiFi connection, it also supports hotspots and can be used in non-projector-on-glass mode to suit your travel needs. 
Safe, convenient and highly customizable, CarDash is the ideal companion for every road trip.

Copyright © [2024] [Gabriele Parietti]

Libraries needed to compile the project.
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MD_Word.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
