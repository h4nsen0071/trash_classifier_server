/**
 * @file state_machine.h
 * @brief State machine definitions
 * 
 * Định nghĩa các states của hệ thống
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

// ============================================================
// SYSTEM STATES
// ============================================================

enum SystemState {
    STATE_IDLE,                 // Chờ vật thể
    STATE_OBJECT_DETECTED,      // Phát hiện vật thể
    STATE_WAITING_STABLE,       // Đợi vật thể ổn định
    STATE_REQUESTING_CAPTURE,   // Gửi lệnh CAPTURE
    STATE_WAITING_RESULT,       // Chờ kết quả từ CAM
    STATE_OPENING_BIN,          // Mở nắp thùng
    STATE_HOLDING_OPEN,         // Giữ nắp mở
    STATE_CLOSING_BIN,          // Đóng nắp thùng
    STATE_COOLDOWN,             // Thời gian chờ trước khi tiếp tục
    STATE_ERROR                 // Trạng thái lỗi
};

// ============================================================
// STATE NAMES (for debugging)
// ============================================================

inline const char* getStateName(SystemState state) {
    switch (state) {
        case STATE_IDLE:                return "IDLE";
        case STATE_OBJECT_DETECTED:     return "OBJECT_DETECTED";
        case STATE_WAITING_STABLE:      return "WAITING_STABLE";
        case STATE_REQUESTING_CAPTURE:  return "REQUESTING_CAPTURE";
        case STATE_WAITING_RESULT:      return "WAITING_RESULT";
        case STATE_OPENING_BIN:         return "OPENING_BIN";
        case STATE_HOLDING_OPEN:        return "HOLDING_OPEN";
        case STATE_CLOSING_BIN:         return "CLOSING_BIN";
        case STATE_COOLDOWN:            return "COOLDOWN";
        case STATE_ERROR:               return "ERROR";
        default:                        return "UNKNOWN";
    }
}

#endif // STATE_MACHINE_H
