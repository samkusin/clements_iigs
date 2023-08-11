#include "hddcard.h"
#include "clem_shared.h"
#include "clem_smartport.h"
#include "prodos_hdd32.h"
#include "slot7hdd_firmware.h"

#include "clem_debug.h"
#include "serializer.h"

#include <stdlib.h>
#include <string.h>

#define CLEM_CARD_HDD_STATE_IDLE      0x00
#define CLEM_CARD_HDD_STATE_COMMAND   0x01
#define CLEM_CARD_HDD_STATE_DMA_W     0x02
#define CLEM_CARD_HDD_STATE_DMA_R     0x04
#define CLEM_CARD_HDD_STATE_DMA       (CLEM_CARD_HDD_STATE_DMA_W + CLEM_CARD_HDD_STATE_DMA_R)
#define CLEM_CARD_HDD_STATE_FORMAT    0x08
#define CLEM_CARD_HDD_STATE_SMARTPORT 0x10

/*
+ 0x70 for slot 7
IO_CONTROL      equ  $C080          ; Write control, Read Handshake
IO_COMMAND      equ  $C081          ; Write command bytes
IO_RESULT0      equ  $C082          ; results for status = X
IO_RESULT1      equ  $C083          ; results for status = Y

Control and Command protocol
CTL: write $00 to start a command (this will cancel any current command)
CTL: read until bit 7 is lo (ready, idle)
CMD: write command bytes
CTL: when done, Write $80 to control fire the command
CTL: Read where bit 7 is hi for being in progress
                bit 0 is hi if error
CMD: Read the error code until CTL is set back $00
*/
#define CLEM_CARD_HDD_IO_CONTROL 0x00
#define CLEM_CARD_HDD_IO_COMMAND 0x01
#define CLEM_CARD_HDD_IO_RESULT0 0x02
#define CLEM_CARD_HDD_IO_RESULT1 0x03

#define CLEM_CARD_HDD_RES_ERROR 0x00
#define CLEM_CARD_HDD_RES_MISC  0x01
#define CLEM_CARD_HDD_RES_0     0x02
#define CLEM_CARD_HDD_RES_1     0x03

#define CLEM_CARD_HDD_PRODOS_ERR_NONE  0x00
#define CLEM_CARD_HDD_PRODOS_ERR_IO    0x27
#define CLEM_CARD_HDD_PRODOS_ERR_NODEV 0x28
#define CLEM_CARD_HDD_PRODOS_ERR_WPROT 0x2b

#define CLEM_CARD_HDD_CONTROL_FLAG_OK          0x00
#define CLEM_CARD_HDD_CONTROL_FLAG_ERROR       0x01
#define CLEM_CARD_HDD_CONTROL_FLAG_IN_PROGRESS 0x80

#define CLEM_CARD_HDD_COMMAND_STATUS 0x00
#define CLEM_CARD_HDD_COMMAND_READ   0x01
#define CLEM_CARD_HDD_COMMAND_WRITE  0x02
#define CLEM_CARD_HDD_COMMAND_FORMAT 0x03

#define CLEM_CARD_HDD_MISC_SMARTPORT_COMMAND_READY 0x80

struct ClemensHddCardSmartPortStatus {
    uint16_t status_list_ptr;
    uint8_t status_code;
};

struct ClemensHddCardSmartPortBlockIO {
    uint16_t buffer_ptr;
    unsigned block_num;
};

struct ClemensHddCardSmartPortControl {
    uint16_t control_list_ptr;
    uint8_t code;
};

union ClemensHddCardSmartPort {
    struct ClemensHddCardSmartPortStatus status;
    struct ClemensHddCardSmartPortBlockIO blockio;
    struct ClemensHddCardSmartPortControl control;
};

