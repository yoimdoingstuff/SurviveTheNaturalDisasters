# UPDATE_MODEL.md

The base release is intentionally narrow and stable.

Required base features before the update track:

- Main menu
- Offline gameplay
- Local lobby creation
- Join local lobby
- Server/lobby settings
- Full base round loop
- Local multiplayer
- iOS 8 build
- Legacy performance pass

After the base release is locked, updates can add modular systems.

## Cosmetics

Cosmetics should be data-driven. A cosmetic definition should specify its icon, appearance hooks, unlock requirements, and compatible player slots.

## Gear

Gear should be implemented as an extension system with a clear activation/effect lifecycle. Example future items include a green balloon-style item, red apple-style consumable, and compass-style utility item.

Gear must work offline and replicate correctly in local multiplayer.

## Compatibility

Updates must not raise the minimum target hardware unless explicitly declared. Prefer optional content/quality reductions for legacy hardware.
