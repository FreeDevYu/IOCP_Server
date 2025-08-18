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
        상속 / 인터페이스 구현 필요 없음
        런타임에 수정되지 않기 떄문에 데이터무결성 유지 가능

        -> 정적 클래스로 구현
         */


        public static readonly string NETWORK_IP = "127.0.0.1";
        public static readonly int NETWORK_PORT = 9091;
        public static readonly int NETWORK_BUFFER_SIZE = 2048;
        public static readonly int NETWORK_HEADER_SIZE = 4; // 메시지 길이를 나타내는 헤더 크기 (int형)

        public static readonly int MAX_PLAYER_COUNT = 1; // 최대 플레이어 수

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
            //- Skip().Take().ToArray()는 LINQ를 통해 IEnumerable을 생성하고 복사하므로 약간의 오버헤드가 있다.
            Body = new ByteBuffer(tempBody);

        }
    }
}
