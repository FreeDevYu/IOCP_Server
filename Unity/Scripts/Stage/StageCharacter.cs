using System.Collections.Concurrent;
using Unity.VisualScripting;
using UnityEngine;

public class StageCharacter : MonoBehaviour
{
    [SerializeField]
    private GameObject _gameObject;
    [SerializeField]
    private Transform _body;

    [SerializeField]
    private float _rotationSpeed = 2f;

    private float _moveSpeed = 5f;
    private string _playerID;

    private ConcurrentQueue<StageController.MovePacket> _movePacketQueue;

    public void Initialized(string playerID, float x, float y, float z)
    {
        _movePacketQueue = new ConcurrentQueue<StageController.MovePacket>();
        _gameObject.transform.position = new Vector3(x, y, z);
        _playerID = playerID;
    }

    public void Deinitialized()
    {

    }

    public string PlayerID { get { return _playerID; } }
    public float MoveSpeed { get { return _moveSpeed; } }
    private bool _isMoving = false;

    public void EnqueueMovePacket(StageController.MovePacket movePacket)
    {
        _movePacketQueue.Enqueue(movePacket);
    }

    public void MoveUpdate()
    {
        if (_isMoving)
            return;

        while( _movePacketQueue.Count > 0 )
        {
            _isMoving = true;
            if(_movePacketQueue.TryDequeue(out StageController.MovePacket movePacket))
            {
                Vector3 direction = Vector3.zero;
                switch (movePacket.Direction)
                {
                    case protocol.MoveDirection.FORWARD:
                        direction = Vector3.forward;
                        break;
                    case protocol.MoveDirection.BACKWARD:
                        direction = Vector3.back;
                        break;
                    case protocol.MoveDirection.LEFT:
                        direction = Vector3.left;
                        break;
                    case protocol.MoveDirection.RIGHT:
                        direction = Vector3.right;
                        break;
                    case protocol.MoveDirection.LEFT_FORWARD:
                        direction = (Vector3.left + Vector3.forward).normalized;
                        break;
                    case protocol.MoveDirection.RIGHT_FORWARD:
                        direction = (Vector3.right + Vector3.forward).normalized;
                        break;
                    case protocol.MoveDirection.LEFT_BACKWARD:
                        direction = (Vector3.left + Vector3.back).normalized;
                        break;
                    case protocol.MoveDirection.RIGHT_BACKWARD:
                        direction = (Vector3.right + Vector3.back).normalized;
                        break;
                    case protocol.MoveDirection.NONE:
                    default:
                        direction = Vector3.zero;
                        break;
                }
                if (direction != Vector3.zero)
                {
                    // 회전
                    Quaternion targetRotation = Quaternion.LookRotation(direction);
                   // _body.rotation = Quaternion.Slerp(_body.rotation, targetRotation, _rotationSpeed * Time.deltaTime);
                    _body.rotation = targetRotation;
                    // 이동
                    _gameObject.transform.Translate(direction * movePacket.MoveSpeed * movePacket.MoveDuration, Space.World);
                }
            }
        }

        _isMoving = false;
    }
    //ivate void Start()
    //
    //
    //
    //
    //ivate void Update()
    //
    //  // W키: 앞으로 이동
    //  if (Input.GetKey(KeyCode.W))
    //  {
    //      transform.Translate(Vector3.forward * _moveSpeed * Time.deltaTime);
    //  }
    //
    //  // D키: 오른쪽으로 회전
    //  if (Input.GetKey(KeyCode.D))
    //  {
    //      transform.Rotate(Vector3.up * _rotationSpeed * Time.deltaTime);
    //  }
    //
    //  // A키: 왼쪽으로 회전 (선택사항)
    //  if (Input.GetKey(KeyCode.A))
    //  {
    //      transform.Rotate(Vector3.up * -_rotationSpeed * Time.deltaTime);
    //  }


}
