using System.Collections.Concurrent;
using Network;
using UnityEngine;

public class GameManager : MonoBehaviour
{
    public Account MyAccount;
    public StageController StageController;

    private NetworkUser _networkUser;
    public NetworkUser GetNetworkUser { get { return _networkUser; } }

    private NetworkBase _networkBase;
    private MessageReadModule _messageModule;


    private ConcurrentQueue<Network.MessageData> _receiveMessageQueue;

    private System.Action _updateAction;

    void Start()
    {
        _networkBase = new NetworkBase();
        int result = _networkBase.Construct();

        _networkUser = new Network.NetworkUser();
        int feedback = _networkUser.Initialize(_networkBase, Network.NetworkDefine.NETWORK_IP, Network.NetworkDefine.NETWORK_PORT);

        if (feedback == Network.NetworkDefine.NETWORK_ERROR)
        {
            Debug.LogError("Failed to initialize network user.");
            return;
        }

        _messageModule = new MessageReadModule();
        _messageModule.Initialize(this);

        _receiveMessageQueue = new ConcurrentQueue<Network.MessageData>();

        StageController= new StageController();
        StageController.Construct(this);
        _updateAction += StageController.UpdateLogic;

        CreateAccount();
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

        if (Input.GetKeyDown(KeyCode.Escape))
        {
            DestroyAccount();
        }

        _updateAction?.Invoke();
    }

    private void CreateAccount()
    {
        if(MyAccount != null)
        {
            Debug.LogWarning("Player already exists. Destroying the current player before creating a new one.");
            DestroyAccount();
        }

        MyAccount = new Account();
        string tmpID = "TESTER" + Random.Range(1,100);
        MyAccount.Initialize(tmpID, _networkBase, _networkUser, _receiveMessageQueue);
        MyAccount.StartReceiveThraed();
        Debug.Log($"Player created with ID: {MyAccount.GetPlayerId()}");

        MyAccount.RequestRegist();
    }

    private void DestroyAccount()
    {
        if(MyAccount == null)
        {
            Debug.LogWarning("No player to destroy.");
            return;
        }
        string playerID = MyAccount.GetPlayerId();
        MyAccount.Deinitialize();
        MyAccount = null;
        Debug.Log($"Player with ID: {playerID} destroyed.");
    }

    public void EnterStage(string playerID, float positionX, float positionY, float positionZ)
    {
        if (MyAccount != null && playerID == MyAccount.GetPlayerId())
        {
           StageController.CreatePlayerCharacter(playerID, positionX, positionY, positionZ, true);
        }
        else
        {
            StageController.CreatePlayerCharacter(playerID, positionX, positionY, positionZ, false);
        }
    }
    
    public void ResponseHeartBeat()
    {
        MyAccount.ResponseHeartBeat();
    }
   
    // public Account FindPlayerByID(string id)
    // {
    //     if(_players.TryGetValue(id, out var player))
    //     {
    //         return player;
    //     }
    //
    //     return null;
    // }
    //

    // public void CreatePlayerCharacter(string playerID)
    // {
    //     Account targetPlayer = FindPlayerByID(playerID);
    //     if (targetPlayer == null)
    //         return;
    //
    //     _stageController.CreatePlayerCharacter(playerID);
    // }

    //초기화없이 강제종료시 c#의 쓰레드가 남는 문제가 있어서, 안정적 초기화를 위한 코드.
    void OnApplicationQuit()
    {
        if (MyAccount != null)
        {
            MyAccount.Deinitialize();
            Debug.Log($"Current player with ID: {MyAccount.GetPlayerId()} destroyed.");
        }
    }
}
