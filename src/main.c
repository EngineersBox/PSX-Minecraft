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
#include "psxpad.h"

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
    _bu_init();
    printf("Init backup unit\n");
    int channel = 0x00;
    int result = _card_load(channel);
    printf("Loaded card: %d\n", result);
    result = _card_info(channel);
    printf("Card info: %d\n", result);
    result = _card_wait(channel);
    printf("Wait card: %d\n", result);
    u8 result_data[0x80] = {0};
    /*result = _card_read(channel, 1, result_data);*/
    /*printf("Async read: %d\n", result);*/
    /*if (result == 0) {*/
    /*    result = _card_info(channel);*/
    /*    printf("Read failed: %d\n", result);*/
    /*}*/
    /*result = _card_wait(channel);*/
    /*printf("Wait card: %d\n", result);*/
    /*printf("Result data: %s\n", (char*) result_data);*/
    // Card write
    /*union {*/
    /*    u8 _data[140];*/
    /*    MemCardRequest req;*/
    /*} test_data = {0};*/
    /*test_data.req.addr = 0x81;*/
    /*test_data.req.cmd = MCD_CMD_WRITE_SECTOR;*/
    /*test_data.req.lba_h = 0;*/
    /*test_data.req.lba_l = 1;*/
    /*test_data.req.checksum;*/
    u8 test_data[0x80] = {0};
    test_data[0] = 'T';
    test_data[1] = 'e';
    test_data[2] = 's';
    test_data[3] = 't';
    test_data[4] = '\0';
    result = _card_write(channel, 1, test_data);
    printf("Async write: %d\n", result);
    if (result == 0) {
        result = _card_info(channel);
        printf("Write failed: %d\n", result);
    }
    result = _card_wait(channel);
    printf("Wait card: %d\n", result);
    /*StopCARD();*/
    // Card read
    /*StartCARD();*/
    memset(result_data, '\0', sizeof(u8) * 0x80);
    result = _card_read(channel, 1, result_data);
    printf("Async read: %d\n", result);
    if (result == 0) {
        result = _card_info(channel);
        printf("Read failed: %d\n", result);
    }
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

#include <stdbool.h>
#include "hardware/spi.h"

static bool write_finished = false;

void writeCallback(u32 port, const volatile u8* buff, size_t rx_len) {
    write_finished = true;
}

static bool read_finished = false;
static char data[4] = {0};

void readCallback(u32 port, const volatile u8* buff, size_t rx_len) {
    printf("RX Len: %d\n", rx_len);
    if (rx_len < 5) {
        read_finished = true;
        return;
    }
    const MemCardResponse* response = (MemCardResponse*) buff;
    #pragma GCC unroll 4
    for (int i = 0; i < 5; i++) {
        data[i] = response->read.data[i];
    }
    printf("Read callback: %s\n", data);
    read_finished = true;
}

volatile int value = 0;

void pollCallback(u32 port, const volatile u8* buf, size_t rx_len) {
    if (rx_len <= 0) {
        printf("Bad response, RX len: %d <= 0\n", rx_len);
        return;
    }
    const MemCardResponse* response = (MemCardResponse*) buf;
    switch (response->flags) {
        case MCD_FLAG_WRITE_ERROR:
            printf("[SPI] Write error\n");
            break;
        case MCD_FLAG_NOT_WRITTEN:
            printf("[SPI] Not written\n");
            break;
        default:
            printf("[SPI] Unknown error\n");
            break;
    }
    switch (response->read.stat) {
        case MCD_STAT_OK:
            printf("[SPI] Status: Ok\n");
            break;
        case MCD_STAT_BAD_CHECKSUM:
            printf("[SPI] Status: Bad Checksum\n");
            break;
        case MCD_STAT_BAD_SECTOR:
            printf("[SPI] Status: Bad Sector\n");
            break;
    }
    value++;
}

int old_main() {
    InitCARD(0);
    printf("Init card\n");
    StartCARD();
    printf("Started card\n");
    _temp_bu_init();
    printf("Init backup unit\n");
    SPI_Init(pollCallback);
    printf("SPI init\n");
    SPI_SetPollRate(65);
    SPI_Request* request = SPI_CreateRequest();
    request->memory_card_request.data[0] = 'T';
    request->memory_card_request.data[1] = 'e';
    request->memory_card_request.data[2] = 's';
    request->memory_card_request.data[3] = 't';
    request->memory_card_request.data[4] = '\0';
    request->memory_card_request.addr = 0x81;
    request->memory_card_request.lba_h = 0x0;
    request->memory_card_request.lba_l = 0x0;
    request->memory_card_request.cmd = MCD_CMD_WRITE_SECTOR;
    request->memory_card_request.checksum =
        request->memory_card_request.lba_h
        ^ request->memory_card_request.lba_l
        ^ 'T' ^ 'e' ^ 's' ^ 't' ^ '\0';
    request->len = SPI_BUFF_LEN;
    request->port = 0x0;
    request->callback = writeCallback;
    printf("Value: %d\n", value);
    printf("Created write request\n");
    while (!write_finished) {
        __asm__ volatile("");
    }
    write_finished = false;
    printf("Finished write\n");
    SPI_Request* request1 = SPI_CreateRequest();
    request1->memory_card_request.addr = 0x81;
    request1->memory_card_request.lba_h = 0x0;
    request1->memory_card_request.lba_l = 0x0;
    request1->memory_card_request.cmd = MCD_CMD_READ_SECTOR;
    request1->len = SPI_BUFF_LEN;
    request1->port = 0x0;
    request1->callback = readCallback;
    printf("Value: %d\n", value);
    printf("Created read request\n");
    while (!read_finished) {
        __asm__ volatile("");
    }
    read_finished = false;
    printf("Finished read\n");
    while (1) {
        __asm__ volatile("");
    }
    StopCARD();
    return 0;
}
