# Round System

The first playable gameplay layer uses a small deterministic round state machine shared by offline play and the future local multiplayer host.

## States

```text
Intermission -> Playing -> Results -> Intermission
```

The initial prototype uses:

- 10 seconds of intermission
- 60 seconds of active survival
- 5 seconds of results

These are configuration defaults, not permanent gameplay values.

The round system deliberately does not know about rendering, networking, UI, or a specific disaster. Those systems observe the state and attach their own behavior later. This keeps the offline simulation path compatible with the planned host-authoritative LAN implementation.

## Current prototype behavior

At the end of intermission a round number is incremented and the round enters `Playing`. When the active timer expires, the prototype marks the local player as survived and enters `Results`. A later gameplay pass will replace that unconditional survival result with actual player health/death tracking and disaster completion.
