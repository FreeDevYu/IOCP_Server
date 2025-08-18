using System;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using Network;
using UnityEngine;

public class PlayerManager : MonoBehaviour
{
    private NetworkBase _networkBase;
    private Player _currentPlayer;
    private MessageReadModule _messageModule;

    private Queue<UInt32> _playerIdQueue;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    void Start()
    {
        _networkBase = new NetworkBase();
        int result = _networkBase.Construct();

        _messageModule = new MessageReadModule();
        _messageModule.Initialize(this);

        _receiveMessageQueue = new ConcurrentQueue<Network.MessageData>();
        _playerIdQueue = new Queue<UInt32>();
        for (UInt32 i = 1; i <= Network.NetworkDefine.MAX_PLAYER_COUNT; i++)
        {
            _playerIdQueue.Enqueue(i);
        }
    }

    void Update()
    {
        if (_receiveMessageQueue.Count > 0)
        {
            bool feedback = _receiveMessageQueue.TryDequeue(out MessageData messageData);
            if (feedback && messageData != null)
            {
                _messageModule.Process(messageData);
                // �޽��� ó�� ����
                //Debug.Log($"Received message: {messageData.Message}");
            }
        }

        //�����ν����ͷ� ��ư����� ?
        if(Input.GetKeyDown(KeyCode.Q))
        {
            CreatePlayer();
        }

        if (Input.GetKeyDown(KeyCode.W))
        {
            DestroyPlayer();
        }
    }

    private void CreatePlayer()
    {
        if(_currentPlayer != null)
        {
            Debug.LogWarning("Player already exists. Destroying the current player before creating a new one.");
            DestroyPlayer();
        }

        UInt32 playerId = _playerIdQueue.Dequeue();
        _currentPlayer = new Player();
        _currentPlayer.Initialize(playerId, _networkBase, _receiveMessageQueue);
        _currentPlayer.StartReceiveThraed();
        Debug.Log($"Player created with ID: {playerId}");
    }

    private void DestroyPlayer()
    {
        UInt32 playerId = _currentPlayer.GetPlayerId();
        _currentPlayer.Deinitialize();
        _playerIdQueue.Enqueue(playerId);
        _currentPlayer = null;
        Debug.Log($"Player with ID: {playerId} destroyed.");
    }

    //�ʱ�ȭ���� ��������� c#�� �����尡 ���� ������ �־, ������ �ʱ�ȭ�� ���� �ڵ�.
    void OnApplicationQuit()
    {
        if (_currentPlayer != null)
        {
            _currentPlayer.Deinitialize();
            Debug.Log($"Current player with ID: {_currentPlayer.GetPlayerId()} destroyed.");
        }
    }
}
