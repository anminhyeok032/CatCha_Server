#include "global.h"
#include "protocol.h"
#include "Over_IO.h"
#include "GameSession.h"
#include "Player.h"

// Global variables
SOCKET g_server_socket, g_client_socket;
HANDLE g_h_iocp;
Over_IO g_over;

int GetSessionNumber()
{
	// TODO : 매치메이킹 시스템 시 비어있는 매칭 서버 값 돌려줘야한다
	return 0;
}


std::unordered_map<int, GameSession> g_sessions;

void Worker()
{
	while (true)
	{
		DWORD bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_h_iocp, &bytes, &key, &over, INFINITE);
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

		switch (ex_over->io_key_) {
		case IO_ACCEPT:
		{
			// TODO : DB 체크 후 로그인 시키는 과정 들어가야함. 디버깅 속도를 추후 추가 예정
			int client_id = 0;
			if (client_id != -1)
			{
				{
					std::lock_guard<std::mutex> ll(objects[client_id]->mut_state_);
					objects[client_id]->state_ = OS_ACTIVE;
				}
				g_sessions[client_id]->x_ = 0;
				objects[client_id]->y_ = 0;
				objects[client_id]->id_ = client_id;
				objects[client_id]->name_[0] = 0;
				objects[client_id]->prev_packet_.clear();
				objects[client_id]->visual_ = 0;
				objects[client_id]->SetSocket(g_client_socket);
				objects[client_id]->PutInSector();
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_client_socket),
					g_h_iocp, client_id, 0);
				objects[client_id]->DoReceive();
				// 접속 플레이어 리스트에 저장
				g_mut_player_list.lock();
				g_player_list.insert(client_id);
				g_mut_player_list.unlock();
				// 다른 플레이어 위해 소켓 초기화
				g_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else
			{
				std::cout << "Error : Max User" << std::endl;
			}

			ZeroMemory(&g_over.over_, sizeof(g_over.over_));
			AcceptEx(g_server_socket, g_client_socket, g_over.send_buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &g_over.over_);
			break;
			break;
		}
		case IO_RECV:
		{
			char* p = ex_over->send_buf_;

			// TODO : 매치 메이킹 이후 받은 세션값으로 변경이 필요함
			int total_data = bytes + g_sessions[key].players_[0].prev_packet_.size();

			auto& buffer = g_sessions[key].players_[0].prev_packet_;
			buffer.insert(buffer.end(), ex_over->send_buf_, ex_over->send_buf_ + bytes);

			while (buffer.size() > 0)
			{
				int packet_size = static_cast<int>(p[0]);
				if (packet_size <= buffer.size())
				{
					g_sessions[key].players_[0].ProcessPacket(buffer.data());
					buffer.erase(buffer.begin(), buffer.begin() + packet_size);
				}
				else
				{
					break;
				}
			}
			g_sessions[key].players_[0].DoReceive();
			break;
		}
		case IO_SEND:
		{
			delete ex_over;
			break;
		}
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
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_server_socket), g_h_iocp, 9999, 0);
	g_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_over.io_key_ = IO_ACCEPT;
	AcceptEx(g_server_socket, g_client_socket, g_over.send_buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &g_over.over_);


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


	closesocket(g_server_socket);
	WSACleanup();
}