# Embedded_fitness_challenge

The goal of the project is to detect situps, pushups, squats and jumping jacks, program for fixed workout routine (say 'x' repetition of each exercise).

## Hardware

**STM32F407** (Arm) microcontroller, with on-board **LIS3DSH** accelerometer is used. The readings from accelerometer is used to differentiate different exercises, and LEDs on board are used to indicate a particular type of exercise. The **USER BUTTON** is used to indicate beginning of demo, as well as beginning of actual exercise.

## Data Collection

The 4 types of exercises were performed (5 repetitions of each exercise), 6 types of data were recorded.
- acceleration in x-axis
- acceleration in y-axis
- acceleration in z-axis
- orientation wrt to x-axis
- orientation wrt to y-axis
- orientation wrt to z-axis

The data collected for 5 repetitions for each type of exercise, their plots can be found in Data_collection folder. Only 4 types of data were found to be useful. Data related to x-axis were neglected.

## Mode of operation

There are two modes of operation
1. Detection mode
2. Counting mode

In detection mode, the user has to press button once indicating that the “Demo” mode has started. The user has to do the exercise once, and wait for microcontroller to indicate what exercise it is through LEDs. For squat – orange LED, jumping jack – green LED, situp – red LED, push up – blue LEDs are indicated.

In counting mode, once detection is indicated, user has to press button again to indicate that actual exercise is getting started. The progress of the exercise is indicated by glowing of binary encoded LEDs. The completion of exercise is indicated by turning off all LEDs. This project has been coded 5 repetitions of each exercise.

## Code working

First in detection mode, once button is pressed, first 20 values of raw Y, raw Z acceleration values of demo exercise are recorded. Later mean and standard deviation of these values are used to detect further type of exercises.

First, the entire problem is divided into two sub-problems, **lying exercise** and **standing exercise** type. This is done based on raw Y acceleration mean values. If it’s lying type exercise, raw Z acceleration mean values and angle wrt Z axis is used to differentiate between situp and pushup. If it’s standing type exercise, raw Y acceleration standard deviation values are used to differentiate between squat and jumping jack.

## Installation/Use

This project was created in Visual Studio Code. While creating a new project, choose board name as **ST STM32F4DISCOVERY**, framework as **Mbed**. Only the files in **src** folder is needed.

## Protocols, Libraries and Peripherals

LIS3DSH library for Mbed framework is used. To receive data from the accelerometer, **SPI** serial communication protocol is used. **Timers** are used in Counting mode, the timer limit has to be changed accordingly if you want to increase the repetitions in each exercise.
