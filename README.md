# 📟 STM32 RT-Thread Data Acquisition System

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Hardware](https://img.shields.io/badge/Hardware-STM32-orange.svg)
![OS](https://img.shields.io/badge/OS-RT--Thread%20RTOS-success.svg)

## 📌 Overview
This repository contains the firmware for a robust embedded data acquisition system built on the **STM32** microcontroller architecture. The system leverages the **RT-Thread Real-Time Operating System (RTOS)** to manage concurrent tasks efficiently and interfaces with multiple **ADS1115 (16-bit Precision ADCs)** via the I2C protocol. 

## 🚀 Key Features
* **Real-Time Multithreading:** Utilizes RT-Thread RTOS for deterministic task scheduling, ensuring precise timing for sensor polling and data processing.
* **Multi-Device I2C Communication:** Configured the STM32 Hardware Abstraction Layer (HAL) to communicate with multiple ADS1115 devices on the same I2C bus using distinct 8-bit shifted slave addresses.
* **Hardware Interrupts:** Implemented External Interrupts (EXTI) via GPIO pins for event-driven data sampling, minimizing CPU polling overhead.
* **Robust Error Handling:** Integrated hardware-level assertion and fallback loops for safe debugging and system stability.

## 🛠️ Tech Stack & Hardware
* **Microcontroller:** STMicroelectronics STM32 Family.
* **Sensors/Peripherals:** ADS1115 (16-bit Analog-to-Digital Converter).
* **Framework/OS:** STM32 HAL (Hardware Abstraction Layer), RT-Thread RTOS.
* **Protocols:** I2C (Inter-Integrated Circuit).
