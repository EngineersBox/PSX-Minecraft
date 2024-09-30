/*#include "core/engine.h"*/
/*#include "core/app_logic.h"*/
/*#include "game/minecraft.h"*/
/**/
/*AppLogic app_logic;*/
/*Minecraft* minecraft;*/
/**/
/*int main() {*/
/*    app_logic = DYN_LIT(Minecraft, AppLogic, {});*/
/*    minecraft = (Minecraft*) &app_logic;*/
/*    Engine engine = (Engine) {*/
/*        .app_logic =  &app_logic,*/
/*        .target_fps = 60,*/
/*        .target_tps = 20*/
/*    };*/
/*    engineInit(&engine, NULL);*/
/*    engineRun(&engine);*/
/*    return 0;*/
/*}*/

#include <lzp/lzp.h>
#include <psxapi.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "hardware/card.h"

typedef uint8_t u8;
typedef uint32_t u32;

typedef enum DataType {
    DATA_TYPE_STATELESS = 0,
    DATA_TYPE_CHEST = 1,
    DATA_TYPE_FURNACE = 2
} DataType;

// Size: 1 + 1 = 2
typedef struct SourceData {
    u8 type: 2;
    u8 metadata_id: 6;
    u8 id;
} SourceData;

typedef struct ItemData {
    u8 id;
    u8 metadata_id;
    u8 stack_size;
} ItemData;

#define ITEM(i,m,s) (ItemData){.metadata_id=(m),.id=(i),.stack_size=(s)}

typedef struct ChestData {
    SourceData block;
    // Slots
    ItemData storage[9 * 3];
} ChestData;

typedef struct FurnaceData {
    SourceData block;
    // 0: input 1: fuel 2: output
    ItemData storage[3];
} FurnaceData;

