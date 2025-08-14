using System.Collections;
using System.Collections.Generic;
using Network;
using UnityEngine;

public class Player 
{
    private Network.NetworkUser _networkUser;

    public void Initialize(Network.NetworkBase networkBase)
    {
        _networkUser = new Network.NetworkUser();
        _networkUser.Initialize(networkBase);


    }

    public void Deinitialize()
    {
        if (_networkUser != null)
        {
            _networkUser.Disconnect();
            _networkUser = null;
        }
    }

    public void SendMessage(string message)
    {
        _networkUser.SendMessage(message);
    }
}
