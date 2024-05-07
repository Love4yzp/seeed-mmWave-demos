/**
 * @file mylib.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-07
 *
 * @copyright Copyright (c) 2024
 *
 * 呼吸+心率检测项目
 *
 */
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define HEADER_START 0x01
uint8_t get_cksum(uint8_t* data, uint8_t len) {
    uint8_t ret = 0;
    for (int i = 0; i < len; i++)
        ret ^= data[i];
    ret = ~ret;
    return ret;
}

#pragma pack(push, 1)

// 0A13
typedef struct {
    uint8_t sof; // head
    uint16_t id;
    uint16_t len;
    uint16_t type;
    uint8_t head_cksum;
    union {
        uint32_t data[3]; // total_phase, breath_phase, heart_phase
        struct {          // type = 0A13 // 相位测试结果
            uint32_t total_phase;
            uint32_t breath_phase;
            uint32_t heart_phase;
        };
    };
    uint8_t data_cksum;
} HeartBreathPhase, BaseStruct;

typedef struct {
    uint8_t sof; // head
    uint16_t id;
    uint16_t len;
    uint16_t type;
    uint8_t head_cksum;
    union {
        uint32_t data[1]; // total_phase, breath_phase, heart_phase
        struct {          // type = 0A14 呼吸速率
            uint32_t breath_rate;
        };
    };
    uint8_t data_cksum;
} BreathRate;

// 0x0A15
typedef struct {
    uint8_t sof; // head
    uint16_t id;
    uint16_t len;
    uint16_t type;
    uint8_t head_cksum;
    union {
        uint32_t data[1]; // total_phase, breath_phase, heart_phase
        struct {          // type = 0A15 心率心跳速率
            uint32_t heart_rate;
        };
    };
    uint8_t data_cksum;
} HeartRate;

// 0x0A16
typedef struct {
    uint8_t sof; // head
    uint16_t id;
    uint16_t len;
    uint16_t type;
    uint8_t head_cksum;
    union {
        uint32_t data[2]; // total_phase, breath_phase, heart_phase
        struct            // 0x0A16 检测目标距离
        {
            uint32_t flag;  // true 则输出距离 cm false 则没有, 同 float 是一样的大小(esp32?)
            uint32_t range; // distance
        };
    };
    uint8_t data_cksum;
} HeartBreathDistance;
#pragma pack(pop)

enum HeartBreathType {
    HeartBreathPhaseType = 0x0A13,
    BreathRateType = 0x0A14,
    HeartRateType = 0x0A15,
    HeartBreathDistanceType = 0x0A16
};

typedef struct {
    void* data_ptr;
    enum HeartBreathType type;
} Constructer;
// Manual byte-swapping function
uint16_t swap_bytes(uint16_t value) {
    return (value >> 8) | (value << 8);
}
// Manual byte-swapping function for uint32_t
uint32_t swap_bytes_32(uint32_t value) {
    return (value >> 24) | ((value << 8) & 0x00FF0000) | ((value >> 8) & 0x0000FF00) | (value << 24);
}

