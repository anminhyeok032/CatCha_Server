#include "global.h"
#include "protocol.h"
#include "Over_IO.h"
#include "GameSession.h"
#include "Player.h"
#include "AIPlayer.h"
#include "CharacterState.h"
#include "MapData.h"
//#include "VoxelPatternManager.h"

// Global variables
SOCKET g_server_socket, g_client_socket;
HANDLE g_h_iocp;
Over_IO g_over;

std::unordered_map<int, GameSession> g_sessions;
concurrency::concurrent_priority_queue<TIMER_EVENT> commandQueue;
concurrency::concurrent_priority_queue<TIMER_EVENT> AI_Queue;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
std::vector<ObjectOBB> g_obbData;
DirectX::BoundingOrientedBox g_EscapeOBB;
VoxelPatternManager g_voxel_pattern_manager;

int GetSessionNumber(bool is_cat)
{
	int room_num = 0;
	for (const auto& room : g_sessions)
	{
		// �ӽ� ���� �н�
		if(room.first == -1)
		{
			continue;
		}

		std::lock_guard <std::mutex> lg{ g_sessions[room_num].mt_session_state_ };
		// ���� �� �� ���
		if (room.second.session_state_ == SESSION_FULL)
		{
			room_num++;
			continue;
		}
		// ���� �� ���� ���� ���
		else
		{	
			// ����̸� ������ ���
			if (true == is_cat)
			{
				// �ش� �濡 ����̰� �̹� �ִ� ���
				if (room.second.session_state_ == SESSION_HAS_CAT)
				{
					room_num++;
					continue;
				}
				// �ش� �濡 ����̰� ���� ���
				else
				{
					// ����� �ִ� ������ ����
					if (g_sessions[room.first].CheckCharacterNum() < SESSION_MAX_USER - 1)
					{
						g_sessions[room.first].session_state_ = SESSION_HAS_CAT;
					}
					// ���� ���� FULL�� ����
					else
					{
						g_sessions[room.first].session_state_ = SESSION_FULL;
					}
					return room.first;
				}
			}
			// �㸦 ������ ���
			else
			{
				// ����̰� �����ϴ� ���϶�
				if (room.second.session_state_ == SESSION_HAS_CAT)
				{
					// ���� ���� ��á����
					if (g_sessions[room.first].CheckCharacterNum() < SESSION_MAX_USER)
					{
						// ���� ���ͼ� �� á�ٸ�
						if(g_sessions[room.first].CheckCharacterNum() + 1 == SESSION_MAX_USER)
						{
							g_sessions[room.first].session_state_ = SESSION_FULL;
						}
						return room.first;
					}
					// ���� �� �� ���
					else
					{
						// ���� ���� ���·� ����
						room_num++;
						g_sessions[room.first].session_state_ = SESSION_FULL;
						continue;
					}
				}
				// ����̰� ���� ���϶�
				else
				{
					// ���� ���� ��á����
					if (g_sessions[room.first].CheckCharacterNum() < SESSION_MAX_USER - 1)
					{
						return room.first;
					}
					// ���� �� �� ���
					else
					{
						// �㸸 ������ �ٸ� �� ����
						room_num++;
						continue;
					}
				}
			}
		}
	}
	// ���ο� ���� ����
	g_sessions[room_num].session_num_ = room_num;
	if(true == is_cat)
	{
		std::lock_guard <std::mutex> lg{ g_sessions[room_num].mt_session_state_ };
		g_sessions[room_num].session_state_ = SESSION_HAS_CAT;
	}
	g_sessions[room_num].StartSessionUpdate();
	return room_num;
}

int GetWaitingPlayerNum()
{
	int player_num = 0;
	{
		std::lock_guard<std::mutex> lg(g_sessions[-1].mt_session_state_);
		for (int i = 0; i < MAX_USER; i++)
		{
			auto search = g_sessions[-1].players_.find(i);
			if (search != g_sessions[-1].players_.end())
			{
				player_num++;
				continue;
			}
			else
			{
				g_sessions[-1].players_.emplace(i, std::make_unique<Player>());
				std::cout << "Player [" << i << "] is waiting" << std::endl;
				return player_num;
			}
		}
	}
	std::cout << "Player [" << player_num << "] is waiting" << std::endl;
	return player_num;
}

