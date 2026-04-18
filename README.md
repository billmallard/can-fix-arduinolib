# MakerPlane CAN-FIX Arduino Library

**Status:** Open Source — Experimental Amateur-Built Category  
**License:** See COPYING  
**Protocol:** CAN-FIX (CAN bus implementation of the Flight Information eXchange protocol)

---

## What This Is

This is an Arduino library that enables Arduino-based devices to participate in a **CAN-FIX avionics network**. CAN-FIX is an open-source protocol designed specifically for Experimental Amateur-Built (E-AB) aircraft to exchange flight data between avionics nodes in a vendor-neutral way.

With this library, Arduino hardware can:
- Read and write named flight data parameters (airspeed, altitude, heading, engine data, etc.) on the CAN bus
- Act as a sensor node, sending data from connected hardware into the avionics network
- Act as an actuator or display node, receiving and responding to parameter updates
- Interoperate with other CAN-FIX devices including FIX-Gateway, pyEfis, and custom electronics

## Protocol Background

**FIX** (Flight Information eXchange) is a family of open, Creative Commons-licensed specifications for aircraft avionics communication. **CAN-FIX** is the CAN bus implementation of FIX. It is designed to:

- Allow any builder to construct devices that communicate with other FIX-compatible hardware without licensing fees
- Provide a standard parameter namespace covering the full range of aircraft state (position, attitude, airspeed, engine data, control surfaces, systems status)
- Enable redundancy through multiple nodes publishing the same parameter type on separate identifiers

See the [canfix-spec](../canfix-spec) repository for the full protocol specification.

## Repository Contents

| File | Description |
|---|---|
| `canfix.h` | Library header — parameter definitions, message structures, node API |
| `canfix.cpp` | Library implementation |
| `examples/` | Example Arduino sketches demonstrating common use patterns |
| `keywords.txt` | Arduino IDE syntax highlighting keywords |

## Installation

Follow standard Arduino library installation:
1. Download or clone this repository
2. Copy the folder into your Arduino `libraries/` directory
3. Restart the Arduino IDE
4. Access the library under **Sketch → Include Library → CAN-FIX**

For detailed installation guidance see: http://arduino.cc/en/Guide/Libraries

## Hardware Requirements

- Any Arduino board with a CAN controller, or an Arduino paired with a CAN transceiver module (MCP2515-based shields are common in the MakerPlane ecosystem)
- CAN bus wiring per the CAN-FIX physical layer spec (120Ω termination at each end)

## Role in the MakerPlane / MAOS Ecosystem

This library is the **embedded hardware entry point** for the avionics network. Arduino nodes built with this library can:

- Feed sensor data (pressure altitude, OAT, EGT, RPM, voltage, etc.) into [FIX-Gateway](../fix-gateway)
- Receive control outputs from [FIX-Gateway](../fix-gateway) to drive physical annunciators, relays, or trim motors
- Bridge custom MAOS subsystem sensors to the avionics display stack without custom protocol work

## Important Disclaimer

> **This library is experimental and is not suited for primary or backup flight/engine instrumentation or navigation. Use at your own risk.**  
> For Experimental Amateur-Built aircraft use only. Not FAA-approved avionics software.
