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
	return room_num;
}


std::unordered_map<int, GameSession> g_sessions;

void Worker()
{
	while (true)
	{
		DWORD bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		// TODO : key�� 2���� ������ �ް� ����ü�� Create������
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
			// TODO : DB üũ �� �α��� ��Ű�� ���� ������. ����� �ӵ��� ���� ���� �߰� ����

			int room_num = GetSessionNumber();
			std::cout << "check accept**" << std::endl;	
			std::cout << "Session ���� check : " << room_num << std::endl;	
			int client_id = g_sessions[room_num].CheckCharacterNum();

			// TODO : ���ǿ� �÷��̾ �� �������� ��� ��Ű���� �и��� ��

			// �÷��̾� �ʱ�ȭ
			g_sessions[room_num].characters_[client_id] = std::make_unique<Player>();
			g_sessions[room_num].characters_[client_id]->SetSocket(g_client_socket);

			// �ٸ� �÷��̾� ���� ���� �ʱ�ȭ
			g_client_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);


			ZeroMemory(&g_over.over_, sizeof(g_over.over_));
			AcceptEx(g_server_socket, g_client_socket, g_over.send_buf_, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, &g_over.over_);
			break;
		}
		case IO_RECV:
		{
			char* p = ex_over->send_buf_;

			// TODO : ��ġ ����ŷ ���� ���� ���ǰ����� ������ �ʿ���
			int total_data = bytes + g_sessions[key].characters_[0]->prev_packet_.size();

			auto& buffer = g_sessions[key].characters_[0]->prev_packet_;
			buffer.insert(buffer.end(), ex_over->send_buf_, ex_over->send_buf_ + bytes);

			while (buffer.size() > 0)
			{
				int packet_size = static_cast<int>(p[0]);
				if (packet_size <= buffer.size())
				{
					g_sessions[key].characters_[0]->ProcessPacket(buffer.data());
					buffer.erase(buffer.begin(), buffer.begin() + packet_size);
				}
				else
				{
					break;
				}
			}
			g_sessions[key].characters_[0]->DoReceive();
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


	// �� cpu �ھ� ������ŭ�� ������ ����
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