void Worker()
{
	while (true)
	{
		DWORD bytes;
		ULONG_PTR key;
		Over_IO* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &bytes, &key, (LPOVERLAPPED*)&over, INFINITE);
		Over_IO* ex_over = reinterpret_cast<Over_IO*>(over);
		// add logic
		if (FALSE == ret)
		{
			if (ex_over->io_key_ == IO_ACCEPT)
			{
				//std::cout << "Error : Accept" << std::endl;
				//disconnect(key);
			}
			else
			{
				//std::cout << "Error : GQCS error Client [" << key << "]" << std::endl;
				//disconnect(key);
				if (ex_over->io_key_ == IO_SEND) delete ex_over;
				continue;
			}
		}
		if (bytes == 0)
		{
			if ((ex_over->io_key_ == IO_RECV) || (ex_over->io_key_ == IO_SEND))
			{
				//std::cout << "Error : Client [" << key << "]" << std::endl;
				//disconnect(key);
				if (ex_over->io_key_ == IO_SEND) delete ex_over;
				continue;
			}
		}

		// Completion Key�� ���� ���� �� �÷��̾� �ĺ�
		CompletionKey* completionKey = reinterpret_cast<CompletionKey*>(key);
		int sessionId;
		int playerIndex;
		if (!completionKey || !completionKey->session_id || !completionKey->player_index)
		{
			// ���ο� ���� ���ӽ�
		}
		else 
		{
			sessionId = *(completionKey->session_id);
			playerIndex = *(completionKey->player_index);
		}


		{
			switch (ex_over->io_key_) {
			case IO_ACCEPT:
			{
				int* client_id = new int(GetWaitingPlayerNum());
				if(*client_id != -1)
				{
					// �ΰ��� �� �ӽ� Session�� ��� ĳ���� ���ý� �ű�
					int* temp_session_num = new int(-1);
					{
						std::lock_guard<std::mutex> lg{ g_sessions[*temp_session_num].players_[*client_id]->mt_player_server_state_};
						g_sessions[*temp_session_num].players_[*client_id]->player_server_state_ = PLAYER_STATE::PS_ALLOC;
					}
					g_sessions[*temp_session_num].players_[*client_id]->SetSocket(g_client_socket);
					
					// �ӽ� Completion Key ����
					CompletionKey* completion_key = new CompletionKey{ temp_session_num, client_id};
					g_sessions[*temp_session_num].players_[*client_id]->SetCompletionKey(*completion_key);

					// IOCP ���
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_client_socket), g_h_iocp, reinterpret_cast<ULONG_PTR>(completion_key), 0);
					// ���� ����
					g_sessions[*temp_session_num].players_[*client_id]->DoReceive();
				}
				else 
				{
					std::cout << "Error: MAX USER EXCEEDED" << std::endl;
					break;
				}

				// AcceptEx�� ���� ���� Ŭ���̾�Ʈ ���
				g_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				ZeroMemory(&g_over.over_, sizeof(g_over.over_));
				int res = AcceptEx(g_server_socket, g_client_socket, g_over.send_buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &g_over.over_);
				if (res == SOCKET_ERROR)
				{
					std::cout << "Error: AcceptEx failed" << std::endl;
					closesocket(g_client_socket);
				}
				break;
			}
			case IO_RECV:
			{
				auto& player = g_sessions[sessionId].players_[playerIndex];
				char* p = ex_over->send_buf_;
				size_t total_data = bytes + player->prev_packet_.size();

				// �ش� ĳ���� ���� ������
				auto& buffer = player->prev_packet_;
				buffer.insert(buffer.end(), p, p + bytes);

				// ��Ŷ ó��
				while (buffer.size() > 0)
				{
					size_t packet_size = static_cast<size_t>(buffer[0]);

					if (packet_size <= buffer.size())
					{
						player->ProcessPacket(buffer.data());
						if (false == buffer.empty())
						{
							buffer.erase(buffer.begin(), buffer.begin() + packet_size);
						}
					}
					else
					{
						std::cout << "player num : " << playerIndex << " packet_size : " << packet_size << " buffer size : " << buffer.size() << std::endl;
						std::cout << "��Ŷ�� �������� ����\n";
						break;
					}
				}
				
				if (player)
				{
					player->DoReceive();
				}
				else
				{
					// ���ο� �������� �ӽ� ���� �÷��̾� ���� �ű�
					g_sessions[sessionId].players_.erase(playerIndex);
				}
				
				break;
			}
			case IO_SEND:
			{
				delete ex_over;
				break;
			}
			case IO_MOVE:
			{

				for (int i = 0; i < SESSION_MAX_USER; i++)
				{
					bool has_moved = (playerIndex & (1 << i)) != 0;
					if (true == has_moved)
					{
						g_sessions[sessionId].BroadcastPosition(i);
					}
				}

				// 10�� �̻� �̵��� ���� ������ ������ ���� ��ġ ��ε�ĳ����
				g_sessions[sessionId].update_count_++;
				if(g_sessions[sessionId].update_count_ >= 50)
				{
					g_sessions[sessionId].update_count_ = 0;
					g_sessions[sessionId].BroadcastSync();
				}

				delete completionKey->player_index;
				delete completionKey;
				delete ex_over;
				break;
			}
			case IO_AI_MOVE:
			{
				for (int i = NUM_AI1; i <= NUM_AI4; i++)
				{
					bool has_moved = (playerIndex & (1 << i)) != 0;
					if (true == has_moved)
					{
						g_sessions[sessionId].BroadcastAIPostion(i);
					}
				}
				delete completionKey->player_index;
				delete completionKey;
				delete ex_over;
				break;
			}
			case IO_TIME:
			{
				g_sessions[sessionId].BroadcastTime();
				delete completionKey->player_index;
				delete completionKey;
				delete ex_over;
				break;
			}
			case IO_GAME_EVENT:
			{
				switch (static_cast<GAME_EVENT>(playerIndex))
				{
				case GAME_EVENT::GE_OPEN_DOOR:
				{
					g_sessions[sessionId].BroadcastDoorOpen();
					break;
				}
				case GAME_EVENT::GE_WIN_CAT:
				case GAME_EVENT::GE_WIN_MOUSE:
				{
					g_sessions[sessionId].CheckResult();
					break;
				}
				default:
					std::cout << "Error : IO thread IO_GAME_EVENT" << std::endl;
					break;
				}

				break;
			}

			default:
			{
				std::cout << "Error : IO thread ex_over->io_key_" << std::endl;
				break;
			}

			}
		}
	}
}

