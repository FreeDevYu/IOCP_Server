using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Network
{
    public class MessageBuilder
    {
        private readonly int _messageHeadrSize;

        private int _messageBufferLength;
        private byte[] _messageBuffer;
        private int _currentIndex;
 
        public MessageBuilder()
        {
            _messageBufferLength = 0;
            _messageBuffer = null;
            _currentIndex = 0;
            _messageHeadrSize = Marshal.SizeOf<MessageHeader>();
        }

        public void Initialize(int bufferSize)
        {
            _messageBufferLength = bufferSize;
            _messageBuffer = new byte[_messageBufferLength];
            _currentIndex = 0;
        }

        // _messageBuffer �����۾��� ���Ͼ����忡�� �����ϰ� �ֱ� ������ ������ �������� ������� �ʽ��ϴ�.
        public void InsertByte(byte value)
        {
            _messageBuffer[_currentIndex++] = value;
        }

        public void InsertMessage(byte[] buffer, int offset, int count)
        {
            if (_messageBuffer == null)
            {
                _messageBuffer = new byte[_messageBufferLength];
            }

            Array.Copy(buffer, offset, _messageBuffer, _currentIndex, count);
            _currentIndex += count;
        }

        public bool PopCompleteMessage(out byte[] completeMessage)
        {
            if (_currentIndex == 0 || _currentIndex < _messageHeadrSize)
            {
                completeMessage = null;
                return false;
            }

            MessageHeader header = new MessageHeader(_messageBuffer.AsSpan(0, _messageHeadrSize));
            int totalMessageSize = _messageHeadrSize + (int)header.BodySize;
            if (_currentIndex < totalMessageSize)
            {
                completeMessage = null; // Not enough data for a complete message
                return false;
            }

            completeMessage = new byte[totalMessageSize]; //gc �ڵ�����... c++�� �ٸ���
            System.Array.Copy(_messageBuffer, 0, completeMessage, 0, totalMessageSize);


           //byte[] bodyMessage = new byte[header.BodySize];
           //System.Array.Copy(_messageBuffer, _messageHeadrSize, bodyMessage, 0, header.BodySize);

            // Reset for next message
            System.Array.Copy(_messageBuffer, totalMessageSize, _messageBuffer, 0, _currentIndex - totalMessageSize);
            _currentIndex -= totalMessageSize;

            return true;
        }
    }
}