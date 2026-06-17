/**
 *   PROTO_EJ2640 Driver for StrokeEngine
 *   Driver for fucking machine powered by EJ2640 electronics
 *   Based on Generic Stepper Motor Driver of StrokeEngine
 *   A library to create a variety of stroking motions with a stepper or servo motor on an ESP32.
 *   https://github.com/theelims/StrokeEngine
 *
 */

#ifndef PROTO_EJ2640_H
#define PROTO_EJ2640_H

#include <Arduino.h>
#include <FastAccelStepper.h>

#include "StrokeEngineMotor.h"

using ProtoMeasureCallbackType = std::function<void(bool)>;

/**************************************************************************/
/*!
  @brief  Struct defining the motor (stepper or servo with STEP/DIR
  interface) and the motion system translating the rotation into a
  linear motion.
*/
/**************************************************************************/
typedef struct
{
  bool enableActiveLow; /*> Polarity of the enable signal. True for active low. */
  int stepPin;          /*> Pin connected to the STEP input */
  int directionPin;     /*> Pin connected to the DIR input */
  int enablePin;        /*> Pin connected to the ENA input */
} ProtoEJ2640Properties;

/**************************************************************************/
/*!
  @brief  ProtoEJ2640Motor inherits from MotorInterface and provides a generic
  STEP/DIR interface tuned for EJ2640-driven heavy duty machine.
  Potentially useful for any common stepper (and servo?) drivers. 
  Uses FastAccelStepper for trapezoidal motion planning and hardware step signal generation. 
  The ProtoEJ2640Motor class uses a physical endstop switch at home position for homing. 
  It also implements a physical endstop switch at END position (maximum extent of travel) for
  future features to measure travel length and for safety during operation.
*/
/**************************************************************************/
class ProtoEJ2640Motor : public MotorInterface
{
public:
  ProtoEJ2640Motor() {}

