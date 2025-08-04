#include "ServerManager.h"

namespace Manager
{
	void ServerManager::ProcessHeartBeat()
	{
		int size = _playerMap.size();
		flatbuffers::FlatBufferBuilder builder;// while�� �ȿ��� ������ builder�� while������� �����ȴ�.
		builder.Finish(protocol::CreateREQUEST_HEARTBEAT(builder));

		for(int i=0;i< size; ++i)
		{
			auto it = _playerMap.begin();
			std::advance(it, i);
			if (it == _playerMap.end())
				break;
			auto player = it->second;
			if (player == nullptr)
				continue;

			player->RequestHeartBeat();
			_clientManager->AddMessageToClient(player->GetCompletionKey(), builder.GetBufferPointer(), builder.GetSize());
		}
	}

} 