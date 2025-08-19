using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using Google.FlatBuffers;
using Network;
using UnityEngine;


public class Player 
{
    //external
    private Network.NetworkBase _networkBase;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    //internal
    private Network.NetworkUser _networkUser;
    private Google.FlatBuffers.FlatBufferBuilder _flatBufferBuilder;

    //Player Data
    private string _playerId;
    private string _secretKey;
    private bool _isRegistered;

    public Player()
    {
        _networkBase = null;
        _receiveMessageQueue = null;
        _networkUser = null;
        _isRegistered = false;
        _playerId = "";
        _secretKey = "";
    }

    public int Initialize(string playerId, Network.NetworkBase networkBase, ConcurrentQueue<Network.MessageData> receiveMessageQueue)
    {
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

        _playerId = playerId;
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
        Network.MessageData messageData = new Network.MessageData(_playerId, completedMessage);
        _receiveMessageQueue.Enqueue(messageData);
    }

    public string GetPlayerId()
    {
        return _playerId;
    }

    public void Regist()
    {
        MessageSendModule.Instance.REQUEST_REGISTER(_flatBufferBuilder, _networkUser, _playerId, _secretKey);
    }

    public void ResponseHeartBeat()
    {
        MessageSendModule.Instance.RESPONSE_HEARTBEAT(_flatBufferBuilder, _networkUser, _playerId);
    }
}