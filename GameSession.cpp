#include "GameSession.h"
#include "Player.h" 
#include "CatPlayer.h"
#include "MousePlayer.h"

bool GameSession::Update()
{
    bool need_update = false;
    uint64_t current_time = GetServerTime();
    int move_players = 0;
    {
        std::lock_guard<std::mutex> lock(mt_session_state_);
        float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

        // 이전 updateTime이 FIXED_TIME_STEP보다 적으면 false
        if (deltaTime < FIXED_TIME_STEP)
        {
            return false;
        }


        // 움직인 플레이어의 Postion만 업데이트 하도록 int 8마리의 움직임 여부를 파싱해서 담음
        for (auto& pl : players_)
        {
            if (true == pl.second->UpdatePosition(deltaTime))
            {
                need_update = true;
                move_players |= (1 << pl.second->comp_key_.player_index);
            }
        }

        // 현재 세션 시간 업데이트
        lastupdatetime_ = current_time;

        // 업데이트가 필요할때만, 클라이언트에게 브로드캐스팅
        if (true == need_update)
        {
            SendPlayerUpdate(move_players);
            
            //MarkDirty();
            commandQueue.push(session_num_);
            
            //std::cout << "re-update check" << std::endl;
        }
        else
        {
            // 업데이트 할게 없으면
            ClearDirty();
        }

    }
   
    return true;
    
}

void GameSession::SendPlayerUpdate(int move_players)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_MOVE;
    CompletionKey* completion_key = new CompletionKey{ session_num_, move_players };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}

void GameSession::SendTimeUpdate()
{
    SC_TIME_PACKET p;
    p.size = sizeof(p);
    p.type = SC_TIME;
    p.time = remaining_time_;

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }

    TIMER_EVENT ev{ std::chrono::system_clock::now(), session_num_ };
    timer_queue.push(ev);

}

void GameSession::InitUDPSocket()
{
    // UDP 소켓 설정
    udp_socket_ = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (udp_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Bind socket to port
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDPPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return;
    }

    CompletionKey completion_key{ session_num_, -1 };
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(udp_socket_), g_h_iocp, reinterpret_cast<ULONG_PTR>(&completion_key), 0);
    
    Over_IO* over = new Over_IO();
    DWORD flag = 0;
    int res = WSARecvFrom(udp_socket_, &over->wsabuf_, 1, nullptr, &flag, reinterpret_cast<sockaddr*>(&over->clientAddr_),
        &over->clientAddrLen_, &over->over_, nullptr);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        print_error("WSARecvFrom", WSAGetLastError());
    }
}

void GameSession::BroadcastPosition(int player)
{
    SC_MOVE_PLAYER_PACKET p;
    p.size = sizeof(p);
    p.type = SC_MOVE_PLAYER;
    p.id = player;
    p.x = players_[player]->position_.x;
    p.y = players_[player]->position_.y;
    p.z = players_[player]->position_.z;
    // tick동안 쌓인 pitch 전송
    p.player_pitch = players_[player]->total_pitch_;
    // 쌓인 pitch 초기화
    players_[player]->total_pitch_ = 0.0f;
    //std::cout << "Player Position : " << p.x << ", " << p.y << ", " << p.z << std::endl;

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }
    
    // TODO : UDP로 변경
    //for (auto& player : characters_) // 세션에 속한 클라이언트 목록
    //{
    //    int res = sendto(udp_socket_, (char*)&p, sizeof(p), 0,
    //        (sockaddr*)&player.second->client_addr_.sin_addr, sizeof(player.second->client_addr_));
    //    if (res != 0)
    //    {
    //        print_error("udp", WSAGetLastError());
    //    }
    //}
}

void GameSession::BroadcastSync()
{
    for (auto& pl : players_) // 세션에 속한 클라이언트 목록
    {
        SC_SYNC_PLAYER_PACKET p;
        p.size = sizeof(p);
        p.type = SC_SYNC_PLAYER;
        p.id = pl.first;
        p.x = players_[pl.first]->position_.x;
        p.y = players_[pl.first]->position_.y;
        p.z = players_[pl.first]->position_.z;
        p.quat_x = players_[pl.first]->rotation_quat_.x;
        p.quat_y = players_[pl.first]->rotation_quat_.y;
        p.quat_z = players_[pl.first]->rotation_quat_.z;
        p.quat_w = players_[pl.first]->rotation_quat_.w;

        for (auto& player : players_) // 세션에 속한 클라이언트 목록
        {
            if(pl.first == player.first) continue; // 자기 자신 제외
            player.second->DoSend(&p);
        }
    }
}

