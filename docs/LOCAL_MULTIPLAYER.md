# LOCAL_MULTIPLAYER.md

## Philosophy

Local multiplayer should be an extension of the offline simulation, not a separate game implementation.

## Host model

One device owns authoritative round state and physics outcomes. Clients send player inputs and receive relevant replicated state.

## Discovery

Start with LAN discovery because it is easier to debug and generally more predictable than trying to make every legacy Bluetooth stack behave nicely.

Bluetooth is a later transport experiment.

## Bandwidth strategy

Do not synchronize every rendered frame.

Synchronize:

- inputs where needed
- player transforms
- important physics objects
- disaster state
- round state
- deaths/spawns
- relevant item use

Use interpolation for remote visuals.

## Offline mode

Use an in-process loopback transport or direct interfaces so networking code can remain present without requiring sockets.
