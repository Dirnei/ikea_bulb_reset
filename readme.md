# Sonoff Ikea TRÅDFRI reset

Resetting an Ikea Trådfri bulb can be quite frustrating. To solve this issue, I wrote this firmware for the Sonoff S20 / S26 I had lying around. It should also work for other sonoffs, but I have not tested that. Please let me know if you tried it on another device so that I can mention it here.

I wrote it first without any networking features to quickly prototype the reset procedure. After that, I added MQTT support, because that's how I communicate with the sonoff devices. Usually with TASMOTA, but the current implementation is all I use from it anyway.

## Flashing

Clone this repo and flash it to your S20 / S26 with your favourite programmer. E.g. I use VS Code with the Arduino extension.
If you also use the same as i do, it is allready configured for you. If not use the following settings:

```
    CpuFrequency    =   80
    FlashFreq       =   40
    FlashMode       =   dout
    UploadSpeed     =   115200
    FlashSize       =   512K64
    ResetMethod     =   ck
    Debug           =   Disabled
    DebugLevel      =   None____
```

## Usage without MQTT

Plug it into your wall outlet, long-press the button on the front, and it will start the reset procedure.

## Usage with MQTT enabled

You have to uncomment the first line in edit config.h to enable MQTT support. Edit also your wireless configuration and if you use no MQTT security change  

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

You don't have to set all properties. All properties that are not set will be set to the default value described below. That means, if you are happy with the default values, you can also send an empty payload.

#### Working payload examples:

Have a look at the [wiki](https://github.com/Dirnei/ikea_bulb_reset/wiki/Known-Reset-Configurations) for working payloads. If you tried it with bulbs of other vendors please let me know so i can add it to the list. 

#### lockDuration

Time in milliseconds for how long the relay state is locked for changes after the reset procedure finished. Default: __20000__

#### cycles

The number of ON-OFF cycles that will be performed during the reset procedure. Default: __6__

#### delayOn

Time in milliseconds for the delay after the relay is turned *ON* in the reset procedure. Default: __500__

#### delayOff

Time in milliseconds for the delay after the relay is turned *OFF* in the reset procedure. Default: __500__

### MQTT Feedback

When the relay toggles you will get a status message to your __mqttTopic__ with the suffix __/status__.
During the reset procedure, you will not get any status updates. Only when it finishes, you will get a message with status __LOCKED__. When the lock is removed, you will be notified again with the current relay state.

## Tested devices

The following devices where successfully tested:

- Sonoff S20
- Sonoff S26
