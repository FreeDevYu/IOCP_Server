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

        _dicMessageHandler.Add(protocol.MESSAGETYPE.REQUEST_REGISTER, new RESPONSE_REGISTER());
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

        Debug.Log($"RESPONSE_REGISTER: Feedback = {message.Feedback}");
    }
}