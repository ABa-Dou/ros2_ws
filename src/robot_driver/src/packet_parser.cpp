#include "robot_driver/packet_parser.hpp"

namespace protocol{

    void FrameParser::feed(const uint8_t* data, size_t len)
    {
        buffer_.insert(buffer_.end(), data, data + len);
        try_parse();
    }

    void FrameParser::try_parse()
    {
        while (parse_one_frame().has_value()) {}
    }

    size_t FrameParser::find_header() const
    {
        if (buffer_.size() < 2) return SIZE_MAX;
        for (size_t i = 0; i < buffer_.size() - 1; ++i) {
            if (buffer_[i] == HEADER_0 && buffer_[i + 1] == HEADER_1) {
                return i;
            }
        }
        return SIZE_MAX;
    }

    uint16_t FrameParser::crc16_modbus(const uint8_t* data, size_t len)
    {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < len; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    std::optional<Frame> FrameParser::parse_one_frame(){
        if (buffer_.size() < 14) return std::nullopt;

        size_t header_pos = find_header();
        if (header_pos == SIZE_MAX) {
            if (buffer_.back() == HEADER_0) {
                uint8_t last = buffer_.back();
                buffer_.clear();
                buffer_.push_back(last);
            } else {
                buffer_.clear();
            }
            return std::nullopt;
        }

        if (header_pos > 0) {
            buffer_.erase(buffer_.begin(), buffer_.begin() + header_pos);
        }

        if (buffer_.size() < 4) return std::nullopt;

        uint16_t length = buffer_[2] | (buffer_[3] << 8);
        if (length < 8) {
            buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
            return std::nullopt;
        }

        uint16_t payload_len = length - 8;
        size_t total_len = 2 + 2 + payload_len + 2;

        if (buffer_.size() < total_len) return std::nullopt;

        if (buffer_[total_len - 2] != TAIL_0 || buffer_[total_len - 1] != TAIL_1) {
            LOGW("tail mismatch %d %d", buffer_[total_len - 2], buffer_[total_len - 1]);
            buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
            return std::nullopt;
        }

        size_t crc_offset = 2 + 2 + length - 2;
        uint16_t crc_recv = buffer_[crc_offset] | (buffer_[crc_offset + 1] << 8);
        uint16_t crc_calc = crc16_modbus(buffer_.data() + 2, 2 + length - 2);

        if (crc_calc != crc_recv) {
            LOGW("CRC mismatch %d %d", crc_calc, crc_recv);
            buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
            return std::nullopt;
        }

        uint8_t type_val = buffer_[4];
        uint8_t seq = buffer_[5];
        uint32_t timestamp = 0;
        std::memcpy(&timestamp, &buffer_[6], 4);
        const uint8_t* payload = &buffer_[10];

        Frame frame;
        frame.type = static_cast<FrameType>(type_val);
        frame.seq = seq;
        frame.timestamp = timestamp;

        switch (frame.type) {
            case FrameType::SPEED:{
                if (payload_len < 16) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.type = FrameType::SPEED;
                std::memcpy(&frame.speed.lf, payload + 0, 4);
                std::memcpy(&frame.speed.rf, payload + 4, 4);
                std::memcpy(&frame.speed.lr, payload + 8, 4);
                std::memcpy(&frame.speed.rr, payload + 12, 4);
                if (on_speed) on_speed(frame.speed, seq, timestamp);
                break;
            }
            case FrameType::PULSE:{
                if (payload_len < 16) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.type = FrameType::PULSE;
                std::memcpy(&frame.pulse.lf, payload + 0, 4);
                std::memcpy(&frame.pulse.rf, payload + 4, 4);
                std::memcpy(&frame.pulse.lr, payload + 8, 4);
                std::memcpy(&frame.pulse.rr, payload + 12, 4);
                if (on_pulse) on_pulse(frame.pulse, seq, timestamp);
                break;
            }
            case FrameType::IMU:{
                if (payload_len < 24) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.type = FrameType::IMU;
                std::memcpy(&frame.imu.ax, payload + 0, 4);
                std::memcpy(&frame.imu.ay, payload + 4, 4);
                std::memcpy(&frame.imu.az, payload + 8, 4);
                std::memcpy(&frame.imu.gx, payload + 12, 4);
                std::memcpy(&frame.imu.gy, payload + 16, 4);
                std::memcpy(&frame.imu.gz, payload + 20, 4);
                if (on_imu) on_imu(frame.imu, seq, timestamp);
                break;
            }
            case FrameType::LIDAR:{
                if (payload_len < 2) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.type = FrameType::LIDAR;
                std::memcpy(&frame.lidar.point_count, payload + 0, 2);
                if (payload_len < 2 + static_cast<size_t>(frame.lidar.point_count) * 8) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.lidar.points.reserve(frame.lidar.point_count);
                for (uint16_t k = 0; k < frame.lidar.point_count; ++k) {
                    LidarPoint pt;
                    std::memcpy(&pt.angle,    payload + 2 + k * 8, 4);
                    std::memcpy(&pt.distance, payload + 2 + k * 8 + 4, 4);
                    frame.lidar.points.push_back(pt);
                }
                if (on_lidar) on_lidar(frame.lidar, seq, timestamp);
                break;
            }
            case FrameType::PID:{
                if (payload_len < 112) {
                    buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                    return std::nullopt;
                }
                frame.type = FrameType::PID;
                std::memcpy(frame.pid.kp,                      payload + 0,  16);
                std::memcpy(frame.pid.ki,                      payload + 16, 16);
                std::memcpy(frame.pid.kd,                      payload + 32, 16);
                std::memcpy(frame.pid.ff_gain,                 payload + 48, 16);
                std::memcpy(frame.pid.integral_limit,          payload + 64, 16);
                std::memcpy(frame.pid.integral_separation_err, payload + 80, 16);
                std::memcpy(frame.pid.filter_alpha,            payload + 96, 16);
                if (on_pid) on_pid(frame.pid, seq, timestamp);
                break;
            }
            default:{
                LOGW("Unknown frame type %d", frame.type);
                buffer_.erase(buffer_.begin(), buffer_.begin() + 2);
                return std::nullopt;
            }
        }
        buffer_.erase(buffer_.begin(), buffer_.begin() + total_len);
        return frame;
    }

} // namespace protocol