struct ClemensHddCardContext {
    struct ClemensProdosHDD32 *hdd[2];
    unsigned state;
    uint8_t cmd_num;
    uint8_t unit_num;
    uint8_t smartport_param_cnt;
    uint8_t smartport_param_byte;
    uint8_t write_prot[2];
    uint8_t drive_index[2];
    uint16_t dma_addr;
    uint16_t dma_offset;
    uint32_t block_num;
    uint8_t results[4]; // used for returning results in IO_RESULT0/1, error code
    union ClemensHddCardSmartPort smartport; // see cmd_num for selection
    uint8_t block_data[512];
};

static void inline _clem_card_hdd_fail_idle(struct ClemensHddCardContext *context, uint8_t err) {
    context->results[CLEM_CARD_HDD_RES_ERROR] = err;
    context->results[CLEM_CARD_HDD_RES_MISC] = 0xff;
    if (err != CLEM_CARD_HDD_PRODOS_ERR_NODEV && err != CLEM_CARD_HDD_PRODOS_ERR_NODEV) {
        CLEM_WARN("hddcard: device error %02X (state: %04X, command %02X )", err, context->state,
                  context->cmd_num);
    }
    context->state = CLEM_CARD_HDD_STATE_IDLE;
    context->cmd_num = CLEM_CARD_HDD_COMMAND_STATUS;
}

static void inline _clem_card_hdd_ok(struct ClemensHddCardContext *context) {
    context->results[CLEM_CARD_HDD_RES_ERROR] = CLEM_CARD_HDD_PRODOS_ERR_NONE;
    context->results[CLEM_CARD_HDD_RES_MISC] = 0x00;
    context->state = CLEM_CARD_HDD_STATE_IDLE;
    context->cmd_num = CLEM_CARD_HDD_COMMAND_STATUS;
}

static void _clem_card_hdd_command(struct ClemensHddCardContext *context) {
    uint16_t prodos_block_max;
    uint8_t drive_index = context->unit_num >> 7;
    if (!context->hdd[drive_index]) {
        _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_NODEV);
        return;
    }
    prodos_block_max = context->hdd[drive_index]->block_limit;
    if (context->hdd[drive_index]->block_limit >= 0x10000) {
        prodos_block_max = 0xffff;
    }
    switch (context->cmd_num) {
    case CLEM_CARD_HDD_COMMAND_STATUS:
        context->results[CLEM_CARD_HDD_RES_0] = (uint8_t)(prodos_block_max & 0xff);
        context->results[CLEM_CARD_HDD_RES_1] = (uint8_t)((prodos_block_max >> 8) & 0xff);
        _clem_card_hdd_ok(context);
        break;
    case CLEM_CARD_HDD_COMMAND_READ:
        context->dma_offset = 0;
        context->state = CLEM_CARD_HDD_STATE_DMA_W; // write to memory from reading disk
        context->results[CLEM_CARD_HDD_RES_0] = (*context->hdd[drive_index]->read_block)(
            context->hdd[drive_index]->user_context, 0, context->block_num, context->block_data);
        if (context->results[CLEM_CARD_HDD_RES_0] != CLEM_SMARTPORT_STATUS_CODE_OK) {
            _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
        }
        break;
    case CLEM_CARD_HDD_COMMAND_WRITE:
        context->dma_offset = 0;
        context->state = CLEM_CARD_HDD_STATE_DMA_R; // read from memory to write on disk
        if (context->block_num > prodos_block_max) {
            _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
        }
        break;
    case CLEM_CARD_HDD_COMMAND_FORMAT:
        // used as our block index until we hit block limit
        context->block_num = 0;
        // used to count current line (8 byte string) in 512 byte block
        context->dma_offset = 0;
        memset(context->block_data, 0, sizeof(context->block_data));
        context->state = CLEM_CARD_HDD_STATE_FORMAT;
        break;
    default:
        _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
        break;
    }
}

static void _clem_card_hdd_smartport(struct ClemensHddCardContext *context) {}

