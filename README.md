# Tick-Talk
## Presentation time-management app for the Pebble smartwatch

This application for the Pebble smartwatch allows you to prepare perfectly for precisely timed presentations. The application lets you record and play back timing intervals for each slide of your presentation and supports you with configurable visual and haptic feedback during play back.

## Usage

The application has two modes: One for training and one for presenting. 

### Buttons

#### Back

  - Single click: Open / Close settings
  - Double click: Switch between training / presentation mode
  - Triple click: Save all settings and leave the app

### Select

  - Single click: Start / Pause time
  - Long click: Reset time

### Up / Down

  - Single click: Navigate through slides and menus

## Training

Double click the back button to enter training mode. In this mode, you can perform timing measurements for each slide of your presentation. The idea is that you start the measurement with the select button and - while you rehearse your presentation - click the up button every time you advance a slide. This automatically saves the timing for each slide.

You can also edit the timing of a single slide in training mode by simply navigating to the desired slide and starting / pausing the measurement.

Double click the back button to commit your changes and switch back into presentation mode.

## Presentation

In presentation mode, the measurements from your training are played back. Here, you can start / pause and reset the timer for the current slide with the select button, as well as navigate between slides with the up and down buttons.

The presentation mode shows you a progress bar for each slide, which gradually fills up according to the timing you have trained.

### Settings

You can reach the settings menu by double clicking the back button.

## Display style

The application allows for two different display styles for the time during presentation mode:

  - Timer: Shows the elapsed and total time for the current slide in the format ```m:S[elapsed] / m:S[total]```
  - Countdown: Shows the time remaining for the current slide in the format ```m:S.ms[remaining]```

## Warning buzzer

You can set an intermediate haptic feedback to be activated at a certain point in time before the end of a slide. This feedback only emits a short vibration pulse and can either be configured to occur [0, 10, 20, ..., 60] seconds before the end of a slide or turned off entirely.
