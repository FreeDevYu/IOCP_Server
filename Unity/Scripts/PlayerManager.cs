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
                // 메시지 처리 로직
                //Debug.Log($"Received message: {messageData.Message}");
            }
        }

        //오딘인스펙터로 버튼만들까 ?
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

            //현재는 싱글플레이어만 관리하고있어 부자연스러운부분.
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

    //초기화없이 강제종료시 c#의 쓰레드가 남는 문제가 있어서, 안정적 초기화를 위한 코드.
    void OnApplicationQuit()
    {
        if (_currentPlayer != null)
        {
            _currentPlayer.Deinitialize();
            Debug.Log($"Current player with ID: {_currentPlayer.GetPlayerId()} destroyed.");
        }
    }
}