static void _clem_card_hdd_setup_smartport(struct ClemensHddCardContext *context, uint8_t data) {
    switch (context->cmd_num) {
    case CLEM_SMARTPORT_COMMAND_STATUS:
        if (context->smartport_param_byte == 0) {
            context->smartport.status.status_list_ptr = data;
        } else if (context->smartport_param_byte == 1) {
            context->smartport.status.status_list_ptr |= ((uint16_t)(data) << 8);
            context->smartport_param_cnt--;
        } else if (context->smartport_param_byte == 2) {
            context->smartport.status.status_code = data;
            context->smartport_param_cnt--;
        }
        context->smartport_param_byte++;
        break;
    case CLEM_SMARTPORT_COMMAND_READBLOCK:
        break;
    case CLEM_SMARTPORT_COMMAND_WRITEBLOCK:
        break;
    case CLEM_SMARTPORT_COMMAND_FORMAT:
        break;
    case CLEM_SMARTPORT_COMMAND_CONTROL:
        break;
    case CLEM_SMARTPORT_COMMAND_INIT:
        break;
    }
}

static const char *io_name(void *context) { return "hddcard"; }

static void io_reset(struct ClemensClock *clock, void *ctxptr) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(ctxptr);

    //  HDD mount NOT reset (hdd, write_prot, drive_index)
    context->state = CLEM_CARD_HDD_STATE_IDLE;
    context->cmd_num = 0;
    context->unit_num = 0;
    context->dma_addr = 0;
    context->dma_offset = 0;
    context->block_num = 0;
    memset(context->results, 0, sizeof(context->results));
}

static uint32_t io_sync(struct ClemensClock *clock, void *ctxptr) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(ctxptr);
    unsigned i;
    uint8_t drive_index = context->unit_num >> 7;
    if (context->state != CLEM_CARD_HDD_STATE_FORMAT) {
        return (context->state & CLEM_CARD_HDD_STATE_DMA) ? CLEM_CARD_DMA : 0;
    }
    if (!context->hdd[drive_index]) {
        _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_NODEV);
    } else if (context->block_num < context->hdd[drive_index]->block_limit) {
        if (++context->dma_offset >= 64) {
            context->dma_offset = 0;
            context->results[CLEM_CARD_HDD_RES_0] = (*context->hdd[drive_index]->write_block)(
                context->hdd[drive_index]->user_context, 0, context->block_num,
                context->block_data);
            if (context->results[CLEM_CARD_HDD_RES_0] != CLEM_SMARTPORT_STATUS_CODE_OK) {
                _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
            }
            ++context->block_num;
        }
    } else {
        _clem_card_hdd_ok(context);
    }
    return 0;
}

static uint32_t io_dma(uint8_t *data_bank, uint16_t *adr, uint8_t is_adr_bus, void *ctxptr) {
    // MISC = 0; is_adr_bus = true; *data_bank = 0x00 (bank); adr = dma_offset; MISC++
    // MISC = 1; is_adr_bus = false; *data_bank = data; dma_offset++;
    // if dma_offset == 512 switch to IDLE, MISC = 0
    // return 0 for DMA read
    // return 1 for DMA write
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(ctxptr);
    uint32_t out;
    uint8_t drive_index;
    if (context->dma_offset >= (uint16_t)sizeof(context->block_data)) {
        return 0; // default to read, as reading is a non-destructive event
    }
    out = (context->state == CLEM_CARD_HDD_STATE_DMA_W) ? 1 : 0;
    if (is_adr_bus) {
        *data_bank = 0x00;
        *adr = context->dma_addr + context->dma_offset;
    } else {
        if (context->state == CLEM_CARD_HDD_STATE_DMA_W) {
            *data_bank = context->block_data[context->dma_offset++];
        } else {
            context->block_data[context->dma_offset++] = *data_bank;
        }
    }
    drive_index = context->unit_num >> 7;
    if (context->dma_offset == (uint16_t)sizeof(context->block_data)) {
        if (context->cmd_num == CLEM_CARD_HDD_COMMAND_WRITE) {
            // commit block
            if (context->write_prot[drive_index]) {
                _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_WPROT);
            } else {
                context->results[CLEM_CARD_HDD_RES_0] = (*context->hdd[drive_index]->write_block)(
                    context->hdd[drive_index]->user_context, 0, context->block_num,
                    context->block_data);
                if (context->results[CLEM_CARD_HDD_RES_0] != CLEM_SMARTPORT_STATUS_CODE_OK) {
                    _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
                } else {
                    _clem_card_hdd_ok(context);
                }
            }
        } else {
            _clem_card_hdd_ok(context);
        }
    }
    return out;
}