Constructer* init_heart_breath(uint8_t* data_ptr) {
    if (!data_ptr) {
        // printf("Data pointer is NULL\n");
        return NULL;
    }

    uint8_t sof = *data_ptr;
    if (sof != HEADER_START) {
        // printf("Invalid start of frame: %#X\n", sof);
        return NULL;
    } else {
        // printf("Start of frame: %#X\n", sof);
    }

    uint16_t id_16 = swap_bytes(*(uint16_t*)(data_ptr + offsetof(BaseStruct, id)));
    uint16_t len_16 = swap_bytes(*(uint16_t*)(data_ptr + offsetof(BaseStruct, len)));
    uint16_t type = swap_bytes(*(uint16_t*)(data_ptr + offsetof(BaseStruct, type)));
    uint8_t head_cksum = *(uint8_t*)(data_ptr + offsetof(BaseStruct, head_cksum));
    uint8_t data_cksum = 0;
    // // printf("ID: %#X\n", *(uint16_t *)(data_ptr + offsetof(BaseStruct, id)));
    // printf("ID: %#X\n", id_16);
    // printf("Length: %#X\n", len_16);
    // printf("Type: %#X\n", type);

    if (head_cksum != get_cksum(data_ptr, offsetof(BaseStruct, head_cksum))) {
        // printf("Header checksum mismatch\n");
        return NULL;
    } else {
        // printf("Header checksum: %#X\n", head_cksum);
    }

    // Memory allocation based on type
    void* refer = NULL;
    Constructer* constructer = malloc(sizeof(Constructer));
    constructer->type = type;
    switch (type) {
    case HeartBreathPhaseType:
        // check data cksum
        data_cksum = *(uint8_t*)(data_ptr + offsetof(HeartBreathPhase, data_cksum));
        if (get_cksum(data_ptr + offsetof(HeartBreathPhase, data), sizeof(((HeartBreathPhase*)0)->data)) !=
            data_cksum) {
            // printf("Data checksum mismatch for HeartBreathPhaseType\n");
        }
        refer = malloc(sizeof(HeartBreathPhase));
        break;
    case BreathRateType:
        data_cksum = swap_bytes(*(data_ptr + offsetof(BreathRate, data_cksum)));
        data_cksum = *(uint8_t*)(data_ptr + offsetof(HeartBreathPhase, data_cksum));
        if (get_cksum(data_ptr + offsetof(BreathRate, data), sizeof(((BreathRate*)0)->data)) != data_cksum) {
            // printf("Data checksum mismatch for BreathRateType\n");
        }
        refer = malloc(sizeof(BreathRate));
        break;
    case HeartRateType:
        data_cksum = *(uint8_t*)(data_ptr + offsetof(HeartRate, data_cksum));
        if (get_cksum(data_ptr + offsetof(HeartRate, data), sizeof(((HeartRate*)0)->data)) != data_cksum) {
            // printf("Data checksum mismatch for HeartRateType\n");
        }
        refer = malloc(sizeof(HeartRate));
        break;
    case HeartBreathDistanceType:
        data_cksum = *(uint8_t*)(data_ptr + offsetof(HeartBreathDistance, data_cksum));
        if (get_cksum(data_ptr + offsetof(HeartBreathDistance, data), sizeof(((HeartBreathDistance*)0)->data)) !=
            data_cksum) {
            // printf("Data checksum mismatch for HeartBreathDistanceType\n");
        }
        refer = malloc(sizeof(HeartBreathDistance));
        break;
    default:
        // printf("Unknown type: %#X\n", type);
    }

    if (refer) {
        memcpy(refer, data_ptr, sizeof(BaseStruct)); // Copy the common part
        // printf("Data pointer: %p\n", refer);
    } else {
        return NULL;
    }
    constructer->data_ptr = refer;
    return constructer;
}
void free_heart_breath(Constructer* constructer) {
    if (!constructer) {
        // printf("Constructer is NULL\n");
        return;
    }
    free(constructer->data_ptr);
    free(constructer);
}
void print_heart_breath(Constructer* constructer) {
    // Cast the void pointer to a base struct with common fields
    void* data_ptr = constructer->data_ptr;
    enum HeartBreathType type = constructer->type;

    switch (type) {
    case HeartBreathPhaseType: { // Phase test results
        HeartBreathPhase* phase_data = (HeartBreathPhase*)data_ptr;
        printf("Total Phase: %f\n", phase_data->total_phase);
        printf("Breath Phase: %f\n", phase_data->breath_phase);
        printf("Heart Phase: %f\n", phase_data->heart_phase);
        break;
    }
    case BreathRateType: { // Breath rate
        BreathRate* breath_rate_data = (BreathRate*)data_ptr;
        printf("Breath Rate: %f\n", breath_rate_data->breath_rate);
        break;
    }
    case HeartRateType: { // Heart rate
        HeartRate* heart_rate_data = (HeartRate*)data_ptr;
        printf("Heart Rate: %f\n", heart_rate_data->heart_rate);
        break;
    }
    case HeartBreathDistanceType: { // Detection target distance
        HeartBreathDistance* distance_data = (HeartBreathDistance*)data_ptr;
        if (distance_data->flag) {
            printf("Distance: %f cm\n", distance_data->range);
        } else {
            // printf("No target detected.\n");
        }
        break;
    }
    default:
        // printf("Unknown data type: %#X\n", type);
    }
}

void print_frame(uint8_t* frame, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        // printf("%#X ", frame[i]);
    }
    // printf("\n");
}
int main(int argc, char* argv[]) {
    // 01 42 89 00 0C 0A 13 20 65 62 5A 40 42 FB 63 3F 04 89 1F 3F AA
    uint8_t frame[] = {0x01, 0x42, 0x89, 0x00, 0x0C, 0x0A, 0x13, 0x20, 0x65, 0x62, 0x5A,
                       0x40, 0x42, 0xFB, 0x63, 0x3F, 0x04, 0x89, 0x1F, 0x3F, 0xAA};
    print_frame(frame, sizeof(frame));

    Constructer* constr_ptr = init_heart_breath(frame);
    print_heart_breath(constr_ptr);

    free_heart_breath(constr_ptr);
}