using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Google.FlatBuffers;
using UnityEngine;

namespace Network
{
    public static class NetworkDefine
    {

        /*
        ��� / �������̽� ���� �ʿ� ����
        ��Ÿ�ӿ� �������� �ʱ� ������ �����͹��Ἲ ���� ����

        -> ���� Ŭ������ ����
         */


        public static readonly string NETWORK_IP = "127.0.0.1";
        public static readonly int NETWORK_PORT = 9091;
        public static readonly int NETWORK_BUFFER_SIZE = 2048;
        public static readonly int NETWORK_HEADER_SIZE = 4; // �޽��� ���̸� ��Ÿ���� ��� ũ�� (int��)

        public static readonly int MAX_PLAYER_COUNT = 1; // �ִ� �÷��̾� ��

        public static readonly int NETWORK_OK = 1;
        public static readonly int NETWORK_ERROR = -1;
    }

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

    public class MessageData
    {
        public UInt32 PlayerId;
        public MessageHeader Header;
        public ByteBuffer Body;

       public MessageData(UInt32 PlayerId, byte[] completedMessage)
        {
            if (completedMessage == null || completedMessage.Length < NetworkDefine.NETWORK_HEADER_SIZE)
            {
                throw new ArgumentException("Invalid message data.");
            }

            Header = MessageHeader.FromBytes(completedMessage.AsSpan(0, NetworkDefine.NETWORK_HEADER_SIZE));

            if (completedMessage.Length < NetworkDefine.NETWORK_HEADER_SIZE + Header.BodySize)
            {
                throw new ArgumentException("Incomplete message data.");
            }

            this.PlayerId = PlayerId;

            byte[] tempBody = new byte[Header.BodySize];
            Array.Copy(completedMessage, NetworkDefine.NETWORK_HEADER_SIZE, tempBody, 0, Header.BodySize);
            //- Skip().Take().ToArray()�� LINQ�� ���� IEnumerable�� �����ϰ� �����ϹǷ� �ణ�� ������尡 �ִ�.
            Body = new ByteBuffer(tempBody);

        }
    }
}