// �� ������ ��ġ, �ð�, ���� velocity ��� ���� ����ȭ�� 50ms(�ʴ� 20��)������ �����ϴ� update Thread
void UpdateThread()
{
	while (true)
	{
		TIMER_EVENT ev;
		if (true == commandQueue.try_pop(ev))
		{
			// ���� �ð��� ������ �ʾ����� �ٽ� ����
			if (ev.wakeup_time > std::chrono::system_clock::now()) 
			{
				commandQueue.push(ev);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else
			{
				// ���� ������Ʈ
				g_sessions[ev.session_id].Update();
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
	}
}

void AIUpdateThread()
{
	while (true)
	{
		TIMER_EVENT ev;
		
		// AI ������Ʈ
		if (true == AI_Queue.try_pop(ev))
		{
			if (ev.wakeup_time > std::chrono::system_clock::now())
			{
				AI_Queue.push(ev);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else
			{
				g_sessions[ev.session_id].UpdateAI();
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
	}
}

void TimerThread()
{
	while (true)
	{
		TIMER_EVENT ev;
		auto current_time = std::chrono::system_clock::now();
		if (true == timer_queue.try_pop(ev))
		{
			if (ev.wakeup_time > current_time) 
			{
				timer_queue.push(ev);		
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else
			{
				Over_IO* over = new Over_IO;
				over->io_key_ = IO_TIME;
				int* time = new int(-1);
				CompletionKey* completion_key = new CompletionKey{ &ev.session_id, time };
				PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
			}
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}
	}
}

int main()
{
	std::wcout.imbue(std::locale("korean"));

	WSADATA WSADATA;
	WSAStartup(MAKEWORD(2, 2), &WSADATA);

	g_server_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(g_server_socket, reinterpret_cast<SOCKADDR*>(&server_addr), sizeof(server_addr));
	listen(g_server_socket, SOMAXCONN);

	SOCKADDR_IN client_addr;
	int client_addr_size = sizeof(client_addr);
	g_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_server_socket), g_h_iocp, reinterpret_cast<ULONG_PTR>(new CompletionKey{ 0, 0 }), 0);
	g_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_over.io_key_ = IO_ACCEPT;
	AcceptEx(g_server_socket, g_client_socket, g_over.send_buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &g_over.over_);

	//==================
	// MapData Load
	//==================
	MapData* mapData = new MapData();
	if (mapData->LoadMapData("Map.txt")) 
	{
		std::cout << "Map data loaded." << std::endl;
		std::cout << "Loading Tile Map for AI..." << std::endl;
		mapData->CheckTileMap4AI();
		//mapData->PrintTileMap();
		std::cout << "Tile Map for AI loaded." << std::endl;
		delete mapData;
	}
	else 
	{
		std::cout << "Failed to load map data." << std::endl;
	}

	// �ӽ� ���� ����
	g_sessions.try_emplace(-1, -1);


	std::thread update_thread(UpdateThread);
	std::thread update_thread2(UpdateThread);
	std::thread timer_thread(TimerThread);
	
	std::thread AI_thread(AIUpdateThread);


	// �� cpu �ھ� ������ŭ�� ������ ����
	int num_threads = std::thread::hardware_concurrency();
	if(num_threads <= 0)
	{
		std::cout << "Error: No available threads" << std::endl;
		return 0;
	}

	std::vector<std::thread> worker_threads;

	for (int i = 0; i < num_threads; ++i)
	{
		worker_threads.emplace_back(Worker);
	}
	for (auto& w : worker_threads)
	{
		w.join();
	}
	update_thread.join();
	update_thread2.join();
	timer_thread.join();
	AI_thread.join();


	closesocket(g_server_socket);
	WSACleanup();
}