static void io_read(struct ClemensClock *clock, uint8_t *data, uint8_t addr, uint8_t flags,
                    void *ctxptr) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(ctxptr);
    if (flags & CLEM_OP_IO_DEVSEL) {
        *data = slot7hdd_firmware_bytes[addr];
    } else if (!(addr & 0xf0)) {
        // I/O line c0x0 - c0xf
        addr &= 0x0f;
        switch (addr) {
        case CLEM_CARD_HDD_IO_CONTROL:
            if (context->state == CLEM_CARD_HDD_STATE_IDLE) {
                *data = CLEM_CARD_HDD_CONTROL_FLAG_OK;
                if (context->results[CLEM_CARD_HDD_RES_MISC] == 0xff)
                    *data |= CLEM_CARD_HDD_CONTROL_FLAG_ERROR;
            } else if (context->state != CLEM_CARD_HDD_STATE_COMMAND ||
                       context->results[CLEM_CARD_HDD_RES_MISC] > 0x00) {
                *data = CLEM_CARD_HDD_CONTROL_FLAG_IN_PROGRESS;
            }
            break;
        case CLEM_CARD_HDD_IO_COMMAND:
            *data = context->results[CLEM_CARD_HDD_RES_ERROR];
            if (!(flags & CLEM_OP_IO_NO_OP)) {
                context->results[CLEM_CARD_HDD_RES_ERROR] = CLEM_CARD_HDD_PRODOS_ERR_NONE;
            }
            break;
        case CLEM_CARD_HDD_IO_RESULT0:
            *data = context->results[CLEM_CARD_HDD_RES_0];
            break;
        case CLEM_CARD_HDD_IO_RESULT1:
            *data = context->results[CLEM_CARD_HDD_RES_1];
            break;
        }
    }
}

