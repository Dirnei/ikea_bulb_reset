# Sonoff Ikea TRÅDFRI reset

Reset a Ikea Tradfri bulb can be quite frustrating. To solve this issue, i wrote this firmware for the Sonoff S20 i had lying around. It should also work for other sonoffs but i don't tested it. Please let me know if you tried it on a other device, so i can mention it here.

I wrote it first without any networking features, to quickly prototype the reset procedure. After that i added MQTT support, because thats how i use the sonoff devices. Usually with TASMOTA, but the current implementation is all i use from it anyway.

## Usage

Clone this repo and flash it to your S20 with your favourite programmer. E.g. I use VS Code with the arduino extension.

Plug it into your wall outlet, long press the button on the front and it will start the reset procedure.

## Usage with MQTT enabled

You have to uncomment the first line in edit config.h to enable MQTT support. Edit also your wireless configuration and if you use no mqtt security change  

```c
 #def USE_MQTT_AUTH
 ```

to

```c
 //#def USE_MQTT_AUTH
 ```

### Set relay state

To set the relay state publish to your configured __mqttTopic__ e.g. "sonoff/20/relay" with following payload:

``` json
 { "state": "ON" }
```

Available states:

- ON
- OFF
- TOGGLE

### Run reset procedure

To run the reset procedure for the Ikea Tradfri Bulb you have to publish to __mqttResetModeTopic__ e.g. "sonoff/s20/mode/reset" wih follow payload:

``` json
{
    "lockDuration": 20000,
    "cycles": 6,
    "delayOn": 500,
    "delayOff": 500,
}
```

You don't have to set all properties. All properties that are not set will be set to the default value described below. That means, if you are happy with the default values, you can also send a empty payload.

#### lockDuration

Time in milliseconds for how long the relay state is locked for changes after the reset procedure finished. Default: __20000__

#### cycles

The amount of ON OFF cycles that will be performed during the reset procedure. Default: __6__ (for Ikea Tradfri)

#### delayOn

Time in milliseconds for the delay after the relay is turned *ON* in the reset procedure. Default: __500__

#### delayOff

Time in milliseconds for the delay after the relay is turned *OFF* in the reset procedure. Default: __500__

### MQTT Feedback

When the relay toggles you will get a status message to your __mqttTopic__ with the prefix __/status__.
During the reset procedure you will not get any status updates, only when it finishes you will get a message with status __LOCKED__ and when the lock is removed you will be notified again with the current relay state.
