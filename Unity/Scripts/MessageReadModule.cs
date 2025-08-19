using System;
using System.Collections;
using System.Collections.Generic;
using Google.FlatBuffers;
using Network;
using UnityEngine;

public interface IMessageReadHandler
{
    void HandleMessage(MessageReadModule module, Network.MessageData messageData);

}

// 의존성주입.  다른곳에서도 쓰고싶다면 상속/인터페이스로 바꾸는것도좋음
public class MessageReadModule 
{
    public PlayerManager PlayerManager;

    private Dictionary<protocol.MESSAGETYPE, IMessageReadHandler> _dicMessageHandler = new();

    public void Initialize(PlayerManager playerManager)
    {
        PlayerManager = playerManager;

        _dicMessageHandler.Add(protocol.MESSAGETYPE.RESPONSE_REGISTER, new RESPONSE_REGISTER());
        _dicMessageHandler.Add(protocol.MESSAGETYPE.REQUEST_HEARTBEAT, new REQUEST_HEARTBEAT());
    }

    public void Process(Network.MessageData messageData)
    {
        if (_dicMessageHandler.TryGetValue((protocol.MESSAGETYPE)messageData.Header.ContentsType, out var handler))
        {
            handler.HandleMessage(this, messageData);
        }
        else
        {
            Debug.LogWarning($"No handler found for message type: {messageData.Header.ContentsType}");
        }
    }
}

public class RESPONSE_REGISTER : IMessageReadHandler
{
    public void HandleMessage(MessageReadModule module, Network.MessageData messageData)
    {
        var message = protocol.RESPONSE_REGISTER.GetRootAsRESPONSE_REGISTER(messageData.Body);

        string playerID = message.PlayerId;
        bool feedback = message.Feedback;

        Player targetPlayer = module.PlayerManager.FindPlayerByID(playerID);
        if (targetPlayer == null)
            return;


        Debug.Log($"RESPONSE_REGISTER: Feedback = {message.Feedback}");
    }
}

public class REQUEST_HEARTBEAT : IMessageReadHandler
{
    public void HandleMessage(MessageReadModule module, Network.MessageData messageData)
    {
        var message = protocol.REQUEST_HEARTBEAT.GetRootAsREQUEST_HEARTBEAT(messageData.Body);

        string playerID = message.PlayerId;

        Player targetPlayer = module.PlayerManager.FindPlayerByID(playerID);
        if (targetPlayer == null)
            return;

        targetPlayer.ResponseHeartBeat();
        Debug.Log($"REQUEST_HEARTBEAT");
    }
}