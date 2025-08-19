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

    private ConcurrentDictionary<string, Player> _players;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    void Start()
    {
        _networkBase = new NetworkBase();
        int result = _networkBase.Construct();

        _messageModule = new MessageReadModule();
        _messageModule.Initialize(this);

        _receiveMessageQueue = new ConcurrentQueue<Network.MessageData>();
        _players = new ConcurrentDictionary<string, Player>();
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

        if(Input.GetKeyDown(KeyCode.E))
        {
            _currentPlayer.Regist();
        }
    }

    private void CreatePlayer()
    {
        if(_currentPlayer != null)
        {
            Debug.LogWarning("Player already exists. Destroying the current player before creating a new one.");
            DestroyPlayer();
        }

        _currentPlayer = new Player();
        string tmpID = "TESTER";
        _currentPlayer.Initialize(tmpID, _networkBase, _receiveMessageQueue);
        _currentPlayer.StartReceiveThraed();
        bool feedback = _players.TryAdd(tmpID, _currentPlayer);
        if(feedback)
        {
            Debug.Log($"Player created with ID: {_currentPlayer.GetPlayerId()}");
        }
        else
        {
            Debug.Log("Player created Fail");
        }
    }

    private void DestroyPlayer()
    {
        string playerId = _currentPlayer.GetPlayerId();
        if(_players.TryRemove(playerId, out var player))
        {
            player.Deinitialize();

            //����� �̱��÷��̾ �����ϰ��־� ���ڿ�������κ�.
            _currentPlayer = null;
            Debug.Log($"Player with ID: {playerId} destroyed.");
        }
    }

    public Player FindPlayerByID(string id)
    {
        if(_players.TryGetValue(id, out var player))
        {
            return player;
        }

        return null;
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
