using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace Network
{
    public class MessageBuilder
    {
        //const
        private int _messageBufferLength;
        private int _messageHeadrSize;

        private byte[] _messageBuffer;
        private int _currentIndex;
 

        public void Initialize(int bufferSize)
        {
            _messageBufferLength = bufferSize;
            _messageBuffer = new byte[_messageBufferLength];
            _currentIndex = 0;
            _messageHeadrSize = Marshal.SizeOf<MessageHeader>();
        }

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

            MessageHeader header = MessageHeader.FromBytes(_messageBuffer.AsSpan(0, _messageHeadrSize));
            int totalMessageSize = _messageHeadrSize + (int)header.BodySize;
            if (_currentIndex < totalMessageSize)
            {
                completeMessage = null; // Not enough data for a complete message
                return false;
            }

            completeMessage = new byte[totalMessageSize]; //gc 자동해제.
            System.Array.Copy(_messageBuffer, 0, completeMessage, 0, totalMessageSize);

            // Reset for next message
            System.Array.Copy(_messageBuffer, totalMessageSize, _messageBuffer, 0, _currentIndex - totalMessageSize);
            _currentIndex -= totalMessageSize;

            return true;
        }
    }
}