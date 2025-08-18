using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using Network;
using UnityEngine;

public class Player 
{
    //external
    private Network.NetworkBase _networkBase;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    //internal
    private Network.NetworkUser _networkUser;


    //Player Data
    private UInt32 _playerId;
    private bool _isRegistered;

    public Player()
    {
        _networkBase = null;
        _receiveMessageQueue = null;
        _networkUser = null;
        _playerId = 0;
        _isRegistered = false;
    }

    public int Initialize(UInt32 playerId, Network.NetworkBase networkBase, ConcurrentQueue<Network.MessageData> receiveMessageQueue)
    {
        _playerId = playerId;

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

    public void SendMessage(string message)
    {
        _networkUser.SendMessage(message);
    }

    public UInt32 GetPlayerId()
    {
        return _playerId;
    }
}