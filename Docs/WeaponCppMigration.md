# Single-player weapon controls

`ARPlayer` now binds `/Game/Input/IA_ToggleWeapon` to arm/disarm and
`/Game/Input/IA_SwitchWeapon` to cycle valid inventory weapon items in the Weapon
slot. These are the only two weapon input properties: Toggle Weapon Action and
Switch Weapon Action. Existing mappings in `IMC_Player` determine the keys.
The `ability.weapon.equip` and
`ability.weapon.disarm` input tags are also handled directly in C++ rather than
forwarded to GAS. Actions already bound as native weapon controls are excluded
from the generic ability bindings.

## Blueprint migration required

After closing the editor and rebuilding, open `BP_Player`:

1. Remove its old weapon toggle event graph and any IA_SwitchWeapon
   event graph. Keep the actions mapped in the input mapping context.
2. Remove the weapon-control calls that activate GA_WeaponEquip or
   GA_WeaponDisarm. The C++ control path calls the weapon directly. These assets
   can remain for other uses, but should not also run for these inputs.
3. Check existing weapon item Blueprint defaults: Slot must be Weapon and
   Weapon Class must point to the desired RWeapon Blueprint. Existing assets
   may retain an older serialized slot despite the corrected native default.
4. Check each weapon's Active Weapon Socket, Inactive Weapon Socket, Equip Anim,
   Equip Anim, Disarm Anim, Equip SFX and Disarm SFX. Toggle plays the arm/disarm
   montages. Arming attaches to the hand immediately; disarming moves to the
   inactive socket when its montage finishes. FinishHolster may be called by
   an animation notify at the sheath frame. Switching attaches immediately,
   preserves armed state, and plays no montages. It cancels a pending toggle
   montage and holster timer. Blueprint overrides of ShouldUseOnAdd must call the parent
   or enforce the same empty-slot check to use the new pickup behavior.

Disarm uses Disarm Anim, or Equip Anim's Disarm section if Disarm Anim is unset.
Inventory pickup and weapon replacement use immediate equip/unequip methods;
only the toggle path uses animated arm/disarm methods.

## Behavior

- A valid weapon pickup auto-equips when there is no weapon in its slot or
  already attached. Later pickups stay in the inventory.
- Drawing an empty slot selects the first valid inventory weapon. Drawing an
  occupied slot reuses its actor. Holstering retains that actor and item.
- Switching cycles inventory weapon items, safely removes the old actor, and
  preserves the armed/holstered state. Zero weapons and one selected weapon
  are safe no-ops.
- Drawing and switching resolve inventory through RInventoryBFL, including
  the inventory on BP_PState rather than requiring one on BP_Player.
- A native UMG panel is created for the local player. It displays the item name,
  icon and armed state, with toggle and switch input hint images. The panel
  uses equipment change notifications and removes bindings when destroyed.
  The hint images are visual references; weapon actions use the input bindings.

## Designing the panel in Blueprint

Create a Widget Blueprint with REquippedWeaponWidget as its parent, then add
your own layout in Designer. Check Is Variable on these controls and name them
exactly as follows to bind them to the C++ behavior:

| Name | Designer type |
| --- | --- |
| WeaponName | Text Block |
| WeaponState | Text Block |
| WeaponIcon | Image |
| ToggleButton | Image |
| SwitchButton | Image |

All controls are optional and exposed as BlueprintReadWrite. Equipment is also
read/write; its Blueprint setter updates the change subscription. RefreshWeapon
is callable from Blueprint. ToggleButton and SwitchButton retain their names but
are Images for input hints, with no click handling. Set their brushes in Designer.
Native refresh updates text, icon texture and icon
visibility, while icon size and styling come from your Designer layout.

Assign your Widget Blueprint to BP_Player's Weapon > UI > Equipped Weapon
Widget Class property. The player creates this class at the existing bottom-right
position. The C++ layout remains the fallback when there is no Designer root.

## Play-in-editor checks

1. Start without weapons: draw, disarm and switch should be safe.
2. Pick up one valid weapon: exactly one actor should attach to its active socket.
3. Draw repeatedly: actor count stays one. Holster/draw swaps sockets and state.
4. Pick up another weapon: the current slot remains unchanged.
5. Switch while armed, then while holstered: name/icon updates and state persists.
6. Unequip/drop the selected item: its actor disappears and the panel clears.
7. Use ToggleWeapon and SwitchWeapon input. Confirm no duplicate Blueprint
   input callbacks or equip/disarm abilities run.

The native panel builds its controls using Unreal's UWidgetTree API:
https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/UWidgetTree
