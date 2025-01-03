#include "global.h"
#include "protocol.h"
#include "Over_IO.h"
#include "GameSession.h"
#include "Player.h"
#include "CharacterState.h"
#include "MapData.h"

// Global variables
SOCKET g_server_socket, g_client_socket;
HANDLE g_h_iocp;
Over_IO g_over;

std::unordered_map<int, GameSession> g_sessions;
concurrency::concurrent_priority_queue<TIMER_EVENT> commandQueue;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
std::unordered_map<std::string, ObjectOBB> g_obbData;

int GetSessionNumber()
{
	int room_num = 0;
	for (const auto& room : g_sessions)
	{
		std::lock_guard <std::mutex> lg{ g_sessions[room_num].mt_session_state_ };
		if (room.second.state_ == SESSION_FULL)
		{
			room_num++;
			continue;
		}
		else
		{
			if (g_sessions[room.first].CheckCharacterNum() < MAX_USER + MAX_NPC)
			{
				return room.first;
			}
			else
			{
				g_sessions[room.first].state_ = SESSION_FULL;
			}
		}
	}
	g_sessions[room_num].session_num_ = room_num;
	g_sessions[room_num].InitUDPSocket();
	return room_num;
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

		// Completion Key를 통해 세션 및 플레이어 식별
		CompletionKey* completionKey = reinterpret_cast<CompletionKey*>(key);
		int sessionId = completionKey->session_id;
		int playerIndex = completionKey->player_index;

		// TODO : UDP 처리
		/* 
		// 소켓 타입에 따라 처리 분기
		// UDP
		if (ex_over->socket_type_ == SOCKET_TYPE::UDP_SOCKET)
		{
			std::cout << "udp 받음\n";
		}
		//{
		//	// UDP 패킷 처리
		//	Packet* packet = reinterpret_cast<Packet*>(ex_over->wsabuf_.buf);
		//	g_sessions[sessionId].ProcessPacket(packet);  // 게임 세션을 통해 패킷 처리

		//	// ACK 패킷 전송
		//	g_sessions[sessionId].SendAck((SOCKET)completionKey, packet.sequenceNumber, ex_over->clientAddr_, ex_over->clientAddrLen_);

		//	// 비동기 수신을 위해 IOCP에 다시 등록
		//	memset(&(ex_over->over_), 0, sizeof(WSAOVERLAPPED));
		//	ex_over->wsabuf_.buf = ex_over->send_buf_;
		//	ex_over->wsabuf_.len = sizeof(Packet);
		//	ex_over->clientAddrLen_ = sizeof(ex_over->clientAddr_);

		//	DWORD flags = 0;
		//	WSARecvFrom((SOCKET)completionKey, &(ex_over->wsabuf_), 1, NULL, &flags,
		//		reinterpret_cast<sockaddr*>(&ex_over->clientAddr_), &ex_over->clientAddrLen_, &(ex_over->over_), NULL);
		//}
		//else if (ex_over->socket_type_ == SOCKET_TYPE::TCP_SOCKET) */

		{
			switch (ex_over->io_key_) {
			case IO_ACCEPT:
			{
				int room_num = GetSessionNumber();
				int client_id = static_cast<int>(g_sessions[room_num].CheckCharacterNum());

				// 새 소켓 생성 및 초기화
				SOCKET new_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				if (new_client_socket == INVALID_SOCKET) 
				{
					std::cout << "Error: Failed to create new_client_socket socket" << std::endl;
					break;
				}

				// 플레이어 초기화
				g_sessions[room_num].players_[client_id] = std::make_unique<Player>();
				g_sessions[room_num].players_[client_id]->SetSocket(g_client_socket);
				// Completion Key 생성
				CompletionKey* completion_key = new CompletionKey{room_num, client_id};
				g_sessions[room_num].players_[client_id]->SetCompletionKey(*completion_key);
				// IOCP 등록
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_client_socket), g_h_iocp, reinterpret_cast<ULONG_PTR>(completion_key), 0);
				// 수신 시작
				g_sessions[room_num].players_[client_id]->DoReceive();

				// AcceptEx를 통한 다음 클라이언트 대기
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

				// 해당 캐릭터 버퍼 재조립
				auto& buffer = player->prev_packet_;
				buffer.insert(buffer.end(), p, p + bytes);

				// 패킷 처리
				while (buffer.size() > 0)
				{
					size_t packet_size = static_cast<size_t>(buffer[0]);

					if (packet_size <= buffer.size())
					{
						player->ProcessPacket(buffer.data());
						buffer.erase(buffer.begin(), buffer.begin() + packet_size);
					}
					else
					{
						std::cout << "player num : " << playerIndex << " packet_size : " << packet_size << " buffer size : " << buffer.size() << std::endl;
						std::cout << "패킷이 완전하지 않음\n";
						break;
					}
				}
				
				player->DoReceive();
				
				break;
			}
			case IO_SEND:
			{
				delete ex_over;
				break;
			}
			case IO_MOVE:
			{

				for (int i = 0; i < MAX_USER + MAX_NPC; i++)
				{
					bool has_moved = (playerIndex & (1 << i)) != 0;
					if (true == has_moved)
					{
						g_sessions[sessionId].BroadcastPosition(i);
					}
				}

				// 10번 이상 이동이 있을 때마다 보정을 위한 위치 브로드캐스팅
				g_sessions[sessionId].update_count_++;
				if(g_sessions[sessionId].update_count_ >= 50)
				{
					g_sessions[sessionId].update_count_ = 0;
					std::cout << "****Sync Update Position ****\n";
					g_sessions[sessionId].BroadcastSync();
				}
				break;
			}

			}
		}
	}
}

