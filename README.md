# RF Asset Finder

A small 433 MHz asset-presence system built around explicit packet framing, CRC-16 validation, sequence numbers, acknowledgements, retries, and timeouts.

The repository has two layers:

- `src/` is a portable C++17 protocol and base-station tracker that can be tested on any computer.
- `firmware/` contains Arduino sketches for a tag and a base station using the RadioHead `RH_ASK` driver.

## Try the lab simulation

```sh
make demo
make test
```

The demo feeds three tags into the tracker, advances the clock, and reports the tag that crossed its timeout boundary.

## Packet format

| Field | Bytes | Notes |
| --- | ---: | --- |
| Magic | 2 | `RF` |
| Version | 1 | Protocol version 1 |
| Type | 1 | Beacon or acknowledgement |
| Device ID | 2 | Big-endian tag identifier |
| Sequence | 2 | Used to reject stale acknowledgements |
| Payload length | 1 | Maximum 16 bytes |
| Payload | 0–16 | Battery voltage or future telemetry |
| CRC-16/CCITT | 2 | Covers every preceding byte |

## Hardware notes

The example sketches assume one transmitter and one receiver on each node so the base can acknowledge a beacon. Pin assignments, antenna length, and logic-voltage compatibility must be checked for the specific 433 MHz modules and microcontroller. Cheap ASK modules are noisy; this protocol detects corruption but does not provide encryption or reliable ranging.

## Dependencies

- Computer simulation: a C++17 compiler and `make`
- Arduino sketches: Arduino IDE or PlatformIO plus the RadioHead library

MIT licensed.
