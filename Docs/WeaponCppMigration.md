# Single-player weapon controls

`ARPlayer` now binds `/Game/Input/IA_DrawWeapon` to draw/holster and
`/Game/Input/IA_SwitchWeapon` to cycle valid inventory weapon items in the Weapon
slot. These are the only two weapon input properties: Toggle Weapon Action and
Switch Weapon Action. Existing mappings in `IMC_Player` determine the keys.
The `ability.weapon.equip` and
`ability.weapon.disarm` input tags are also handled directly in C++ rather than
forwarded to GAS. Actions already bound as native weapon controls are excluded
from the generic ability bindings.

## Blueprint migration required

After closing the editor and rebuilding, open `BP_Player`:

1. Remove its old IA_DrawWeapon equip/disarm event graph and any IA_SwitchWeapon
   event graph. Keep the actions mapped in the input mapping context.
2. Remove the weapon-control calls that activate GA_WeaponEquip or
   GA_WeaponDisarm. The C++ control path calls the weapon directly. These assets
   can remain for other uses, but should not also run for these inputs.
3. Check existing weapon item Blueprint defaults: Slot must be Weapon and
   Weapon Class must point to the desired RWeapon Blueprint. Existing assets
   may retain an older serialized slot despite the corrected native default.
4. Check each weapon's Active Weapon Socket, Inactive Weapon Socket, Equip Anim,
   Disarm Anim, Equip SFX and Disarm SFX. Drawing attaches to the hand immediately.
   Holstering keeps the weapon in the hand until the disarm montage finishes.
   An animation notify can call FinishHolster on the current weapon at the exact
   sheath frame; this cancels the fallback timer. Drawing again cancels pending
   holster attachment. Blueprint overrides of ShouldUseOnAdd must call the parent
   or enforce the same empty-slot check to use the new pickup behavior.

Disarm uses Disarm Anim and its Disarm section when present; otherwise it plays
the separate montage from its beginning (for example AM_SheatheKatana's Default
section). If Disarm Anim is unassigned, Equip Anim's Disarm section is used.
Both draw and disarm play through the player's mesh AnimInstance.

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
  icon and armed state, with Draw / Holster and Next weapon buttons. The panel
  uses equipment change notifications and removes bindings when destroyed.
  Buttons use the game's existing cursor/input mode; keyboard actions work
  without showing a cursor.

## Play-in-editor checks

1. Start without weapons: draw, disarm and switch should be safe.
2. Pick up one valid weapon: exactly one actor should attach to its active socket.
3. Draw repeatedly: actor count stays one. Holster/draw swaps sockets and state.
4. Pick up another weapon: the current slot remains unchanged.
5. Switch while armed, then while holstered: name/icon updates and state persists.
6. Unequip/drop the selected item: its actor disappears and the panel clears.
7. Use the UI buttons with the cursor enabled. Confirm no duplicate Blueprint
   input callbacks or equip/disarm abilities run.

The native panel builds its controls using Unreal's UWidgetTree API:
https://dev.epicgames.com/documentation/en-us/unreal-engine/API/Runtime/UMG/UWidgetTree
