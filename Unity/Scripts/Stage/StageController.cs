using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class StageController
{
    private Dictionary<string, StageCharacter> _stageCharacters;

    public void Construct()
    {
        _stageCharacters = new Dictionary<string, StageCharacter>();
    }

    public void CreatePlayerCharacter(string playerID)
    {
        StageCharacter stageCharacter = new StageCharacter();// 실제로는 케릭터에서 모델링key를 받아서 getcomponent할 생각중.
        stageCharacter.Initialized(playerID);
        if(!_stageCharacters.TryAdd(playerID, stageCharacter))
        {
            Debug.Log($"Player Character Create Fail_DuplicateID : {playerID}");
            stageCharacter.Deinitialized();
        }
    }
}
