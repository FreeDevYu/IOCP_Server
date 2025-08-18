using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;


using UnityEngine;


namespace Network
{
    public class NetworkBase
    {
        public int Construct()
        {
            int result = 0;

            return result;
        }

        public int ConnectToServer(ref TcpClient client, string ip, int port)
        {
            try
            {
                client = new TcpClient(ip, port);
                Debug.Log("서버에 연결되었습니다: " + ip + ":" + port);
                return Network.NetworkDefine.NETWORK_OK;
            }
            catch (SocketException e)
            {
                Debug.LogError("서버 연결 실패: " + e.Message);
                return Network.NetworkDefine.NETWORK_ERROR;
            }
        }

        public int SendMessageToServer(ref NetworkStream stream, string message)
        {
            if (stream == null) 
                return Network.NetworkDefine.NETWORK_ERROR;

            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log("메시지 전송됨");
            return Network.NetworkDefine.NETWORK_OK;
        }

        public int Receive(NetworkStream stream, MessageBuilder messageBuilder, byte[] receiveBuffer) // client.GetStream
        {
            try
            {
                int bytesRead = stream.Read(receiveBuffer, 0, receiveBuffer.Length);
                if (bytesRead > 0)
                {
                    messageBuilder.InsertMessage(receiveBuffer, 0, bytesRead);
                    return Network.NetworkDefine.NETWORK_OK;
                }

                return Network.NetworkDefine.NETWORK_ERROR;
            }
            catch (IOException ex)
            {
                // 네트워크 오류 처리
                Console.WriteLine("ReceiveThread error: " + ex.Message);
                return Network.NetworkDefine.NETWORK_ERROR;
            }
        }

    }
}