static void io_write(struct ClemensClock *clock, uint8_t data, uint8_t addr, uint8_t flags,
                     void *ctxptr) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(ctxptr);
    if (!(flags & CLEM_OP_IO_DEVSEL) && !(addr & 0xf0)) {
        // I/O line c0x0 - c0xf
        addr &= 0x0f;
        switch (addr) {
        case CLEM_CARD_HDD_IO_CONTROL:
            if (context->state == CLEM_CARD_HDD_STATE_IDLE) {
                context->state =
                    !data ? CLEM_CARD_HDD_STATE_COMMAND : CLEM_CARD_HDD_STATE_SMARTPORT;
                context->cmd_num = CLEM_CARD_HDD_COMMAND_STATUS;
                context->unit_num = 0x00;
                context->smartport_param_cnt = 0xff;
                context->smartport_param_byte = 0x00;
                context->results[CLEM_CARD_HDD_RES_MISC] = 0x00;
            } else if (context->state == CLEM_CARD_HDD_STATE_COMMAND) {
                //  firmware sets data == 0x80, but since this is a state machine
                //  we can just handle writes
                if (context->results[CLEM_CARD_HDD_RES_MISC] == 0x06) {
                    //  fire!
                    _clem_card_hdd_command(context);
                } else {
                    //  command not well formed
                    _clem_card_hdd_fail_idle(context, CLEM_CARD_HDD_PRODOS_ERR_IO);
                }
            } else if (context->state == CLEM_CARD_HDD_STATE_SMARTPORT) {
                if (context->results[CLEM_CARD_HDD_RES_MISC] >=
                    CLEM_CARD_HDD_MISC_SMARTPORT_COMMAND_READY) {
                    //  fire!
                    _clem_card_hdd_smartport(context);
                } else {
                    //  command not well formed
                    _clem_card_hdd_fail_idle(context, CLEM_SMARTPORT_STATUS_CODE_BUS_ERR);
                }

            } else {
                //  can interrupt operation if a command is in progress
                _clem_card_hdd_ok(context);
            }
            break;
        case CLEM_CARD_HDD_IO_COMMAND:
            if (context->state == CLEM_CARD_HDD_STATE_COMMAND) {
                //  set up DMA if applicable for the fire command
                switch (context->results[CLEM_CARD_HDD_RES_MISC]) {
                case 0: // command byte
                    context->cmd_num = data;
                    break;
                case 1: // unit
                    context->unit_num = data;
                    break;
                case 2: // buffer ptr lo
                    context->dma_addr &= 0xff00;
                    context->dma_addr |= data;
                    break;
                case 3: // buffer ptr hi
                    context->dma_addr &= 0x00ff;
                    context->dma_addr |= ((uint16_t)(data) << 8);
                    break;
                case 4: // block number lo
                    context->block_num &= 0xff00;
                    context->block_num |= data;
                    break;
                case 5: // block number hi
                    context->block_num &= 0x00ff;
                    context->block_num |= ((uint16_t)(data) << 8);
                    break;
                default:
                    CLEM_WARN("hddcard: command overflow (%02X)", data);
                    break;
                }
                ++context->results[CLEM_CARD_HDD_RES_MISC];
            } else if (context->state == CLEM_CARD_HDD_STATE_SMARTPORT) {
                switch (context->results[CLEM_CARD_HDD_RES_MISC]) {
                case 0: //  command
                    context->cmd_num = data;
                    break;
                case 1: //  parameter count
                    context->smartport_param_cnt = data;
                    break;
                case 2: //  unit number
                    context->unit_num = data;
                    context->smartport_param_cnt--;
                    break;
                default:
                    _clem_card_hdd_setup_smartport(context, data);
                    break;
                }
                if (context->smartport_param_cnt > 0) {
                    ++context->results[CLEM_CARD_HDD_RES_MISC];
                } else {
                    context->results[CLEM_CARD_HDD_RES_MISC] =
                        CLEM_CARD_HDD_MISC_SMARTPORT_COMMAND_READY;
                }
            }
            break;
        case CLEM_CARD_HDD_IO_RESULT0: // nop
        case CLEM_CARD_HDD_IO_RESULT1: // nop
            break;
        }
    }
}

void clem_card_hdd_initialize(ClemensCard *card) {
    struct ClemensHddCardContext *context =
        (struct ClemensHddCardContext *)calloc(1, sizeof(struct ClemensHddCardContext));
    context->drive_index[0] = CLEM_CARD_HDD_INDEX_NONE;
    context->drive_index[1] = CLEM_CARD_HDD_INDEX_NONE;

    card->context = context;
    card->io_reset = &io_reset;
    card->io_sync = &io_sync;
    card->io_read = &io_read;
    card->io_write = &io_write;
    card->io_name = &io_name;
    card->io_dma = &io_dma;
}

void clem_card_hdd_uninitialize(ClemensCard *card) {
    free(card->context);
    card->context = NULL;
    card->io_reset = NULL;
    card->io_sync = NULL;
    card->io_read = NULL;
    card->io_write = NULL;
    card->io_name = NULL;
    card->io_dma = NULL;
}

