  GNU nano 2.9.3                                                     cmd_interface_linux.h                                                               

#ifndef __LINUX_SERIAL_PORT_H__
#define __LINUX_SERIAL_PORT_H__

#include <thread>
#include <inttypes.h>
#include <atomic>
#include <mutex>
#include <vector>
#include <functional>
#include <string>
#include <condition_variable>
#include <string.h>

class CmdInterfaceLinux
{
public:
    CmdInterfaceLinux(int32_t ver);
    ~CmdInterfaceLinux();

    bool Open(std::string& port_name);
    bool Close();
    bool ReadFromIO(uint8_t *rx_buf, uint32_t rx_buf_len, uint32_t *rx_len);
    bool WriteToIo(const uint8_t *tx_buf, uint32_t tx_buf_len, uint32_t *tx_len);
    bool GetCmdDevices(std::vector<std::pair<std::string, std::string> >& device_list);
        void SetReadCallback(std::function<void(const char *, size_t length)> callback) { mReadCallback = callback; }
        bool IsOpened() { return mIsCmdOpened.load(); };


private:
    std::thread *mRxThread;
    static void mRxThreadProc(void *param);
        long long mRxCount;
    int32_t version;
    int32_t mComHandle;
    std::atomic<bool> mIsCmdOpened, mRxThreadExitFlag;
        std::function<void(const char *, size_t length)> mReadCallback;
};

#endif
