using System.Collections.Concurrent;
using System.Collections.Generic;
using UnityEngine;

public class StageController
{
    public struct MovePacket
    {
        public string PlayerID;
        public protocol.MoveDirection Direction;
        public float MoveSpeed;
        public float MoveDuration;
        public float PosX;
        public float PosY;
        public float PosZ;
    }

    private GameManager _gameManager;
    private Dictionary<string, StageCharacter> _stageCharacters;
    private StageCharacter _myStageCharacter;
    private GameObject _characterPrefab;
    private ConcurrentQueue<MovePacket> _movePacketQueue;

    public void Construct(GameManager gameManager)
    {
        _gameManager = gameManager;
        _stageCharacters = new Dictionary<string, StageCharacter>();
        _movePacketQueue = new ConcurrentQueue<MovePacket>();
        _characterPrefab = Resources.Load<GameObject>("Prefab/Character");
    }

    public void Destruct()
    {
        _gameManager = null;
        _stageCharacters.Clear();
        _stageCharacters = null;
        _myStageCharacter = null;
    }

    public void CreatePlayerCharacter(string playerID, float positionX, float positionY, float positionZ, bool isMyID = false)
    {
        if(_stageCharacters.ContainsKey(playerID))
        {
            Debug.Log($"Player Character Create Fail_DuplicateID : {playerID}");
        }

        GameObject characterPrefab = GameObject.Instantiate(_characterPrefab);
        StageCharacter stageCharacter = characterPrefab.GetComponent<StageCharacter>();// 실제로는 케릭터에서 모델링key를 받아서 getcomponent할 생각중.
        stageCharacter.Initialized(playerID, positionX, positionY, positionZ);
        _stageCharacters.Add(playerID, stageCharacter);

        if (isMyID)
        {
            _myStageCharacter = stageCharacter;
        }

        Debug.Log($"Player Character Create Success : {playerID}");
    }

    public void RemovePlayerCharacter(string playerID)
    {
        if (!_stageCharacters.TryGetValue(playerID, out StageCharacter character))
        {
            Debug.Log($"Player Character Remove Fail_NotExistID : {playerID}");
            return;
        }
        character.Deinitialized();
        GameObject.Destroy(character.gameObject);
        _stageCharacters.Remove(playerID);
        if (_myStageCharacter == character)
            _myStageCharacter = null;
        Debug.Log($"Player Character Remove Success : {playerID}");
    }

    public void UpdatePlayerPosition(
        protocol.MoveDirection direction,
        float moveSpeed,
        float moveDuration,
        string playerID, float posX, float posY, float posZ)
    {
        if (!_stageCharacters.TryGetValue(playerID, out StageCharacter character))
        {
            Debug.Log($"Player Character Not Exist : {playerID}");
            return;
        }

        MovePacket movePacket = new MovePacket
        {
            PlayerID = playerID,
            Direction = direction,
            MoveSpeed = moveSpeed,
            MoveDuration = moveDuration,
            PosX = posX,
            PosY = posY,
            PosZ = posZ
        };
        _movePacketQueue.Enqueue(movePacket);
    }

    public void UpdateLogic()
    {
        if(_myStageCharacter == null)
            return;

        InputMyMove();
        UpdateCharacterPositions();
    }

    private float _inputKeyDuration = 0f;
    private protocol.MoveDirection _inputMoveDirection = protocol.MoveDirection.NONE;

    private void InputMyMove()
    {
        bool w = Input.GetKey(KeyCode.W);
        bool a = Input.GetKey(KeyCode.A);
        bool s = Input.GetKey(KeyCode.S);
        bool d = Input.GetKey(KeyCode.D);

        if (w || a || s || d)
        {
            _inputKeyDuration += Time.deltaTime;

            if (w && a)
                _inputMoveDirection = protocol.MoveDirection.LEFT_FORWARD;
            else if (w && d)
                _inputMoveDirection = protocol.MoveDirection.RIGHT_FORWARD;
            else if (s && a)
                _inputMoveDirection = protocol.MoveDirection.LEFT_BACKWARD;
            else if (s && d)
                _inputMoveDirection = protocol.MoveDirection.RIGHT_BACKWARD;
            else if (w)
                _inputMoveDirection = protocol.MoveDirection.FORWARD;
            else if (s)
                _inputMoveDirection = protocol.MoveDirection.BACKWARD;
            else if (a)
                _inputMoveDirection = protocol.MoveDirection.LEFT;
            else if (d)
                _inputMoveDirection = protocol.MoveDirection.RIGHT;

            TrySendMove();
        }
        else
        {
            _inputKeyDuration = 0f;
            _inputMoveDirection = protocol.MoveDirection.NONE;
        }
    }
 
    private void TrySendMove()
    {
        if (_inputKeyDuration > StaticValue.StageInputKeyMinDuration &&
            _inputKeyDuration <= StaticValue.StageInputKeyMaxDuration)
        {
            // FlatBufferBuilder 생성부분 고민필요.
            var builder = new Google.FlatBuffers.FlatBufferBuilder(1024);
            var networkUser = _gameManager.GetNetworkUser;
            MessageSendModule.Instance.REQUEST_PLAYERMOVE(builder, networkUser, _myStageCharacter.PlayerID, _inputMoveDirection, _myStageCharacter.MoveSpeed, _inputKeyDuration);
            _inputKeyDuration = 0;
        }
    }

    private void UpdateCharacterPositions()
    {
        while (_movePacketQueue.TryDequeue(out MovePacket movePacket))
        {
            if (!_stageCharacters.TryGetValue(movePacket.PlayerID, out StageCharacter character))
            {
                Debug.Log($"Player Character Not Exist : {movePacket.PlayerID}");
                return;
            }
            character.EnqueueMovePacket(movePacket);
            character.MoveUpdate();
        }
    }
}
