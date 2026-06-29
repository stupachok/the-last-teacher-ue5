# The Last Teacher — Tower Defence Survival (UE5)

> Bachelor's thesis project | Unreal Engine 5.7 | C++ + Blueprint

## 🎮 Demo
*Coming soon — gameplay video in progress*

## Overview
Post-apocalyptic tower defence survival game. The player takes the role 
of the last teacher who crafts robot defenders, mines resources, survives 
monster waves, and must build a rocket to escape.

## Key Systems
- **AI**: Behavior Tree + MLP neural network (C++, topology 9→12→4)
- **Bot behavior**: Priority logic — Follow / Fight / Repair / GoHome
- **Crafting**: Robot crafting menu → slot assignment → bot spawn
- **Resources**: Ionite / Plasmite / Quantite with depletion/respawn
- **Teleportation**: Player-controlled bot follow/return system via slots
- **Landscape**: Procedurally generated in Gaea, imported via heightmap

## Tech Stack
- Unreal Engine 5.7
- C++ + Blueprint Visual Scripting
- Gaea (terrain generation)
- Visual Studio 2022
