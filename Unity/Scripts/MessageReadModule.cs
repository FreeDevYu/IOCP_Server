using System.Collections.Generic;
using UnityEngine;

public interface IMessageReadHandler
{
    void HandleMessage(GameManager gameManager, Network.MessageData messageData);

}

// 의존성주입.  다른곳에서도 쓰고싶다면 상속/인터페이스로 바꾸는것도좋음
public class MessageReadModule 
{
    private GameManager _gameManager;

    private Dictionary<protocol.MESSAGETYPE, IMessageReadHandler> _dicMessageHandler = new();

    public void Initialize(GameManager playerManager)
    {
        _gameManager = playerManager;

        _dicMessageHandler.Add(protocol.MESSAGETYPE.RESPONSE_REGISTER, new RESPONSE_REGISTER());
        _dicMessageHandler.Add(protocol.MESSAGETYPE.REQUEST_HEARTBEAT, new REQUEST_HEARTBEAT());
        _dicMessageHandler.Add(protocol.MESSAGETYPE.NOTICE_ENTRANCE_STAGE, new NOTICE_ENTRANCE_STAGE());
        _dicMessageHandler.Add(protocol.MESSAGETYPE.NOTICE_EXIT_STAGE, new NOTICE_EXIT_STAGE());
        _dicMessageHandler.Add(protocol.MESSAGETYPE.NOTICE_PLAYERPOSITION, new NOTICE_PLAYERPOSITION());
    }

    public void Process(Network.MessageData messageData)
    {
        if (_dicMessageHandler.TryGetValue((protocol.MESSAGETYPE)messageData.Header.ContentsType, out var handler))
        {
            handler.HandleMessage(_gameManager, messageData);
        }
        else
        {
            Debug.LogWarning($"No handler found for message type: {messageData.Header.ContentsType}");
        }
    }
}

public class RESPONSE_REGISTER : IMessageReadHandler
{
    public void HandleMessage(GameManager gameManager, Network.MessageData messageData)
    {
        var message = protocol.RESPONSE_REGISTER.GetRootAsRESPONSE_REGISTER(messageData.Body);

        string playerID = message.PlayerId;
        bool feedback = message.Feedback;

        gameManager.MyAccount.ResponseRegist(feedback);
       
        Debug.Log($"RESPONSE_REGISTER: Feedback = {message.Feedback}");
    }
}

public class NOTICE_ENTRANCE_STAGE : IMessageReadHandler
{
    public void HandleMessage(GameManager gameManager, Network.MessageData messageData)
    {
        var message = protocol.NOTICE_ENTRANCE_STAGE.GetRootAsNOTICE_ENTRANCE_STAGE(messageData.Body);
        string playerID = message.PlayerId;
        float posX = message.PositionX;
        float posY = message.PositionY;
        float posZ = message.PositionZ;

        gameManager.EnterStage(playerID, posX, posY, posZ);
        Debug.Log($"NOTICE_ENTRANCE_STAGE: PlayerID = {playerID}");
    }
}

public class NOTICE_EXIT_STAGE : IMessageReadHandler
{
    public void HandleMessage(GameManager gameManager, Network.MessageData messageData)
    {
        var message = protocol.NOTICE_EXIT_STAGE.GetRootAsNOTICE_EXIT_STAGE(messageData.Body);
        string playerID = message.PlayerId;
        gameManager.StageController.RemovePlayerCharacter(playerID);
        Debug.Log($"NOTICE_EXIT_STAGE: PlayerID = {playerID}");
    }
}
public class REQUEST_HEARTBEAT : IMessageReadHandler
{
    public void HandleMessage(GameManager gameManager, Network.MessageData messageData)
    {
        var message = protocol.REQUEST_HEARTBEAT.GetRootAsREQUEST_HEARTBEAT(messageData.Body);

        string playerID = message.PlayerId;

        gameManager.MyAccount.ResponseHeartBeat();
        Debug.Log($"REQUEST_HEARTBEAT");
    }
}

public class NOTICE_PLAYERPOSITION : IMessageReadHandler
{
    public void HandleMessage(GameManager gameManager, Network.MessageData messageData)
    {
        var message = protocol.NOTICE_PLAYERPOSITION.GetRootAsNOTICE_PLAYERPOSITION(messageData.Body);
        string playerID = message.PlayerId;
        protocol.MoveDirection direction = message.Direction;
        float speed = message.MoveSpeed;
        float duration = message.Duration;

        float posX = message.PositionX;
        float posY = message.PositionY;
        float posZ = message.PositionZ;

        gameManager.StageController.UpdatePlayerPosition(direction, speed, duration, playerID, posX, posY, posZ);
        Debug.Log($"NOTICE_PLAYERPOSITION: PlayerID = {playerID}, Position = ({posX}, {posY}, {posZ})");
    }
}