void GameSession::BroadcastChangeCharacter(int player_num, int CHARACTER_NUM)
{
    SC_CHANGE_CHARACTER_PACKET p;
    p.size = sizeof(p);
    p.type = SC_CHANGE_CHARACTER;
    p.id = player_num;
    p.prev_character_num = static_cast<uint8_t>(players_[player_num]->id_);
    players_[player_num]->SetID(CHARACTER_NUM);
    p.new_character_num = static_cast<uint8_t>(players_[player_num]->id_);
    for (auto& pl : players_)
	{
		pl.second->DoSend(&p);
	}
}

void GameSession::BroadcastAddCharacter(int player_num, int recv_index)
{
    SC_ADD_PLAYER_PACKET p;
    p.size = sizeof(p);
    p.type = SC_ADD_PLAYER;
    p.id = player_num;
    p.character_num = static_cast<uint8_t>(players_[player_num]->id_);
    p.x = players_[player_num]->position_.x;
    p.y = players_[player_num]->position_.y;
    p.z = players_[player_num]->position_.z;
    p.quat_x = players_[player_num]->rotation_quat_.x;
    p.quat_y = players_[player_num]->rotation_quat_.y;
    p.quat_z = players_[player_num]->rotation_quat_.z;
    p.quat_w = players_[player_num]->rotation_quat_.w;

    players_[recv_index]->DoSend(&p);
}

uint64_t GameSession::GetServerTime()
{
    // 서버 시간 가져오기
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}


void GameSession::SetCharacter(int room_num, int client_index, bool is_cat) 
{
    // Update 쓰레드 세션 접근 락
    std::lock_guard<std::mutex> lock(mt_session_state_);

    auto& player = players_[client_index];

    if (player->id_ == NUM_GHOST)
    {
        for (auto& pl : players_)
        {
            if(client_index == pl.second->comp_key_.player_index) continue;              // 자기 자신
            if(pl.second->id_ == NUM_GHOST) continue;       		                     // 유령 상태    
            BroadcastAddCharacter(client_index, pl.second->comp_key_.player_index);
            BroadcastAddCharacter(pl.second->comp_key_.player_index, client_index);
        }
    }

    // 고양이일 때
    if (is_cat) 
    {
        player->SetState(std::make_unique<CatPlayer>());
        player->SetID(NUM_CAT);
        player->Set_OBB(player->state_->GetOBB());
        //player->position_.y = -58.4f;
    }
    // 쥐일 때
    else 
    {
        player->SetState(std::make_unique<MousePlayer>());
        player->SetID(GetMouseNum());
        player->Set_OBB(player->state_->GetOBB());
        //player->position_.y = -59.53f;
    }

    // 캐릭터 교체 브로드캐스트
    BroadcastChangeCharacter(client_index, players_[client_index]->id_);
    
}

int GameSession::GetMouseNum()
{
    // 사용 중인 마우스 ID 확인용 array
    std::array<bool, 4> mouse_ids_check = { false, false, false, false };

    // 현재 플레이어들의 ID 확인 후 체크
    for (const auto& pl : players_) 
    {
        int id = pl.second->id_; 
        if (id >= NUM_MOUSE1 && id <= NUM_MOUSE4) 
        {
            mouse_ids_check[id] = true;
        }
    }

    // 사용 가능한 마우스 ID return
    for (int i = 0; i < 4; ++i) 
    {
        if (false == mouse_ids_check[i]) 
        {
            return i;
        }
    }

    // 모든 마우스 ID가 사용 중이라면 -1 반환 (에러)
    std::cout << "모든 마우스 ID가 사용 중!!" << std::endl;
    return -1;
}

// Dirty Flag 설정
void GameSession::MarkDirty() 
{
    dirty_.store(true);
}

// Dirty Flag 확인
bool GameSession::IsDirty() const 
{
    return dirty_.load();
}

// Dirty Flag 초기화
void GameSession::ClearDirty() 
{
    dirty_.store(false);
}