// 각 세션의 위치, 시간, 남은 velocity 계산 등의 동기화를 50ms(초당 20번)단위로 실행하는 update Thread
void UpdateThread()
{
	while (true)
	{
		TIMER_EVENT ev;
		if (true == commandQueue.try_pop(ev))
		{
			// 아직 시간이 지나지 않았으면 다시 삽입
			if (ev.wakeup_time > std::chrono::system_clock::now()) 
			{
				commandQueue.push(ev);
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else
			{
				// 세션 업데이트
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

//void TimeThread()
//{
//	while (true)
//	{
//		TIMER_EVENT ev;
//		auto current_time = std::chrono::system_clock::now();
//		if (true == timer_queue.try_pop(ev))
//		{
//			if (ev.wakeup_time > current_time) {
//				timer_queue.push(ev);		
//				std::this_thread::yield();
//				continue;
//			}
//			// TODO : 시간 스레드는 분리할것
//			// 1초에 한번씩 게임 남은 시간 브로드캐스팅
//			uint64_t current_time = g_sessions[ev.session_id].GetServerTime();
//			std::cout << current_time << std::endl;
//
//			// 현재 세션 시간 업데이트
//			g_sessions[ev.session_id].lastupdatetime_ = current_time;
//			if (g_sessions[ev.session_id].lastupdatetime_ - g_sessions[ev.session_id].last_game_time_ >= 1000)
//			{
//				std::cout << "Time : " << g_sessions[ev.session_id].remaining_time_ << std::endl;
//				g_sessions[ev.session_id].last_game_time_ = current_time;
//				g_sessions[ev.session_id].remaining_time_--;
//				g_sessions[ev.session_id].SendTimeUpdate();
//			}
//		}
//	}
//}

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
		delete mapData;
	}
	else 
	{
		std::cout << "Failed to load map data." << std::endl;
	}


	std::thread update_thread(UpdateThread);
	std::thread update_thread2(UpdateThread);
	//std::thread timer_thread(TimeThread);
	

	// 내 cpu 코어 개수만큼의 스레드 생성
	int num_threads = std::thread::hardware_concurrency();
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
	//timer_thread.join();



	closesocket(g_server_socket);
	WSACleanup();
}