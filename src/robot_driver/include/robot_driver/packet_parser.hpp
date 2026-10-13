#pragma once
#include <string>
#include <functional>
#include "easy_log.hpp"
namespace protocol{
    enum class FrameType : uint8_t {
        SPEED    = 1,
        PULSE    = 2,
        IMU      = 3,
        LIDAR    = 4,
        SPEED_CMD = 5,
        PID      = 6,
    };

    struct SpeedData {
        float lf;
        float rf;
        float lr;
        float rr;
    };

    struct PulseData {
        int32_t lf;
        int32_t rf;
        int32_t lr;
        int32_t rr;
    };

    struct ImuData {
        float ax, ay, az;
        float gx, gy, gz;
    };

    struct LidarPoint {
        float angle;
        float distance;
    };

    struct LidarData {
        uint16_t point_count;
        std::vector<LidarPoint> points;
    };

    struct PidData {
        float kp[4];
        float ki[4];
        float kd[4];
        float ff_gain[4];
        float integral_limit[4];
        float integral_separation_err[4];
        float filter_alpha[4];
    };

    struct Frame {
        FrameType type;
        uint8_t seq;
        uint32_t timestamp;
        SpeedData speed{};
        PulseData pulse{};
        ImuData imu{};
        LidarData lidar{};
        PidData pid{};
    };


    class FrameParser {
        public:
            FrameParser() = default;

            void feed(const uint8_t* data, size_t len);

            static uint16_t crc16_modbus(const uint8_t* data, size_t len);

            static constexpr uint8_t HEADER_0 = 0xAA;
            static constexpr uint8_t HEADER_1 = 0x55;
            static constexpr uint8_t TAIL_0   = 0x0D;
            static constexpr uint8_t TAIL_1   = 0x0A;

            std::function<void(const SpeedData&, uint8_t seq, uint32_t ts)> on_speed;
            std::function<void(const PulseData&, uint8_t seq, uint32_t ts)> on_pulse;
            std::function<void(const ImuData&,   uint8_t seq, uint32_t ts)> on_imu;
            std::function<void(const LidarData&, uint8_t seq, uint32_t ts)> on_lidar;
            std::function<void(const PidData&,   uint8_t seq, uint32_t ts)> on_pid;

        private:
            std::vector<uint8_t> buffer_;

            void try_parse();
            size_t find_header() const;
            std::optional<Frame> parse_one_frame();
    };
}