# 1.1.6

## Fixes

- Fixed path not being drawn for anybody on dedicated servers
- Fixed "Show path" button on a "spline node edit UI" (the arrow thingy in the world) not being aligned with the actual
  state of the path
- Made the path completely client-specific. Players won't affect each others

## Technical

- Removed unused pathMeshPool
- Removed all HasAuthority() checks
- Refactored Button blueprints a bit