void clem_card_hdd_mount(ClemensCard *card, ClemensProdosHDD32 *hdd, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    context->hdd[drive_index - 1] = hdd;
    context->drive_index[drive_index - 1] = drive_index;
}

ClemensProdosHDD32 *clem_card_get_mount(ClemensCard *card, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    return context->hdd[drive_index - 1];
}

ClemensProdosHDD32 *clem_card_hdd_unmount(ClemensCard *card, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    struct ClemensProdosHDD32 *hdd = context->hdd[drive_index - 1];
    context->hdd[drive_index - 1] = NULL;
    context->drive_index[drive_index - 1] = CLEM_CARD_HDD_INDEX_NONE;
    return hdd;
}

unsigned clem_card_hdd_get_status(ClemensCard *card, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    unsigned status = 0;
    if (context->cmd_num == CLEM_CARD_HDD_COMMAND_READ ||
        context->cmd_num == CLEM_CARD_HDD_COMMAND_WRITE) {
        uint8_t cmd_drive_index = context->unit_num >> 7;
        if (cmd_drive_index == drive_index - 1) {
            status |= CLEM_CARD_HDD_STATUS_DRIVE_ON;
        }
    }
    return status;
}

void clem_card_hdd_lock(ClemensCard *card, uint8_t lock, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    //  note, this will take effect on the next block write
    context->write_prot[drive_index - 1] = lock ? 1 : 0;
}

uint8_t clem_card_hdd_drive_index_has_image(ClemensCard *card, uint8_t drive_index) {
    struct ClemensHddCardContext *context = (struct ClemensHddCardContext *)(card->context);
    return context->drive_index[drive_index - 1];
}

static struct ClemensSerializerRecord kCard[] = {
    // hdd is fixed up after the initial load by the owning system
    CLEM_SERIALIZER_RECORD_UINT32(struct ClemensHddCardContext, state),
    CLEM_SERIALIZER_RECORD_UINT8(struct ClemensHddCardContext, cmd_num),
    CLEM_SERIALIZER_RECORD_UINT8(struct ClemensHddCardContext, unit_num),
    CLEM_SERIALIZER_RECORD_ARRAY(struct ClemensHddCardContext, kClemensSerializerTypeUInt8,
                                 write_prot, 2, 0),
    CLEM_SERIALIZER_RECORD_ARRAY(struct ClemensHddCardContext, kClemensSerializerTypeUInt8,
                                 drive_index, 2, 0),
    CLEM_SERIALIZER_RECORD_UINT16(struct ClemensHddCardContext, dma_addr),
    CLEM_SERIALIZER_RECORD_UINT16(struct ClemensHddCardContext, dma_offset),
    CLEM_SERIALIZER_RECORD_UINT32(struct ClemensHddCardContext, block_num),
    CLEM_SERIALIZER_RECORD_ARRAY(struct ClemensHddCardContext, kClemensSerializerTypeUInt8, results,
                                 4, 0),
    CLEM_SERIALIZER_RECORD_ARRAY(struct ClemensHddCardContext, kClemensSerializerTypeUInt8,
                                 block_data, 512, 0),
    CLEM_SERIALIZER_RECORD_EMPTY()};

void clem_card_hdd_serialize(mpack_writer_t *writer, ClemensCard *card) {
    struct ClemensSerializerRecord root;
    memset(&root, 0, sizeof(root));
    root.type = kClemensSerializerTypeRoot;
    root.records = &kCard[0];
    clemens_serialize_object(writer, (uintptr_t)card->context, &root);
}

void clem_card_hdd_unserialize(mpack_reader_t *reader, ClemensCard *card,
                               ClemensSerializerAllocateCb alloc_cb, void *context) {
    struct ClemensSerializerRecord root;
    memset(&root, 0, sizeof(root));
    root.type = kClemensSerializerTypeRoot;
    root.records = &kCard[0];
    clemens_unserialize_object(reader, (uintptr_t)card->context, &root, alloc_cb, context);
}
