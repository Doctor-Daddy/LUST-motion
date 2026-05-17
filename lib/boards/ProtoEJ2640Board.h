#pragma once

/**
 *   LUST-motion on ProtoEJ2640
 * 
 *   by Doctor Daddy the Alfa Man
 **/

/**
 *   Board configuration for the proto-EJ2640 platform still in development by Dr. Daddy
 *
 *   Reference notes: OSSM board supports RS232 modbus communication and current/voltage sensor which proto-EJ2640 does not (or does it idk)
 **/

// Motor properties
#define STEP_PER_REV 12800       // 2000 steps per revolution
#define PULLEY_TEETH 40         // 20 teeth on the pulley
#define INVERT_DIRECTION true   // invert direction of the motor or S6
#define KEEP_OUT 5.0            // keep out distance in mm (??????)
#define SENSORLESS_TRIGGER 5.0  // trigger percentage for sensorless homing (Torque or Current) (??????)
#define BELT_PITCH 5.08            // belt pitch in mm
#define MAX_RPM 60 //300.0          // maximum RPM
#define MAX_ACCELERATION 10000 // maximum acceleration in mm/s^2 -- probably 1500-3000-8000-10000
#define ENABLE_ACTIVE_LOW false  // enable pin is active low

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

// Current sensor properties -- As it turns out, you do need these to exist even if not used.
#define AMPERE_PER_MILLIVOLT -1     // ADC conversion factor current reading: 2.5mA per mV
#define AMPERE_OFFSET_IN_MILLIVOLT -1 // ADC offset in mV for 0A
#define VOLT_PER_MILLIVOLT -1       // ADC conversion factor voltage reading: 40mV per V
#define MAX_AMPERE -1                  // maximum range current sensor in Ampere
#define MAX_VOLTAGE -1                // maximum range voltage sensor in Volt

// Available motor drivers - VIRTUAL driver is enabled by default
#define DRIVER_PROTO_EJ2640
//#define DRIVER_GENERIC_STEPPER   // Generic stepper driver with endstop switch
// #define DRIVER_OSSM_REF_BOARD_V2 // OSSM Reference Board V2 with sensorless homing via current sensor
