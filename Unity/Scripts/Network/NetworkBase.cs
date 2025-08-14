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

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct MessageHeader
    {
        public uint BodySize;
        public uint ContentsType;

        public static MessageHeader FromBytes(ReadOnlySpan<byte> span)
        {
            return MemoryMarshal.Read<MessageHeader>(span);
        }
    }


    public class NetworkBase
    {
        public static readonly string NETWORK_IP = "127.0.0.1";
        public static readonly int NETWORK_PORT = 9001;
        public static readonly int NETWORK_BUFFER_SIZE = 2048;
        public static readonly int NETWORK_HEADER_SIZE = 4; // 메시지 길이를 나타내는 헤더 크기 (int형)

        public static readonly int NETWORK_OK = 1;
        public static readonly int NETWORK_ERROR = -1;


        public int Construct(ref TcpClient client)
        {
            int result = 0;
            result = ConnectToServer(ref client, NETWORK_IP, NETWORK_PORT); // 서버 IP와 포트

            if (result == NETWORK_ERROR)
            {
                Debug.LogError("네트워크 연결에 실패했습니다.");
                return result;
            }

            return result;
        }

        public void SendMessageToServer(ref NetworkStream stream, string message)
        {
            if (stream == null) return;

            byte[] data = Encoding.UTF8.GetBytes(message);
            stream.Write(data, 0, data.Length);
            Debug.Log("메시지 전송됨");
        }

        private int ConnectToServer(ref TcpClient client, string ip, int port)
        {
            try
            {
                client = new TcpClient(ip, port);
                Debug.Log("서버에 연결되었습니다: " + ip + ":" + port);
                return NETWORK_OK;
            }
            catch (SocketException e)
            {
                Debug.LogError("서버 연결 실패: " + e.Message);
                return NETWORK_ERROR;
            }
        }

        public int Receive(NetworkStream stream, MessageBuilder messageBuilder, byte[] receiveBuffer) // client.GetStream
        {
            try
            {
                int bytesRead = stream.Read(receiveBuffer, 0, receiveBuffer.Length);
                if (bytesRead > 0)
                {
                    messageBuilder.InsertMessage(receiveBuffer, 0, bytesRead);
                    return NETWORK_OK;
                }

                return NETWORK_ERROR;
            }
            catch (IOException ex)
            {
                // 네트워크 오류 처리
                Console.WriteLine("ReceiveThread error: " + ex.Message);
                return NETWORK_ERROR;
            }
        }

    }
}