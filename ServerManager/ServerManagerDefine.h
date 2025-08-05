#pragma once

#define HEARTBEAT_OK	1
#define HEARTBEAT_WAIT	 -1

namespace Manager
{
    class ServerManagerDefine
    {
    public:
        static ServerManagerDefine& Instance()
        {
            static ServerManagerDefine instance;
            return instance;
        }

        DWORD GetHeartBeatInterval() const { return _interval; }
        DWORD GetHeartBeatTimeout() const { return _timeout; }
        DWORD GetHeartBeatMaxCount() const { return _maxCount; }

        void SetHeartBeatInterval(DWORD interval) { _interval = interval; }
        void SetHeartBeatTimeout(DWORD timeout) { _timeout = timeout; }
        void SetHeartBeatMaxCount(DWORD maxCount) { _maxCount = maxCount; }

    private:
        // ���� �� ���� ����
        ServerManagerDefine() {}
        ServerManagerDefine(const ServerManagerDefine&) = delete;
        ServerManagerDefine& operator=(const ServerManagerDefine&) = delete;

    private:
        DWORD _interval;
        DWORD _timeout;
        DWORD _maxCount;
    };
}