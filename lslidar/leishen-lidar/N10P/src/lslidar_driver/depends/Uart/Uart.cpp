#include "Uart.hpp"
#include <unistd.h>
#include <cstdio>
#include <cerrno>
#include <cstring>

namespace Uart
{
    // ==================== 构造函数 / 析构函数 ====================
    Uart::Uart()
    {
        // 初始化成员变量（阻塞模式最优默认值）
        this->uart_fd = -1;
        this->baud_rate = B9600;
        this->stop_flag = UART_STOP1;
        this->data_flag = UART_DATA8;
        this->check_flag = UART_NONE;
        this->hardware_flow_control = false;
        this->receiver_enable = true;
        this->modem_control_ignore = true;
        this->raw_input = true; // 原始输入
        this->software_flow_control = false;
        this->raw_output = true;  // 原始输出
        this->read_min_bytes = 1; // 至少读取1字节才返回（阻塞）
        this->read_timeout = 10;  // 字节间超时1秒（10 * 0.1秒）
    }

    Uart::~Uart()
    {
        if (this->uart_fd != -1)
            close(this->uart_fd);
    }

    // ==================== 私有核心配置函数 ====================
    bool Uart::applyConfiguration()
    {
        if (this->uart_fd == -1)
            return false;

        // 获取当前 termios
        if (tcgetattr(this->uart_fd, &this->ts) != 0)
        {
            perror("tcgetattr failed");
            return false;
        }

        // ---- 控制模式 c_cflag ----
        // 硬件流控
        if (this->hardware_flow_control)
            this->ts.c_cflag |= CRTSCTS;
        else
            this->ts.c_cflag &= ~CRTSCTS;

        // 接收器使能
        if (this->receiver_enable)
            this->ts.c_cflag |= CREAD;
        else
            this->ts.c_cflag &= ~CREAD;

        // 忽略 modem 控制线
        if (this->modem_control_ignore)
            this->ts.c_cflag |= CLOCAL;
        else
            this->ts.c_cflag &= ~CLOCAL;

        // 数据位
        this->ts.c_cflag &= ~CSIZE;
        switch (this->data_flag)
        {
        case UART_DATA5:
            this->ts.c_cflag |= CS5;
            break;
        case UART_DATA6:
            this->ts.c_cflag |= CS6;
            break;
        case UART_DATA7:
            this->ts.c_cflag |= CS7;
            break;
        case UART_DATA8:
            this->ts.c_cflag |= CS8;
            break;
        default:
            this->ts.c_cflag |= CS8;
            break;
        }

        // 停止位
        if (this->stop_flag == UART_STOP2)
            this->ts.c_cflag |= CSTOPB;
        else
            this->ts.c_cflag &= ~CSTOPB;

        // 校验位
        this->ts.c_cflag &= ~PARENB;
        this->ts.c_iflag &= ~INPCK;
        switch (this->check_flag)
        {
        case UART_ODD:
            this->ts.c_cflag |= (PARENB | PARODD);
            this->ts.c_iflag |= INPCK;
            break;
        case UART_EVEN:
            this->ts.c_cflag |= PARENB;
            this->ts.c_cflag &= ~PARODD;
            this->ts.c_iflag |= INPCK;
            break;
        default:
            break;
        }

        // ---- 本地模式 c_lflag ----
        if (this->raw_input)
            this->ts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        else
            this->ts.c_lflag |= (ICANON | ECHO | ECHOE | ISIG);

        // ---- 输入模式 c_iflag ----
        if (this->software_flow_control)
            this->ts.c_iflag |= (IXON | IXOFF | IXANY);
        else
            this->ts.c_iflag &= ~(IXON | IXOFF | IXANY);

        // ---- 输出模式 c_oflag ----
        if (this->raw_output)
            this->ts.c_oflag &= ~OPOST;
        else
            this->ts.c_oflag |= OPOST;

        // ---- 读取控制 VMIN / VTIME ----
        this->ts.c_cc[VMIN] = this->read_min_bytes;
        this->ts.c_cc[VTIME] = this->read_timeout;

        // 波特率
        cfsetispeed(&this->ts, this->baud_rate);
        cfsetospeed(&this->ts, this->baud_rate);

        // 应用配置
        if (tcsetattr(this->uart_fd, TCSANOW, &this->ts) != 0)
        {
            perror("tcsetattr failed");
            return false;
        }

        return true;
    }