  // Init
  void begin(ProtoEJ2640Properties *ProtoEJ2640)
  {
    _motor = ProtoEJ2640;

    // Setup FastAccelStepper
    engine.init();
    _stepper = engine.stepperConnectToPin(_motor->stepPin);
    if (_stepper)
    {
      _stepper->setDirectionPin(_motor->directionPin, !_invertDirection);
      _stepper->setEnablePin(_motor->enablePin, _motor->enableActiveLow);
      _stepper->setAutoEnable(false);
      _stepper->disableOutputs();
      ESP_LOGI("ProtoEJ2640", "FastAccelStepper Engine successfully initialized!");
    }
    else
    {
      ESP_LOGE("ProtoEJ2640", "Failed to load FastAccelStepper Engine!");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void setMachineGeometry(float travel, float keepout = 5.0)
  {
    _travel = travel;
    _keepout = keepout;
    _maxPosition = travel - (keepout * 2);
    _minStep = 0;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("ProtoEJ2640", "Machine Geometry Travel = %f", _travel);
    ESP_LOGD("ProtoEJ2640", "Machine Geometry Keepout = %f", _keepout);
    ESP_LOGD("ProtoEJ2640", "Machine Geometry MaxPosition = %f", _maxPosition);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines steps per millimeter of travel. This is used
    to translate between metric units and steps.
    @param  stepsPerMillimeter steps per millimeter of travel.
  */
  /**************************************************************************/
  void setStepsPerMillimeter(int stepsPerMillimeter = 50)
  {
    _stepsPerMillimeter = stepsPerMillimeter;
    _maxStep = int(0.5 + _maxPosition * _stepsPerMillimeter);
    _maxStepPerSecond = int(0.5 + _maxSpeed * _stepsPerMillimeter);
    _maxStepAcceleration = int(0.5 + _maxAcceleration * _stepsPerMillimeter);
    ESP_LOGD("ProtoEJ2640", "Steps per Millimeter = %i", _stepsPerMillimeter);
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
    @param homePosition Position of the homing switch in [mm]. Default is 0.0.
    @param speed Speed of the homing procedure in [mm/s]. Default is 5.0.
  */
  /**************************************************************************/
  void setSensoredHoming(int homePin, uint8_t arduinoPinMode = INPUT, bool activeLow = false, float homePosition = 0.0, float speed = 5.0) // Assumes always homing to back of machine for safety
  {
    // set homing pin as input
    _homingPin = homePin;
    pinMode(_homingPin, arduinoPinMode);
    _homingActiveLow = activeLow;
    _homePosition = int(0.5 + homePosition / float(_stepsPerMillimeter));
    _homingSpeed = speed * _stepsPerMillimeter;
    ESP_LOGI("ProtoEJ2640", "Homing switch on pin %i in pin mode %i is %s", _homingPin, arduinoPinMode, _homingActiveLow ? "active low" : "active high");
    ESP_LOGI("ProtoEJ2640", "Search home with %05.1f mm/s at %05.1f mm.", speed, homePosition);
  }

  void setMaxEndstop(int maxEndstopPin, uint8_t arduinoPinMode = INPUT, bool activeLow = false)
  {
    _maxEndstopPin = maxEndstopPin;
    pinMode(_maxEndstopPin, arduinoPinMode);
    _maxEndstopActiveLow = activeLow;
    ESP_LOGI("ProtoEJ2640", "Max endstop switch on pin %i in pin mode %i is %s", _maxEndstopPin, arduinoPinMode, _maxEndstopActiveLow ? "active low" : "active high");
  }

  /**************************************************************************/
  /*!
   @brief Measures travel from the home endstop to the max endstop.
   @param callBackMeasuring Callback called when measuring completes. The bool
   argument is true if the max endstop was found.
   @param keepout Soft endstop distance in mm.
  */
  /**************************************************************************/
  void measureRailLength(ProtoMeasureCallbackType callBackMeasuring, float keepout = 5.0)
  {
    if (_error || _enabled == false || motionCompleted() == false || _maxEndstopPin < 0)
    {
      ESP_LOGE("ProtoEJ2640", "Rail measurement not possible!");
      if (callBackMeasuring != NULL)
      {
        callBackMeasuring(false);
      }
      return;
    }

    _callBackMeasuring = callBackMeasuring;
    _keepout = keepout;

    ESP_LOGI("ProtoEJ2640", "Measuring rail length...");

    xTaskCreatePinnedToCore(
        this->_measureProcedureImpl,
        "Measuring",
        4096,
        this,
        1,
        &_taskMeasuringHandle,
        1);
    ESP_LOGD("ProtoEJ2640", "Created Measuring Task.");
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
  */
  /**************************************************************************/
  void home()
  {
    // set homed to false so that isActive() becomes false
    _homed = false;

    // first stop current motion and suspend motion tasks
    stopMotion();

    // Quit if stepper not enabled
    if (_enabled == false)
    {
      ESP_LOGE("ProtoEJ2640", "Homing not possible! --> Enable stepper first!");
      return;
    }

    // Create homing task
    xTaskCreatePinnedToCore(
        this->_homingProcedureImpl, // Function that should be called
        "Homing",                   // Name of the task (for debugging)
        4096,                       // Stack size (bytes)
        this,                       // Pass reference to this class instance
        1,                          // Pretty high task priority
        &_taskHomingHandle,         // Task handle
        1                           // Have it on application core
    );
    ESP_LOGD("ProtoEJ2640", "Created Homing Task.");
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void enable()
  {
    ESP_LOGI("ProtoEJ2640", "Stepper Enabled!");
    // Enable stepper
    _enabled = true;
    _stepper->enableOutputs();
  }

  /**************************************************************************/
  /*!
    @brief  Sets the machines mechanical geometries. The values are measured
    from hard endstop to hard endstop and are given in [mm].
    @param travel overal mechanical travel in [mm].
    @param keepout This keepout [mm] is a soft endstop and subtracted at both ends
    of the travel. A typical value would be 5mm.
  */
  /**************************************************************************/
  void disable()
  {
    ESP_LOGI("ProtoEJ2640", "Stepper Disabled!");
    // Disable stepper
    _enabled = false;
    _stepper->disableOutputs();

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
      ESP_LOGD("ProtoEJ2640", "Deleted Homing Task.");
    }

    if (_taskMeasuringHandle != NULL)
    {
      vTaskDelete(_taskMeasuringHandle);
      _taskMeasuringHandle = NULL;
      ESP_LOGD("ProtoEJ2640", "Deleted Measuring Task.");
    }

    // Suspend motion feedback task if it exists already
    if (_taskPositionFeedbackHandle != NULL)
    {
      vTaskSuspend(_taskPositionFeedbackHandle);
      ESP_LOGD("ProtoEJ2640", "Suspended Position Feedback Task.");
    }
  }

  /**************************************************************************/
  /*!
    @brief  Initiates the fastest safe breaking to stand-still stopping all
    motion without loosing position.
  */
  /**************************************************************************/
  void stopMotion()
  {

    ESP_LOGW("ProtoEJ2640", "STOP MOTION!");

    // Delete homing task should the homing sequence be running
    if (_taskHomingHandle != NULL)
    {
      ESP_LOGD("ProtoEJ2640", "Deleted Homing Task: %p", _taskHomingHandle);
      vTaskDelete(_taskHomingHandle);
      _taskHomingHandle = NULL;
    }

    if (_taskMeasuringHandle != NULL)
    {
      ESP_LOGD("ProtoEJ2640", "Deleted Measuring Task: %p", _taskMeasuringHandle);
      vTaskDelete(_taskMeasuringHandle);
      _taskMeasuringHandle = NULL;
    }

    if (_stepper->isRunning())
    {
      // Stop servo motor as fast as legally allowed
      _stepper->setAcceleration(_maxStepAcceleration);
      _stepper->applySpeedAcceleration();
      _stepper->stopMove();
      ESP_LOGD("ProtoEJ2640", "Bring stepper to a safe halt.");
    }

    // Wait for servo stopped
    while (_stepper->isRunning())
      ;
  }

  /**************************************************************************/
  /*!
    @brief  Returns if a trapezoidal motion is carried out, or the machine is
    at stand-still.
    @return `true` if motion is completed, `false` if still under way
  */
  /**************************************************************************/
  bool motionCompleted() { return _stepper->isRunning() ? false : true; }

  /**************************************************************************/
  /*!
    @brief  Returns the currently used acceleration.
    @return acceleration of the motor in [mm/s²]
  */
  /**************************************************************************/
  float getAcceleration() { return float(_stepper->getAcceleration()) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current speed the machine.
    @return speed of the motor in [mm/s]
  */
  /**************************************************************************/
  float getSpeed() { return (float(_stepper->getCurrentSpeedInMilliHz()) * 1.0e-3) / float(_stepsPerMillimeter); }

  /**************************************************************************/
  /*!
    @brief  Returns the current position of the machine.
    @return position in [mm]
  */
  /**************************************************************************/
  float getPosition() { return float(_stepper->getCurrentPosition()) / float(_stepsPerMillimeter); }

  // Misc
  // FastAccelStepperEngine &fastAccelStepperEngineReference();

private:
  /**************************************************************************/
  /*!
    @brief  Internal function that updates the trapezoidal motion path
    generator. Here this is done by calling the appropriate FastAccelStepper
    API calls and translate between metric units and steps
    @param position in [mm]
    @param speed in [mm/s]
    @param acceleration in [mm/s²]
  */
  /**************************************************************************/
  void _unsafeGoToPosition(float position, float speed, float acceleration)
  {
    // Translate between metric and steps
    unsigned int speedInHz = uint32_t(0.5 + speed * _stepsPerMillimeter);
    int stepAcceleration = int(0.5 + acceleration * _stepsPerMillimeter);
    int positionInSteps = int(0.5 + position * _stepsPerMillimeter);
    ESP_LOGD("ProtoEJ2640", "Going to unsafe position %i steps @ %i steps/s, %i steps/s^2", positionInSteps, speedInHz, stepAcceleration);

    // write values to stepper
    _stepper->setSpeedInHz(speedInHz);
    _stepper->setAcceleration(stepAcceleration);
    _stepper->moveTo(positionInSteps);
  }

  void _reportMotionPoint()
  {
    // Call notification callback, if it was defined.
    if (_cbMotionPoint != NULL)
    {
      _cbMotionPoint(millis(), getPosition(), getSpeed(), 0.0, 0.0);
    }
  }

  bool _queryHome() {
    ESP_LOGV("ProtoEJ2640", "Querying homing switch.");
    return (digitalRead(_homingPin) == !_homingActiveLow) ? true : false;
  }

  bool _queryMaxEndstop() {
    ESP_LOGV("ProtoEJ2640", "Querying max endstop switch.");
    return (digitalRead(_maxEndstopPin) == !_maxEndstopActiveLow) ? true : false;
  }

  void _homingProcedure() {
    // Set feedrate for homing
    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    // Check if we are already at the home position
    if (_queryHome()) {
      ESP_LOGD("ProtoEJ2640", "Already at home position. Backing up and try again.");
      // back off 2*keepout from switch
      _stepper->move(_stepsPerMillimeter * 2 * _keepout);

      // wait for move to complete
      while (_stepper->isRunning()) {
        // Pause the task for 100ms while waiting for move to complete
        vTaskDelay(100 / portTICK_PERIOD_MS);
      }

      // move back towards endstop
      _stepper->move(-_stepsPerMillimeter * 4 * _keepout);
    }
    else {
      ESP_LOGD("ProtoEJ2640", "Start searching for home.");
      // Move maximum travel distance + 2*keepout towards the homing switch
      _stepper->move(-_stepsPerMillimeter * (_maxPosition + 4 * _keepout));
    }

    // Poll homing switch
    while (_stepper->isRunning()) {

      // Are we at the home position?
      if (_queryHome()) {
        ESP_LOGD("ProtoEJ2640", "Found home!");
        // Set home position
        // Switch is at -KEEPOUT
        _stepper->forceStopAndNewPosition(_stepsPerMillimeter * int(_homePosition - _keepout));

        // drive free of switch and set axis to lower end
        _stepper->moveTo(_minStep); // equivalent to _stepper->moveTo(0);

        while (_stepper->isRunning()) {
            vTaskDelay(20 / portTICK_PERIOD_MS);
        }

        _homed = true;

        // Break loop, home was found
        break;
      }

      // Pause the task for 20ms to allow other tasks
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    // disable Servo if homing has not found the homing switch
    if (!_homed) {
      _stepper->disableOutputs();
      ESP_LOGE("ProtoEJ2640", "Homing failed! Did not find home position.");
    }

    // Call notification callback, if it was defined.
    if (_callBackHoming != NULL) {
      _callBackHoming();
    }

    // delete one-time task
    _taskHomingHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("ProtoEJ2640", "Homing task self-terminated");
  }

  void _measureProcedure() {
    bool measured = false;

    TaskHandle_t measuringTaskHandle = _taskMeasuringHandle;
    _taskMeasuringHandle = NULL;
    home();
    _taskMeasuringHandle = measuringTaskHandle;

    while (_taskHomingHandle != NULL)
    {
      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    if (!_homed)
    {
      ESP_LOGE("ProtoEJ2640", "Rail measurement failed because homing failed.");
      if (_callBackMeasuring != NULL)
      {
        _callBackMeasuring(false);
      }
      _taskMeasuringHandle = NULL;
      vTaskDelete(NULL);
      return;
    }

    _stepper->setSpeedInHz(_homingSpeed);
    _stepper->setAcceleration(_maxStepAcceleration);

    float searchDistance = 2000.0f; // Max search distance. Machine stops when MAX_ENDSTOP_PIN triggers. This is just a fault bound - assumes all machines have travel < 2000 mm.

    ESP_LOGI("ProtoEJ2640", "Start measuring rail length toward max endstop...");
    _stepper->move(int(0.5f + searchDistance * _stepsPerMillimeter));

    while (_stepper->isRunning())
    {
      if (_queryMaxEndstop())
      {
        _stepper->stopMove();
        while (_stepper->isRunning())
        {
          vTaskDelay(20 / portTICK_PERIOD_MS);
        }

        float travel = getPosition() + _keepout;
        float roundedTravel = floor(travel);
        float adjustedKeepout = (roundedTravel - floor((roundedTravel - 2.0f * _keepout) * 0.1f) * 10.0f) / 2.0f;

        ESP_LOGI("ProtoEJ2640", "Measured rail length: %.2f mm (%.2f in), rounded to %.1f mm", travel, travel / 25.4f, roundedTravel);
        ESP_LOGI("ProtoEJ2640", "Adjusted keepout: %.2f mm", adjustedKeepout);

        setMachineGeometry(roundedTravel, adjustedKeepout);
        _stepper->moveTo(_maxStep);
        measured = true;
        break;
      }

      vTaskDelay(20 / portTICK_PERIOD_MS);
    }

    if (!measured)
    {
      ESP_LOGE("ProtoEJ2640", "Rail measurement failed! Did not find max endstop.");
    }

    if (_callBackMeasuring != NULL)
    {
      _callBackMeasuring(measured);
    }

    _taskMeasuringHandle = NULL;
    vTaskDelete(NULL);
    ESP_LOGV("ProtoEJ2640", "Measuring task self-terminated");
  }

  /**************************************************************************/
  FastAccelStepper *_stepper;
  ProtoEJ2640Properties *_motor;
  FastAccelStepperEngine engine = FastAccelStepperEngine();
  int _stepsPerMillimeter = 50;
  int _minStep;
  int _maxStep;
  int _maxStepPerSecond;
  int _maxStepAcceleration;
  static void _homingProcedureImpl(void *_this) { static_cast<ProtoEJ2640Motor *>(_this)->_homingProcedure(); }
  static void _measureProcedureImpl(void *_this) { static_cast<ProtoEJ2640Motor *>(_this)->_measureProcedure(); }
  unsigned int _homingSpeed;
  float _homePosition;
  int _homingPin = -1;
  bool _homingActiveLow; /*> Polarity of the homing signal*/
  int _maxEndstopPin = -1;
  bool _maxEndstopActiveLow; /*> Polarity of the max endstop signal*/
  TaskHandle_t _taskHomingHandle = NULL;
  TaskHandle_t _taskMeasuringHandle = NULL;
  ProtoMeasureCallbackType _callBackMeasuring = NULL;
};

#endif // PROTO_EJ2640_H
