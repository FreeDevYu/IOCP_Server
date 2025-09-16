using System;
using System.Collections;
using System.Collections.Generic;
using Google.FlatBuffers;
using Network;
using UnityEngine;

public class MessageSendModule 
{
    private static readonly MessageSendModule _instance = new MessageSendModule();

    public static MessageSendModule Instance => _instance;

    private MessageSendModule() { }


    public void REQUEST_REGISTER(FlatBufferBuilder builder, Network.NetworkUser networkUser, string playerID, string secretKey)
    {
        builder.Clear();//쓰레드 안전한지 체크 필요

        StringOffset playerIdOffset = builder.CreateString(playerID);
        StringOffset secretKeyOffset = builder.CreateString(secretKey);

        Offset<protocol.REQUEST_REGISTER> data = protocol.REQUEST_REGISTER.CreateREQUEST_REGISTER(builder, playerIdOffset, secretKeyOffset);
        builder.Finish(data.Value);
        byte[] bodyBytes = builder.SizedByteArray();

        Network.MessageHeader messageHeader = new Network.MessageHeader((uint)bodyBytes.Length, (uint)protocol.MESSAGETYPE.REQUEST_REGISTER);
        byte[] headerBytes = messageHeader.ToBytes();

        byte[] messageBytes = new byte[bodyBytes.Length + Network.NetworkDefine.NETWORK_HEADER_SIZE];
        Buffer.BlockCopy(headerBytes, 0, messageBytes, 0, Network.NetworkDefine.NETWORK_HEADER_SIZE);
        Buffer.BlockCopy(bodyBytes, 0, messageBytes, Network.NetworkDefine.NETWORK_HEADER_SIZE, bodyBytes.Length);

        networkUser.SendMessage(messageBytes);
    }

    public void RESPONSE_HEARTBEAT(FlatBufferBuilder builder, Network.NetworkUser networkUser, string playerID)
    {
        builder.Clear();//쓰레드 안전한지 체크 필요

        StringOffset playerIdOffset = builder.CreateString(playerID);

        Offset<protocol.RESPONSE_HEARTBEAT> data = protocol.RESPONSE_HEARTBEAT.CreateRESPONSE_HEARTBEAT(builder, playerIdOffset);
        builder.Finish(data.Value);
        byte[] bodyBytes = builder.SizedByteArray();

        Network.MessageHeader messageHeader = new Network.MessageHeader((uint)bodyBytes.Length, (uint)protocol.MESSAGETYPE.RESPONSE_HEARTBEAT);
        byte[] headerBytes = messageHeader.ToBytes();

        byte[] messageBytes = new byte[bodyBytes.Length + Network.NetworkDefine.NETWORK_HEADER_SIZE];
        Buffer.BlockCopy(headerBytes, 0, messageBytes, 0, Network.NetworkDefine.NETWORK_HEADER_SIZE);
        Buffer.BlockCopy(bodyBytes, 0, messageBytes, Network.NetworkDefine.NETWORK_HEADER_SIZE, bodyBytes.Length);

        networkUser.SendMessage(messageBytes);
    }

    public void REQUEST_PLAYERMOVE(
        FlatBufferBuilder builder,
        Network.NetworkUser networkUser,
        string playerID, protocol.MoveDirection direction,
        float speed,
        float duration)
    {
        builder.Clear();//쓰레드 안전한지 체크 필요
        StringOffset playerIdOffset = builder.CreateString(playerID);
        
        Offset<protocol.REQUEST_PLAYERMOVE> data = protocol.REQUEST_PLAYERMOVE.CreateREQUEST_PLAYERMOVE(builder, playerIdOffset, direction, speed, duration);
        builder.Finish(data.Value);
        byte[] bodyBytes = builder.SizedByteArray();
        Network.MessageHeader messageHeader = new Network.MessageHeader((uint)bodyBytes.Length, (uint)protocol.MESSAGETYPE.REQUEST_PLAYERMOVE);
        byte[] headerBytes = messageHeader.ToBytes();

        byte[] messageBytes = new byte[bodyBytes.Length + Network.NetworkDefine.NETWORK_HEADER_SIZE];
        Buffer.BlockCopy(headerBytes, 0, messageBytes, 0, Network.NetworkDefine.NETWORK_HEADER_SIZE);
        Buffer.BlockCopy(bodyBytes, 0, messageBytes, Network.NetworkDefine.NETWORK_HEADER_SIZE, bodyBytes.Length);

        networkUser.SendMessage(messageBytes);
    }
}
