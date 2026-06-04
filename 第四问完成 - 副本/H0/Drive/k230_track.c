#include "k230_track.h"

uint8_t line_sensor_data = 0x00;
uint8_t k230_rx_data = 0;

void K230_Parse_Byte(uint8_t byte) 
{
    static uint8_t state = 0; 
    
    switch(state) 
    {
        case 0:
            if (byte == 0xAA) state = 1; 
            else state = 0;
            break;
        case 1:
            if (byte == 0x55) state = 2; 
            else state = 0;
            break;
        case 2:
            line_sensor_data = byte;  
            state = 0;             
            break;
        default:
            state = 0;
            break;
    }
}

int16_t K230_Get_Turn_Speed(uint8_t sensor_val) 
{
    static int16_t last_turn = 0; // ??�W�@����?�V�A???�ϩR��
    int16_t turn_speed = 0;
    switch(sensor_val) 
    {
    case 0x0C: turn_speed = 0;   break;     // 001100 正中
    
    // 微偏
    case 0x08: turn_speed = 2;   break;     // 001000
    case 0x04: turn_speed = -2;  break;     // 000100
    case 0x18: turn_speed = 4;   break;     // 011000
    case 0x06: turn_speed = -4;  break;     // 000110
    
    // 中度偏
    case 0x1C: turn_speed = 6;   break;     // 011100
    case 0x0E: turn_speed = -6;  break;     // 001110
    case 0x38: turn_speed = 7;   break;     // 111000
    case 0x07: turn_speed = -7;  break;     // 000111
    
    // 重度偏（单侧内路）
    case 0x10: turn_speed = 10;  break;     // 010000
    case 0x02: turn_speed = -10; break;     // 000010
    
    // 极度偏（最外侧边缘）
    case 0x30: turn_speed = 14;  break;     // 110000 👈 从18压低到14
    case 0x20: turn_speed = 18;  break;     // 100000 👈 从27强行压低到18
    case 0x03: turn_speed = -14; break;     // 000011 👈 从-18压低到-14
    case 0x01: turn_speed = -18; break;     // 000001 👈 从-27强行压低到-18
    
    default: break;

    }
    
    last_turn = turn_speed; 
    return turn_speed;
}