# RF Asset Finder

[![CI](https://github.com/mjumair7/rf-asset-finder/actions/workflows/ci.yml/badge.svg)](https://github.com/mjumair7/rf-asset-finder/actions/workflows/ci.yml)

This is a small 433 MHz experiment, not a finished asset tracker. I wanted to see what has to exist around a radio send call before a base station can make a useful “present or missing” decision.

That turned into a basic packet format with a CRC, sequence numbers, acknowledgements, retries, and a timeout.

The repository has two layers:

- `src/` is a portable C++17 protocol and base-station tracker that can be tested on any computer.
- `firmware/` contains Arduino sketches for a tag and a base station using the RadioHead `RH_ASK` driver.

```mermaid
sequenceDiagram
    participant T as Tag node
    participant B as Base station
    T->>B: Beacon(device, sequence, battery, CRC)
    alt frame is valid
        B-->>T: ACK(device, same sequence, CRC)
    else corrupt or unexpected
        B--xT: no acknowledgement
    end
    Note over T: retry up to 3 times
    Note over B: mark tag missing after timeout
```

## Try the desktop simulation

```sh
make demo
make test
```

The demo feeds three tags into the tracker, advances the clock, and reports the tag that crossed its timeout boundary. The tests cover encoding, CRC rejection, packet-type validation, duplicate sequence rejection, and timeout state.

## Packet format

| Field | Bytes | Notes |
| --- | ---: | --- |
| Magic | 2 | `RF` |
| Version | 1 | Protocol version 1 |
| Type | 1 | Beacon or acknowledgement |
| Device ID | 2 | Big-endian tag identifier |
| Sequence | 2 | Used to reject stale data and ACKs |
| Payload length | 1 | Maximum 16 bytes |
| Payload | 0–16 | Battery voltage or future telemetry |
| CRC-16/CCITT | 2 | Covers every preceding byte |

## The detail that matters

A packet can be perfectly valid and still be the wrong acknowledgement. The tag therefore checks the device ID, sequence number, packet type, length, and CRC before it accepts an ACK.

The desktop side also rejects duplicate beacons and non-beacon packets. Those checks are small, but without them the demo can look reliable while tracking the wrong message.

## Hardware notes

The example sketches assume one transmitter and one receiver on each node so the base can acknowledge a beacon. Pin assignments, antenna length, and logic-voltage compatibility must be checked for the specific modules and microcontroller.

Cheap ASK modules are noisy. The protocol can detect corruption, but it does not provide encryption, collision avoidance, or reliable ranging. The Arduino sketches still need to be tested with the exact radios, antennas, and power supply used in a physical build. Sequence-number rollover and multi-tag contention are known next steps.

## Dependencies

- Desktop simulation: a C++17 compiler and `make`
- Arduino sketches: Arduino IDE or PlatformIO plus RadioHead

MIT licensed.
