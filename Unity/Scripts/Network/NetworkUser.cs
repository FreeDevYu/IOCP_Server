using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using Network;
using UnityEngine;

namespace Network
{
    public class NetworkUser
    {
        //External
        private NetworkBase _networkBase;


        //Internal
        private bool _connected;

        private TcpClient _tcpClient;
        private NetworkStream _networkStream;
        private MessageBuilder _messageBuilder;
        private System.Threading.Thread _receiveThread;

        private byte[] _receiveBuffer;


        public void Initialize(NetworkBase networkBase)
        {
            _connected = false;

            _networkBase = networkBase;
            if (_networkBase == null)
            {
                Debug.LogError("NetworkBase is not initialized.");
                return;
            }

            int success = _networkBase.Construct(ref _tcpClient);
            if (success == NetworkBase.NETWORK_ERROR)
            {
                Debug.LogError("Failed to construct network client.");
                return;
            }

            _messageBuilder = new MessageBuilder();
            _messageBuilder.Initialize(NetworkBase.NETWORK_BUFFER_SIZE);
            _networkStream = _tcpClient.GetStream();
            _connected = true;

            _receiveBuffer = new byte[NetworkBase.NETWORK_BUFFER_SIZE];

            Debug.Log("Network connection established.");
        }

        public void Disconnect()
        {
            _connected = false;

            if (_receiveThread != null && _receiveThread.IsAlive)
            {
                _receiveThread.Join(); // 쓰레드 종료 대기
            }

            _networkStream?.Close();
            _tcpClient?.Close();
            Debug.Log("Disconnected from server.");
        }

        public void StartReceiveThread()
        {
            if (_networkStream == null || !_connected)
            {
                Debug.LogError("Network stream is not initialized or not connected.");
                return;
            }

            // Start a new thread to handle receiving messages
            _receiveThread = new System.Threading.Thread(() =>
            {
                ReceiveThread();
            });

            _receiveThread.IsBackground = true; // Set as background thread
            _receiveThread.Start();
        }

        private void ReceiveThread()
        {
            byte[] completeMessage;
            int result = 0;

            while (_connected)
            {
                result = _networkBase.Receive(_networkStream, _messageBuilder, _receiveBuffer);

                if (result == NetworkBase.NETWORK_ERROR)
                {
                    Debug.LogError("Failed to receive data from server.");
                    break;
                }

                while (_messageBuilder.PopCompleteMessage(out completeMessage))
                {
                    // 메시지 처리
                }
            }
        }
    }
}