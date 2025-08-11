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

		int GetOverlappedCount() const { return _overlappedCount; }
		int GetMaxClient() const { return _maxClient; }

		void SetOverlappedCount(int count) { _overlappedCount = count; }
		void SetMaxClient(int maxClient) { _maxClient = maxClient; }


        DWORD GetHeartBeatInterval() const { return _interval; }
        DWORD GetHeartBeatTimeout() const { return _timeout; }
        DWORD GetHeartBeatMaxCount() const { return _maxCount; }
		DWORD GetRegisterWaitTime() const { return _registerWaitTime; }

        void SetHeartBeatInterval(DWORD interval) { _interval = interval * TIMEUNIT_SECOND; }
        void SetHeartBeatTimeout(DWORD timeout) { _timeout = timeout * TIMEUNIT_SECOND; }
        void SetHeartBeatMaxCount(DWORD maxCount) { _maxCount = maxCount; }
		void SetRegisterWaitTime(DWORD waitTime) { _registerWaitTime = waitTime * TIMEUNIT_SECOND; }

    private:
        // ���� �� ���� ����
        ServerManagerDefine() {}
        ServerManagerDefine(const ServerManagerDefine&) = delete;
        ServerManagerDefine& operator=(const ServerManagerDefine&) = delete;

    private:
        int _overlappedCount;
        int _maxClient;

        DWORD _interval;
        DWORD _timeout;
        DWORD _maxCount;

		DWORD _registerWaitTime;
    };
}