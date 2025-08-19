using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using Google.FlatBuffers;
using Network;
using UnityEngine;
using static UnityEngine.ParticleSystem;

public class Player 
{
    //external
    private Network.NetworkBase _networkBase;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    //internal
    private Network.NetworkUser _networkUser;
    private Google.FlatBuffers.FlatBufferBuilder _flatBufferBuilder;

    //Player Data
    private UInt32 _playerIndex;
    private string _playerId;
    private string _secretKey;
    private bool _isRegistered;

    public Player()
    {
        _networkBase = null;
        _receiveMessageQueue = null;
        _networkUser = null;
        _playerIndex = 0;
        _isRegistered = false;
    }

    public int Initialize(UInt32 playerId, Network.NetworkBase networkBase, ConcurrentQueue<Network.MessageData> receiveMessageQueue)
    {
        _playerIndex = playerId;
        _flatBufferBuilder = new Google.FlatBuffers.FlatBufferBuilder(2048);
        _networkBase = networkBase;
        _receiveMessageQueue = receiveMessageQueue;

        _networkUser = new Network.NetworkUser();
        int feedback = _networkUser.Initialize(_networkBase, Network.NetworkDefine.NETWORK_IP, Network.NetworkDefine.NETWORK_PORT);
        _networkUser.ActionReceiveMessage += ReceiveMessage;

        if (feedback == Network.NetworkDefine.NETWORK_ERROR)
        {
            Debug.LogError("Failed to initialize network user.");
            return Network.NetworkDefine.NETWORK_ERROR;
        }

        _playerId = "Player";
        _secretKey = "SecretKey";
        return Network.NetworkDefine.NETWORK_OK;
    }

    public void Deinitialize()
    {
        if (_networkUser != null)
        {
            _networkUser.ActionReceiveMessage -= ReceiveMessage;
            _networkUser.Disconnect();
            _networkUser = null;
        }

        _networkBase = null;
        _receiveMessageQueue = null;
        _flatBufferBuilder = null;
    }

    public void StartReceiveThraed()
    {
        _networkUser.StartReceiveThread();
    }

    private void ReceiveMessage(byte[] completedMessage)
    {
        Network.MessageData messageData = new Network.MessageData(_playerIndex, completedMessage);
        _receiveMessageQueue.Enqueue(messageData);
    }

    public void SendMessage(byte[] messageBytes)
    {
        _networkUser.SendMessage(messageBytes);
    }

    public UInt32 GetPlayerId()
    {
        return _playerIndex;
    }

    public void Regist()
    {
        _flatBufferBuilder.Clear();//쓰레드 안전한지 체크 필요

        StringOffset playerIdOffset = _flatBufferBuilder.CreateString(_playerId);
        StringOffset secretKeyOffset = _flatBufferBuilder.CreateString(_secretKey);

        Offset<protocol.REQUEST_REGISTER> data = protocol.REQUEST_REGISTER.CreateREQUEST_REGISTER(_flatBufferBuilder, playerIdOffset, secretKeyOffset);
        _flatBufferBuilder.Finish(data.Value);
        byte[] bodyBytes = _flatBufferBuilder.SizedByteArray();

        Network.MessageHeader messageHeader = new Network.MessageHeader((uint)bodyBytes.Length, (uint)protocol.MESSAGETYPE.REQUEST_REGISTER);
        byte[] headerBytes = messageHeader.ToBytes();

        byte[] messageBytes = new byte[bodyBytes.Length + Network.NetworkDefine.NETWORK_HEADER_SIZE];
        Buffer.BlockCopy(headerBytes, 0, messageBytes, 0, Network.NetworkDefine.NETWORK_HEADER_SIZE);
        Buffer.BlockCopy(bodyBytes, 0, messageBytes, Network.NetworkDefine.NETWORK_HEADER_SIZE, bodyBytes.Length);

        SendMessage(messageBytes);

    }
}