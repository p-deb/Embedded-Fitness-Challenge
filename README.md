# Embedded Fitness Challenge
New York University - Tandon School of Engineering

ECE-5373: Real-Time Embedded Systems -- Fall 2020

Progga Deb

## Overview
The COVID pandemic has forced almost all gyms to close until further notice. Because of this, many of us have become sedentary and are suffering from lack of sufficient exercise causing fatigue and weight-gain, creating the need for new exercising options. The goal of this challenge is to use the data collected from a single accelerometer to record body movements and identify one of 4 exercises:
* Sit-Ups
* Push-Ups
* Jumping Jacks
* Squats

In my implementation, I used a STM32F4 Discovery board to detect the motion for each exercise.

## Requirements
* One microcontroller  with one accelerometer/MPU
* Visual Studio PlatformIO

## User Manual
To make the best use of this device, please follow the instructions below:

1) Install VSCode PlatformIO programming environment.

2) Download the source code from this git repository and upload the code to your microcontroller.

4) Remove the microcontroller from your computer and orient it vertically on your arm, facing forward. An arm band or any other tools can be used to tightly strap the device in place.

2) Connect the device to a power source (e.g., portable charger) using the micro USB port.

3) Complete 5 repetitions of push-ups, sit-ups, squats, and jumping jacks.

4) While completing an exercise, press the user button (blue button) to see how many reps you have remaining for the current exercise. 
    a) Green LED = Blinks the # of remaining reps
    b) Red LED = All reps completed
    c) X LED = LED indicator for current exercise
    
5) After each exercise is complete, there will be a 10 second rest period; the red LED will blink 10 times indicating for the user to change into the starting position for the next exercise.

6) If the red LED blinks each time the button is pressed for an exercise, you have completed your workout!
