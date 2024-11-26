# Notes

* Rework IUI + UI system to allow for groups of slots as a fixed sized array of dimension `(N,M)`
  where `N` and `M` are used along with slot dimenions `(W,H)` and spacing between slots

## Slot Groups

* Rework IUI + UI system to allow for groups of slots.
* Compile time data:
    * Dimensions of group on screen in pixels, i.e. how many slots in X and Y
      directions for rectangle group
    * Dimensions of the slots themselves in pixels
    * Spacing between slots on the X and Y axis in pixels
    * Origin on-screen in pixels
* A `SlotGroup` is essentially just an array of `Slot` defined by the compile-time
  group dimensions
* The `Slot` struct then has everything except the data or ref union and index
  removed from it

### Definitions

```c
typedef struct Slot {
    union {
        // Actual data
        IItem* item;
        // Reference to slot elsewhere, i.e. inventory
        // hotbar slots mapping to actual hotbar slots
        struct Slot* ref;
    } data;
    // UI global slot index starting from top to bottom, left to right on screen
    u8 index;
} Slot;

typedef Slot SlotGroup;

#define slotGroupDim(name, dim) name##_SLOT_GROUP_DIMENSIONS_##dim
#define slotGroupSlotDim(name, dim) name##_SLOT_GROUP_SLOT_DIMENSIONS_##dim
#define slotGroupSlotSpacing(name, dim) name##_SLOT_GROUP_SLOT_SPACING_##dim
#define slotGroupOrigin(name, dim) name##_SLOT_GROUP_ORIGIN_##dim

```

### Usage

```c
// Armour slots
#define INVENTORY_ARMOUR_SLOT_GROUP_DIMENSIONS_X 1
#define INVENTORY_ARMOUR_SLOT_GROUP_DIMENSIONS_Y 4
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_SPACING_X 0
#define INVENTORY_ARMOUR_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_ARMOUR_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_ARMOUR_SLOT_GROUP_ORIGIN_Y 45

// Crafting slots
#define INVENTORY_CRAFTING_SLOT_GROUP_DIMENSIONS_X 2
#define INVENTORY_CRAFTING_SLOT_GROUP_DIMENSIONS_Y 2
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_CRAFTING_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_CRAFTING_SLOT_GROUP_ORIGIN_X 160
#define INVENTORY_CRAFTING_SLOT_GROUP_ORIGIN_Y 63

// Crafting results slots
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_X 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_DIMENSIONS_Y 1
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_RESULT_SLOT_DIMENSIONS_X 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_RESULT_SLOT_DIMENSIONS_Y 16
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_RESULT_SLOT_SPACING_X 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_RESULT_SLOT_SPACING_Y 0
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_X 216
#define INVENTORY_CRAFTING_RESULT_SLOT_GROUP_ORIGIN_Y 73

// Main slots
#define INVENTORY_MAIN_SLOT_GROUP_DIMENSIONS_X 9
#define INVENTORY_MAIN_SLOT_GROUP_DIMENSIONS_Y 3
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_MAIN_SLOT_GROUP_SLOT_SPACING_Y 2
#define INVENTORY_MAIN_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_MAIN_SLOT_GROUP_ORIGIN_Y 121

// Hotbar slots
#define INVENTORY_HOTBAR_SLOT_GROUP_DIMENSIONS_X 9
#define INVENTORY_HOTBAR_SLOT_GROUP_DIMENSIONS_Y 1
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_X 16
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_DIMENSIONS_Y 16
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_SPACING_X 2
#define INVENTORY_HOTBAR_SLOT_GROUP_SLOT_SPACING_Y 0
#define INVENTORY_HOTBAR_SLOT_GROUP_ORIGIN_X 80
#define INVENTORY_HOTBAR_SLOT_GROUP_ORIGIN_Y 179

DEFN_UI(Inventory,
    SlotGroup armour_slots[slotGroupDim(INVENTORY_ARMOUR, Y)][slotGroupDim(INVENTORY_ARMOUR, X)];
    SlotGroup crafting_slots[slotGroupDim(INVENTORY_CRAFTING, Y)][slotGroupDim(INVENTORY_CRAFTING, X)];
    SlotGroup crafting_result_slots[slotGroupDim(INVENTORY_CRAFTING_RESULT, X)];
    SlotGroup main_slots[slotGroupDim(INVENTORY_MAIN, Y)][slotGroupDim(INVENTORY_MAIN, X)];
    SlotGroup hotbar_slots[slotGroupDim(INVENTORY_HOTBAR, Y)][slotGroupSlotDim(INVENTORY_HOTBAR, X)];
    Hotbar* hotbar;
    Timestamp debounce;
);
```
