using UnityEngine;

public class StageCharacter : MonoBehaviour
{
    [SerializeField]
    private Transform _body;

    [SerializeField]
    private float _rotationSpeed = 2f;

    [SerializeField]
    private float _moveSpeed = 2f;

    private string _playerID;

   
    public void Initialized(string playerID)
    {
        _playerID = playerID;


    }

    public void Deinitialized()
    {

    }

    private void Start()
    {
  
    }

    private void Update()
    {
        // WŰ: ������ �̵�
        if (Input.GetKey(KeyCode.W))
        {
            transform.Translate(Vector3.forward * _moveSpeed * Time.deltaTime);
        }

        // DŰ: ���������� ȸ��
        if (Input.GetKey(KeyCode.D))
        {
            transform.Rotate(Vector3.up * _rotationSpeed * Time.deltaTime);
        }

        // AŰ: �������� ȸ�� (���û���)
        if (Input.GetKey(KeyCode.A))
        {
            transform.Rotate(Vector3.up * -_rotationSpeed * Time.deltaTime);
        }


    }

}
