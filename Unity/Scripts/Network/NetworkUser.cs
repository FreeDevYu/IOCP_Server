using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using Network;
using UnityEditor.PackageManager;
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


        public System.Action<byte[]> ActionReceiveMessage;

        public int Initialize(NetworkBase networkBase, string ip, int port)
        {
            _connected = false;
            ActionReceiveMessage = null;

            _networkBase = networkBase;
            if (_networkBase == null)
            {
                Debug.LogError("NetworkBase is not initialized.");
                return Network.NetworkDefine.NETWORK_ERROR;
            }

            int success = _networkBase.ConnectToServer(ref _tcpClient, ip, port);
            if (success == Network.NetworkDefine.NETWORK_ERROR)
            {
                Debug.LogError("Failed to construct network client.");
                return Network.NetworkDefine.NETWORK_ERROR;
            }

            _messageBuilder = new MessageBuilder();
            _messageBuilder.Initialize(Network.NetworkDefine.NETWORK_BUFFER_SIZE);
            _networkStream = _tcpClient.GetStream();

            _connected = true;

            _receiveBuffer = new byte[Network.NetworkDefine.NETWORK_BUFFER_SIZE];

            Debug.Log("Network connection established.");
            return Network.NetworkDefine.NETWORK_OK;
        }

        public void Disconnect()
        {
            _connected = false;
            ActionReceiveMessage = null;

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

            _networkStream.ReadTimeout = 1000; // 1초
            // Start a new thread to handle receiving messages
            _receiveThread = new System.Threading.Thread(() =>
            {
                ReceiveThread();
            });

            _receiveThread.IsBackground = true; // Set as background thread
            _receiveThread.Start();
        }

        public void SendMessage(byte[] messageBytes)
        {
            if (_networkStream == null || !_connected)
            {
                Debug.LogError("Network stream is not initialized or not connected.");
                return;
            }

            int feedback = _networkBase.SendMessageToServer(ref _networkStream, messageBytes);
            if (feedback == Network.NetworkDefine.NETWORK_ERROR)
            {
                Debug.LogError("Failed to send message to server.");
            }
        }

        private void ReceiveThread()
        {
            byte[] completeMessage;
            int result = 0;

            while (_connected)
            {
                result = _networkBase.Receive(_networkStream, _messageBuilder, _receiveBuffer);

                if (result == Network.NetworkDefine.NETWORK_ERROR)
                {
                    Debug.LogError("Failed to receive data from server.");
                }
                else
                {
                    while (_messageBuilder.PopCompleteMessage(out completeMessage))
                    {
                        // 메시지 처리

                        ActionReceiveMessage?.Invoke(completeMessage);
                        Debug.Log("Received message: " + System.Text.Encoding.UTF8.GetString(completeMessage));
                    }
                }     
            }
        }
    }
}