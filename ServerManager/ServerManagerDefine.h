#pragma once

#define HEARTBEAT_OK	1
#define HEARTBEAT_WAIT	 -1

#define TIMEUNIT_SECOND	1000
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
		DWORD GetRegisterWaitTime() const { return _registerWaitTime; }

        void SetHeartBeatInterval(DWORD interval) { _interval = interval * TIMEUNIT_SECOND; }
        void SetHeartBeatTimeout(DWORD timeout) { _timeout = timeout * TIMEUNIT_SECOND; }
        void SetHeartBeatMaxCount(DWORD maxCount) { _maxCount = maxCount; }
		void SetRegisterWaitTime(DWORD waitTime) { _registerWaitTime = waitTime * TIMEUNIT_SECOND; }

    private:
        // 복사 및 대입 방지
        ServerManagerDefine() {}
        ServerManagerDefine(const ServerManagerDefine&) = delete;
        ServerManagerDefine& operator=(const ServerManagerDefine&) = delete;

    private:
        DWORD _interval;
        DWORD _timeout;
        DWORD _maxCount;

		DWORD _registerWaitTime;
    };
}