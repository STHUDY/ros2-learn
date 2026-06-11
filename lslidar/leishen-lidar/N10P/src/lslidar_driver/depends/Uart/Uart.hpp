#pragma once

#include <string>
#include <cstdint>
#include <termios.h>
#include <fcntl.h> // 为了 O_RDWR, O_NOCTTY

namespace Uart
{
    // 停止位
    enum
    {
        UART_STOP1, // 1位停止位
        UART_STOP2  // 2位停止位
    };

    // 数据位
    enum
    {
        UART_DATA5, // 5位数据位
        UART_DATA6, // 6位数据位
        UART_DATA7, // 7位数据位
        UART_DATA8  // 8位数据位
    };

    // 校验位
    enum
    {
        UART_NONE, // 无校验
        UART_ODD,  // 奇校验
        UART_EVEN  // 偶校验
    };

    class Uart
    {
    public:
        Uart();
        ~Uart();

        // 配置串口参数（setter 会自动更新已打开的串口）
        void setBaudrate(uint32_t baudrate);
        void setStopFlagBit(uint8_t stopFlag);
        void setDataFlagBit(uint8_t dataFlag);
        void setCheckFlagBit(uint8_t checkFlag);
        bool setHardwareFlowControl(bool enable);
        bool setReceiverEnable(bool enable);
        bool setModemControlIgnore(bool ignore);
        bool setRawInput(bool enable);
        bool setSoftwareFlowControl(bool enable);
        bool setRawOutput(bool enable);
        bool setReadMinBytes(uint8_t minBytes);
        bool setReadTimeout(uint8_t timeDeciseconds);

        // 打开/关闭串口
        bool openUartSerial(const std::string &uartPath, int flag = O_RDWR | O_NOCTTY); // 默认阻塞模式
        bool openUartSerial(int flag = O_RDWR | O_NOCTTY);                              // 使用之前设置的路径
        bool checkUartOpen() const;
        void closeUart();

        // 读写操作
        ssize_t writeData(const char *buf, size_t len);
        ssize_t readData(char *buf, size_t len);
        void clearRecvBuffer();

    private:
        bool applyConfiguration();

        int uart_fd;
        struct termios ts;
        std::string uart_path;

        speed_t baud_rate;
        uint8_t stop_flag;
        uint8_t data_flag;
        uint8_t check_flag;
        bool hardware_flow_control;
        bool receiver_enable;
        bool modem_control_ignore;
        bool raw_input;
        bool software_flow_control;
        bool raw_output;
        uint8_t read_min_bytes;
        uint8_t read_timeout; // 单位：0.1秒
    };
}