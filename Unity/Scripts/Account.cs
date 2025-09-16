using System.Collections.Concurrent;
using Google.FlatBuffers;



public class Account 
{
    //external
    private Network.NetworkBase _networkBase;
    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    //internal
    private Network.NetworkUser _networkUser;
  
    //Player Data
    private string _playerId;
    private string _secretKey;
    private bool _isRegistered;

    public Account()
    {
        _networkBase = null;
        _receiveMessageQueue = null;
        _networkUser = null;
        _isRegistered = false;
        _playerId = "";
        _secretKey = "";
    }

    public int Initialize(string playerId, Network.NetworkBase networkBase, Network.NetworkUser networkUser, ConcurrentQueue<Network.MessageData> receiveMessageQueue)
    {
        _networkBase = networkBase;
        _receiveMessageQueue = receiveMessageQueue;
        _networkUser = networkUser;
        _networkUser.ActionReceiveMessage += ReceiveMessage;

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

    public bool IsRegistered()
    {
        return _isRegistered;
    }

    public void RequestRegist()
    {
        FlatBufferBuilder flatBufferBuilder = new FlatBufferBuilder(1024);
        MessageSendModule.Instance.REQUEST_REGISTER(flatBufferBuilder, _networkUser, _playerId, _secretKey);
    }

    public void ResponseRegist(bool feedback)
    {
        _isRegistered = feedback;
    }

    public void ResponseHeartBeat()
    {
        FlatBufferBuilder flatBufferBuilder = new FlatBufferBuilder(1024);
        MessageSendModule.Instance.RESPONSE_HEARTBEAT(flatBufferBuilder, _networkUser, _playerId);
    }
}