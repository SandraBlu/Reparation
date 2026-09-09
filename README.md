# Reparation

An Action RPG built in Unreal Engine 5.8.

## What this repository is

A **recovery repository**, not a complete project. It holds everything needed to
rebuild a working project: C++ source, configuration, and gameplay logic
assets. Bulk art is deliberately excluded and lives in local backups.

Roughly 120 MB here, versus ~18 GB for the full project.

### Included

| | |
|---|---|
| `Source/` | C++ for both modules: `Reparation` (runtime) and `ReparationEditor` (editor-only) |
| `Config/` | `DefaultEngine.ini`, `DefaultGame.ini`, `DefaultInput.ini`, `DefaultGameplayTags.ini`, `DefaultEditor.ini` |
| `Reparation.uproject` | Module and plugin manifest |
| `SourceFiles/` | Raw swim locomotion FBX exports |
| Gameplay Blueprints | Every `BP_`, `ABP_`, `WBP_`, `BPI_`, `AIC_`, behaviour tree, `GA_`, `GE_`, `DA_`, `DT_` asset, wherever it lives |
| `Content/GAS`, `UI`, `Input`, `Inventory`, `Dialogue`, `Map` | Small authored data folders, kept whole |

### Not included

Static and skeletal meshes, textures, materials, animation sequences, Niagara
assets, landscape data, World Partition external actors, and all marketplace
art packs. See `.gitignore`.

**Consequence:** cloning this and opening it in the editor gives you a project
that compiles and has all its gameplay logic, but Blueprints will show missing
mesh, material and animation references until the art is restored.

## Rebuilding from scratch

1. Install **Unreal Engine 5.8**.
2. Clone this repository.
3. Reinstall the plugins below into `Plugins/`.
4. Restore `Content/` art from backup, or reacquire the packs below.
5. Right-click `Reparation.uproject` → *Generate Visual Studio project files*.
6. Build the `ReparationEditor` target, or open the `.uproject` and let it compile.

### Required plugins

Not committed here; licensed per seat and reinstalled from Fab.

- **Combo Graph** — `Plugins/ComboGraph`
- **Narrative Pro / Narrative Tales** — `Plugins/NarrativeTales`

Engine plugins enabled in the `.uproject` (GameplayAbilities, CommonUI,
MotionWarping, Water, Buoyancy, NiagaraFluids, MeshPartition, ModelingTools,
ModelContextProtocol) ship with the engine and need no action.

### Art packs referenced by the project

- ResourcePack (GraniteCliffsKit, RagePack, Hospitality)
- BuildingMeshes (Hospitality, Medieval Inn Modular)
- Advanced Companion AI
- Ultra Dynamic Sky
- Aquatic Surface
- Ultimate River Tool
- Open World Locomotion
- HQUI Buttons V2
- Quixel Megascans / Fab assets
- Landscape/Procedural (Oak) foliage

## Project layout

`Source/Reparation` is organised as `Actors`, `AI`, `Anim`, `Characters`,
`Components`, `Framework`, `GAS`, `Input`, `Interfaces`, `Items`, `Navigation`,
`RTypes` and `UI`. `Source/ReparationEditor` holds editor-only tooling that must
not link into a packaged build.