int main() {
    printf("SourceData: %d\n", sizeof(SourceData));
    printf("ItemData: %d\n", sizeof(ItemData));
    printf("ChestData: %d\n", sizeof(ChestData));
    printf("FurnaceData: %d\n", sizeof(FurnaceData));
    size_t buf_size = 1256;
    u8* input_buffer = (u8*) calloc(sizeof(u8), buf_size);
    size_t end = 0;
    // Stone layer
    for (u8 y = 0; y < 4; y++) {
        for (u8 x = 0; x < 8; x++) {
            for (u8 z = 0; z < 8; z++) {
                const SourceData data = (SourceData) {
                    .type = DATA_TYPE_STATELESS,
                    .id = 1,
                    .metadata_id = rand() % 3
                };
                if (end + sizeof(SourceData) >= buf_size) {
                    buf_size += sizeof(SourceData);
                    realloc(input_buffer, buf_size);
                }
                memcpy(&input_buffer[end], &data, sizeof(SourceData));
                end += sizeof(SourceData);
            }
        }
    }
    // Dirt - 4
    for (u8 x = 0; x < 8; x++) {
        for (u8 z = 0; z < 8; z++) {
            const SourceData data = (SourceData) {
                .type = DATA_TYPE_STATELESS,
                .id = x == 4 && z == 4 ? 0 : 2,
                .metadata_id = 0
            };
            if (end + sizeof(SourceData) >= buf_size) {
                buf_size += sizeof(SourceData);
                realloc(input_buffer, buf_size);
            }
            memcpy(&input_buffer[end], &data, sizeof(SourceData));
            end += sizeof(SourceData);
        }
    }
    // Grass - 4
    const SourceData data = (SourceData) {
        .type = DATA_TYPE_STATELESS,
        .id = 3,
        .metadata_id = 0
    };
    if (end + sizeof(SourceData) >= buf_size) {
        buf_size += sizeof(SourceData);
        realloc(input_buffer, buf_size);
    }
    memcpy(&input_buffer[end], &data, sizeof(SourceData));
    end += sizeof(SourceData);
    // Dirt - 5
    for (u8 x = 0; x < 8; x++) {
        for (u8 z = 0; z < 8; z++) {
            const SourceData data = (SourceData) {
                .type = DATA_TYPE_STATELESS,
                .id = x >= 3 && x <= 5 && z >= 3 && z <= 5 ? 0 : 2,
                .metadata_id = 0
            };
            if (end + sizeof(SourceData) >= buf_size) {
                buf_size += sizeof(SourceData);
                realloc(input_buffer, buf_size);
            }
            memcpy(&input_buffer[end], &data, sizeof(SourceData));
            end += sizeof(SourceData);
        }
    }
    // Grass - 5
    for (u8 x = 3; x <= 5; x++) {
        for (u8 z = 3; z <= 5; z++) {
            const SourceData data = (SourceData) {
                .type = DATA_TYPE_STATELESS,
                .id = x == 4 && z == 4 ? 0 : 2,
                .metadata_id = 0
            };
            if (end + sizeof(SourceData) >= buf_size) {
                buf_size += sizeof(SourceData);
                realloc(input_buffer, buf_size);
            }
            memcpy(&input_buffer[end], &data, sizeof(SourceData));
            end += sizeof(SourceData);
        }
    }
    // Grass - 6
    for (u8 x = 0; x < 8; x++) {
        for (u8 z = 0; z < 8; z++) {
            if (x >= 3 && x <= 5 && z >= 3 && z <= 5) {
                continue;
            }
            const SourceData data = (SourceData) {
                .type = DATA_TYPE_STATELESS,
                .id = x >= 3 && x <= 5 && z >= 3 && z <= 5 ? 0 : 2,
                .metadata_id = 0
            };
            if (end + sizeof(SourceData) >= buf_size) {
                buf_size += sizeof(SourceData);
                realloc(input_buffer, buf_size);
            }
            memcpy(&input_buffer[end], &data, sizeof(SourceData));
            end += sizeof(SourceData);
        }
    }
    // Chests
    ChestData chest = (ChestData) {
        .block = (SourceData) {
            .type = DATA_TYPE_CHEST,
            .id = 54,
            .metadata_id = 0
        },
        .storage = {0}
    };
    chest.storage[6] = ITEM(1, 0, 12);
    chest.storage[17] = ITEM(35, 6, 42);
    chest.storage[19] = ITEM(35, 8, 29);
    chest.storage[25] = ITEM(77, 0, 3);
    if (end + sizeof(ChestData) >= buf_size) {
        buf_size += sizeof(ChestData);
        realloc(input_buffer, buf_size);
    }
    memcpy(&input_buffer[end], &chest, sizeof(ChestData));
    end += sizeof(ChestData);
    ChestData chest2 = (ChestData) {
        .block = (SourceData) {
            .type = DATA_TYPE_CHEST,
            .id = 54,
            .metadata_id = 0
        },
        .storage = {0}
    };
    chest2.storage[3] = ITEM(5, 0, 12);
    chest2.storage[9] = ITEM(33, 0, 1);
    chest2.storage[14] = ITEM(96, 0, 2);
    chest2.storage[15] = ITEM(35, 3, 17);
    if (end + sizeof(ChestData) >= buf_size) {
        buf_size += sizeof(ChestData);
        realloc(input_buffer, buf_size);
    }
    memcpy(&input_buffer[end], &chest2, sizeof(ChestData));
    end += sizeof(ChestData);
    // Furnace
    FurnaceData furnace = (FurnaceData) {
        .block = {
            .type = DATA_TYPE_FURNACE,
            .id = 61,
            .metadata_id = 0
        },
        .storage = {0}
    };
    furnace.storage[0] = ITEM(15, 0, 12);
    furnace.storage[1] = ITEM(104, 0, 3);
    furnace.storage[2] = ITEM(106, 0, 9);
    if (end + sizeof(FurnaceData) >= buf_size) {
        buf_size += sizeof(FurnaceData);
        realloc(input_buffer, buf_size);
    }
    memcpy(&input_buffer[end], &furnace, sizeof(FurnaceData));
    end += sizeof(FurnaceData);
    // Compress
    u8* target_buffer = malloc(end * 2);
    printf("Input Buffer: %p In size: %d\n", input_buffer, end);
    int size = lzCompress(
        target_buffer,
        input_buffer,
        end,
        LZP_COMPRESS_NORMAL
    );
    printf("Output buffer: %p\n", target_buffer);
    printf("[NORMAL] Out size: %d\n", size);
    u32 ratio = ((size << 12) / end) * 100;
    #define fracToFloat(frac) ((u32)(100 * ((frac) / 4096.0)))
    u32 float_part = fracToFloat(ratio & 0b111111111111);
    printf("Ratio: %d.%d%%\n", ratio >> 12, float_part);
    size = lzCompress(
        target_buffer,
        input_buffer,
        end,
        LZP_COMPRESS_MAX
    );
    printf("[MAX] Out size: %d\n", size);
    ratio = ((size << 12) / end) * 100;
    #define fracToFloat(frac) ((u32)(100 * ((frac) / 4096.0)))
    float_part = fracToFloat(ratio & 0b111111111111);
    printf("Ratio: %d.%d%%\n", ratio >> 12, float_part);
    // Card init 
    InitCARD(1);
    printf("Init card\n");
    StartCARD();
    printf("Started card\n");
    // BUG: This causes an unimplemented syscall exception from BIOS
    /*_bu_init();*/
    _temp_bu_init();
    printf("Init backup unit\n");
    int channel = 0x00;
    int result = _card_load(channel);
    printf("Loaded card: %d\n", result);
    result = _card_info(channel);
    printf("Card info: %d\n", result);
    // Card write
    u8 test_data[0x80] = {0};
    test_data[0] = 'T';
    test_data[1] = 'e';
    test_data[2] = 's';
    test_data[3] = 't';
    test_data[4] = '\0';
    result = _card_write(channel, 0, test_data);
    printf("Async write: %d\n", result);
    result = _card_wait(channel);
    printf("Wait card: %d\n", result);
    /*StopCARD();*/
    // Card read
    /*StartCARD();*/
    u8 result_data[0x80] = {0};
    result = _card_read(channel, 0, result_data);
    printf("Async read: %d\n", result);
    result = _card_wait(channel);
    printf("Wait card: %d\n", result);
    printf("Result data: %s\n", (char*) result_data);
    StopCARD();
    // Decompress
    u8* cmp_buf = (u8*) malloc(buf_size);
    size = lzDecompress(cmp_buf, target_buffer, size);
    printf("Decompressed size: %d Original: %d\n", size, end);
    result = memcmp(input_buffer, cmp_buf, end);
    printf("Compare result: %d\n", result);
    while (1);
    return 0;
}
