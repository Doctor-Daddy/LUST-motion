#pragma once

/**
 *   LUST-motion
 *
 *   Copyright (C) 2025 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the MIT license. See the LICENSE file for details.
 **/

/**
 *   Board configuration for the OSSM Reference Board V1 & V2
 *   from Research and Desire
 *
 *   It supports the basic functions like stepper motor driver interface,
 *   Neopixel-LED, RS232 modbus communication, endstop and current/voltage sensor.
 *
 *   Depending on the version of the board, the following features are available:
 *
 *   V1:
 *   - stepper motor driver
 *   - Neopixel-LED
 *   - RS232 modbus communication
 *   - endstop switch
 *   - Motor drivers: Generic stepper
 *
 *   V2:
 *   - stepper motor driver
 *   - Neopixel-LED
 *   - RS232 modbus communication
 *   - endstop switch
 *   - current/voltage sensor
 *   - Motor drivers: Generic stepper and OSSM Reference Board V2 with current sensor
 *
 **/

// Motor properties
#define STEP_PER_REV 12800       // 2000 steps per revolution
#define PULLEY_TEETH 40         // 20 teeth on the pulley
#define INVERT_DIRECTION true   // invert direction of the motor or S6
#define KEEP_OUT 5.0            // keep out distance in mm (??????)
#define SENSORLESS_TRIGGER 5.0  // trigger percentage for sensorless homing (Torque or Current) (??????)
#define BELT_PITCH 5.08            // belt pitch in mm
#define MAX_RPM 300.0          // maximum RPM
#define MAX_ACCELERATION 10000 // maximum acceleration in mm/s^2 -- probably 1500-3000-8000-10000
#define ENABLE_ACTIVE_LOW true  // enable pin is active low

// Pin definitions
#define STEP_PIN 48        // step pin
#define DIRECTION_PIN 47   // direction pin
#define ENABLE_PIN 21      // enable pin
#define ALARM_PIN -1       // alarm pin (??????)
#define IN_POSITION_PIN 41  // in position pin (??????)
#define ENDSTOP_PIN 42     // endstop pin
#define ADC_PIN_CURRENT -1 // current sensor ADC pin (??????)
#define ADC_PIN_VOLTAGE -1 // voltage sensor ADC pin (??????)
#define MODBUS_RX_PIN -1   // modbus rx pin (??????)
#define MODBUS_TX_PIN -1   // modbus tx pin (??????)
#define NEOPIXEL_PIN 38    // neopixel data pin

// Current sensor properties -- kept only to satisfy references if any code compiles them
#define AMPERE_PER_MILLIVOLT 2.5e-3     // ADC conversion factor current reading: 2.5mA per mV
#define AMPERE_OFFSET_IN_MILLIVOLT 1666 // ADC offset in mV for 0A
#define VOLT_PER_MILLIVOLT 4.0e-2       // ADC conversion factor voltage reading: 40mV per V
#define MAX_AMPERE 5.0                  // maximum range current sensor in Ampere
#define MAX_VOLTAGE 40.0                // maximum range voltage sensor in Volt

// Available motor drivers - VIRTUAL driver is enabled by default
// #define DRIVER_GENERIC_STEPPER   // Generic stepper driver with endstop switch
// #define DRIVER_OSSM_REF_BOARD_V2 // OSSM Reference Board V2 with sensorless homing via current sensor
