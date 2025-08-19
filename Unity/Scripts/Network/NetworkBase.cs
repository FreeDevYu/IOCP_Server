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
                Debug.Log("������ ����Ǿ����ϴ�: " + ip + ":" + port);
                return Network.NetworkDefine.NETWORK_OK;
            }
            catch (SocketException e)
            {
                Debug.LogError("���� ���� ����: " + e.Message);
                return Network.NetworkDefine.NETWORK_ERROR;
            }
        }

        public int SendMessageToServer(ref NetworkStream stream, byte[] messageBytes)
        {
            if (stream == null) 
                return Network.NetworkDefine.NETWORK_ERROR;

            try
            {
                stream.Write(messageBytes, 0, messageBytes.Length);
                stream.Flush();
                Debug.Log("�޽��� ���۵�");
                return Network.NetworkDefine.NETWORK_OK;
            }
            catch (IOException ioEx)
            {
                Debug.LogError($"��Ʈ��ũ ����: {ioEx.Message}");
                return Network.NetworkDefine.NETWORK_ERROR;
            }
            catch (Exception ex)
            {
                Debug.LogError($"�� �� ���� ����: {ex.Message}");
                return Network.NetworkDefine.NETWORK_ERROR;
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
                    return Network.NetworkDefine.NETWORK_OK;
                }

                return Network.NetworkDefine.NETWORK_ERROR;
            }
            catch (IOException ex)
            {
                // ��Ʈ��ũ ���� ó��
                Console.WriteLine("ReceiveThread error: " + ex.Message);
                return Network.NetworkDefine.NETWORK_ERROR;
            }
        }
    }
}