    // ==================== 公有配置接口 ====================
    void Uart::setBaudrate(uint32_t baudrate)
    {
        switch (baudrate)
        {
        case 0:
            this->baud_rate = B0;
            break;
        case 50:
            this->baud_rate = B50;
            break;
        case 75:
            this->baud_rate = B75;
            break;
        case 110:
            this->baud_rate = B110;
            break;
        case 134:
            this->baud_rate = B134;
            break;
        case 150:
            this->baud_rate = B150;
            break;
        case 200:
            this->baud_rate = B200;
            break;
        case 300:
            this->baud_rate = B300;
            break;
        case 600:
            this->baud_rate = B600;
            break;
        case 1200:
            this->baud_rate = B1200;
            break;
        case 1800:
            this->baud_rate = B1800;
            break;
        case 2400:
            this->baud_rate = B2400;
            break;
        case 4800:
            this->baud_rate = B4800;
            break;
        case 9600:
            this->baud_rate = B9600;
            break;
        case 19200:
            this->baud_rate = B19200;
            break;
        case 38400:
            this->baud_rate = B38400;
            break;
        case 57600:
            this->baud_rate = B57600;
            break;
        case 115200:
            this->baud_rate = B115200;
            break;
        case 230400:
            this->baud_rate = B230400;
            break;
        case 460800:
            this->baud_rate = B460800;
            break;
        case 500000:
            this->baud_rate = B500000;
            break;
        case 576000:
            this->baud_rate = B576000;
            break;
        case 921600:
            this->baud_rate = B921600;
            break;
        case 1000000:
            this->baud_rate = B1000000;
            break;
        case 1152000:
            this->baud_rate = B1152000;
            break;
        case 1500000:
            this->baud_rate = B1500000;
            break;
        case 2000000:
            this->baud_rate = B2000000;
            break;
        case 2500000:
            this->baud_rate = B2500000;
            break;
        case 3000000:
            this->baud_rate = B3000000;
            break;
        case 3500000:
            this->baud_rate = B3500000;
            break;
        case 4000000:
            this->baud_rate = B4000000;
            break;
        default:
            fprintf(stderr, "Unsupported baudrate: %u, keeping previous value\n", baudrate);
            return; // 不修改已有波特率，也不应用配置
        }

        // 如果串口已打开，则应用新配置
        if (this->uart_fd != -1)
            this->applyConfiguration();
    }

    void Uart::setStopFlagBit(uint8_t stopFlag)
    {
        this->stop_flag = stopFlag;
        if (this->uart_fd != -1)
            this->applyConfiguration();
    }

    void Uart::setDataFlagBit(uint8_t dataFlag)
    {
        this->data_flag = dataFlag;
        if (this->uart_fd != -1)
            this->applyConfiguration();
    }

    void Uart::setCheckFlagBit(uint8_t checkFlag)
    {
        this->check_flag = checkFlag;
        if (this->uart_fd != -1)
            this->applyConfiguration();
    }

    bool Uart::setHardwareFlowControl(bool enable)
    {
        this->hardware_flow_control = enable;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setReceiverEnable(bool enable)
    {
        this->receiver_enable = enable;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setModemControlIgnore(bool ignore)
    {
        this->modem_control_ignore = ignore;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setRawInput(bool enable)
    {
        this->raw_input = enable;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setSoftwareFlowControl(bool enable)
    {
        this->software_flow_control = enable;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setRawOutput(bool enable)
    {
        this->raw_output = enable;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setReadMinBytes(uint8_t minBytes)
    {
        this->read_min_bytes = minBytes;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    bool Uart::setReadTimeout(uint8_t timeDeciseconds)
    {
        this->read_timeout = timeDeciseconds;
        if (this->uart_fd != -1)
            return this->applyConfiguration();
        return true;
    }

    // ==================== 串口打开 / 关闭 / 状态 ====================
    bool Uart::openUartSerial(const std::string &uartPath, int flag)
    {
        this->uart_path = uartPath;
        return this->openUartSerial(flag);
    }

    bool Uart::openUartSerial(int flag)
    {
        if (this->uart_path.empty())
        {
            fprintf(stderr, "Uart path not set\n");
            return false;
        }

        if (this->uart_fd != -1)
            close(this->uart_fd);

        // 阻塞模式打开（默认 flag 已去除 O_NONBLOCK）
        this->uart_fd = open(this->uart_path.c_str(), flag);
        if (this->uart_fd == -1)
        {
            perror("open device failed");
            return false;
        }

        if (!this->applyConfiguration())
        {
            close(this->uart_fd);
            this->uart_fd = -1;
            return false;
        }

        return true;
    }

    bool Uart::checkUartOpen() const
    {
        return this->uart_fd != -1;
    }

    void Uart::closeUart()
    {
        if (this->uart_fd != -1)
        {
            close(this->uart_fd);
            this->uart_fd = -1;
        }
    }

    // ==================== 数据读写 ====================
    ssize_t Uart::writeData(const char *buf, size_t len)
    {
        if (this->uart_fd == -1)
            return -1;
        return write(this->uart_fd, buf, len);
    }

    ssize_t Uart::readData(char *buf, size_t len)
    {
        if (this->uart_fd == -1)
            return -1;
        return read(this->uart_fd, buf, len);
    }

    void Uart::clearRecvBuffer()
    {
        if (this->uart_fd != -1)
            tcflush(this->uart_fd, TCIFLUSH);
    }
}