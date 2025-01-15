#include "GameSession.h"
#include "Player.h" 
#include "CatPlayer.h"
#include "MousePlayer.h"

void GameSession::Update()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_players = 0;
    {
        std::lock_guard<std::mutex> lock(mt_session_state_);
        float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

        // 움직인 플레이어의 Postion만 업데이트 하도록 int 8마리의 움직임 여부를 파싱해서 담음
        for (auto& pl : players_)
        {
            // 업데이트 요청이 없으면
            if(pl.second->needs_update_.load() == false) continue;

            if (true == pl.second->UpdatePosition(deltaTime))
            {
                need_update_send = true;
                move_players |= (1 << pl.second->comp_key_.player_index);
            }
        }

        // 현재 세션 시간 업데이트
        lastupdatetime_ = current_time;

        // 업데이트가 필요할때만, 클라이언트에게 브로드캐스팅 및 업데이트 스레드 재동작
        if (true == need_update_send)
        {
            SendPlayerUpdate(move_players);
        }
        TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(UPDATE_PERIOD_INT), session_num_ };
        commandQueue.push(ev);
    }
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
    Player* pl = players_[player].get();
    SC_MOVE_PLAYER_PACKET p;
    p.size = sizeof(p);
    p.type = SC_MOVE_PLAYER;
    p.id = player;
    p.x = pl->position_.x;
    p.y = pl->position_.y;
    p.z = pl->position_.z;
    // tick동안 쌓인 pitch 전송
    p.player_pitch = pl->total_pitch_;
    // 쌓인 pitch 초기화
    pl->total_pitch_ = 0.0f;
    
    // state와 cat_attacked를 파싱해서 unsigned char로 변환
    unsigned char state_value = static_cast<unsigned char>(pl->obj_state_);
    // cat_attacked을 최하위 비트에 저장
    p.state = (state_value << 1) 
        | (cat_attacked_player_[pl->id_] ? 1 : 0);

    if (pl->stop_skill_time_ > 0.001f)
    {
        // 스킬 타음 사용전까지 기다림
    }
    // 점프시작을 전송후, 점프 idle로 변경
    else if (pl->obj_state_ == Object_State::STATE_JUMP_START)
    {
        players_[player]->obj_state_ = Object_State::STATE_JUMP_IDLE;
    }
    // 점프 end 전송후, 다음 스테이트 (idle||MOVE)지정 && 움직일수 있는 상황일때
    else if (pl->obj_state_ == Object_State::STATE_JUMP_END && pl->moveable_ == true)
    {
        if (pl->speed_ > 0.05f)
        {
            players_[player]->obj_state_ = Object_State::STATE_MOVE;
        }
        else
        {
            players_[player]->obj_state_ = Object_State::STATE_IDLE;
        }
        players_[player]->moveable_ = true;
    }


    // 모든 클라이언트에게 브로드캐스팅
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

void GameSession::BroadcastRemoveVoxelSphere(const DirectX::XMFLOAT3& center)
{
    SC_REMOVE_VOXEL_SPHERE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_REMOVE_VOXEL_SPHERE;
	p.cheese_num = 0;
	p.center_x = center.x;
	p.center_y = center.y;
	p.center_z = center.z;

	for (auto& pl : players_)
	{
		pl.second->DoSend(&p);
	}
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
    }
    // 쥐일 때
    else 
    {
        player->SetState(std::make_unique<MousePlayer>());
        player->SetID(GetMouseNum());
        player->Set_OBB(player->state_->GetOBB());
    }

    // 캐릭터 교체 브로드캐스트
    BroadcastChangeCharacter(client_index, players_[client_index]->id_);
    player->RequestUpdate();
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
    std::cout << session_num_ << "번 세션 : 모든 마우스 ID가 사용 중!!" << std::endl;
    return -1;
}


void GameSession::CheckAttackedMice()
{
    // 모든 쥐 검사
    for (auto& mouse : players_)
    {
        if(mouse.second->id_ == NUM_CAT) continue; // 고양이는 제외

        for (const auto& mouse_attacked : cat_attacked_player_)
        {
            // 공격 받은 쥐
            if (mouse_attacked.second == true)
            {
                // 자기 자신이면 중복 공격 방지
                if (mouse.second->id_ == mouse_attacked.first)
                {
                    return;
                }
            }
        }

        // 고양이의 공격 OBB와 쥐의 OBB 충돌 검사
        if (state_)
        {
            if (true == cat_attack_obb_.Intersects(mouse.second->state_->GetOBB()))
            {
                mouse.second->velocity_vector_.x = cat_attack_direction_.x * CAT_PUNCH_POWER * 10.0f;
                mouse.second->velocity_vector_.y = CAT_PUNCH_POWER;
                mouse.second->velocity_vector_.z = cat_attack_direction_.z * CAT_PUNCH_POWER * 10.0f;
                std::cout << "Cat Attack Success : mouse - " << mouse.second->id_ << std::endl;
                cat_attacked_player_[mouse.second->id_] = true;
                mouse.second->RequestUpdate();
            }
        }
    }
}


void GameSession::CrtVoxelCheeseOctree(OctreeNode& root, DirectX::XMFLOAT3 position, float scale, UINT detail_level)
{
    int count = 0;
    int m_random_value = 10;

    std::random_device rd;
    std::uniform_int_distribution<int> uid(1, m_random_value);

    DirectX::XMFLOAT3 pivot_position = position;
    position.y += scale / 2.0f;

    // 치즈 옥트리 초기 기준 정하기
    root.halfSize = VOXEL_CHEESE_DEPTH * scale / 2.0f;             // 가장 긴축을 기준으로 옥트리 크기 설정
    root.center = DirectX::XMFLOAT3(
        pivot_position.x,                               // x축 중심은 시작 x 좌표
        pivot_position.y + VOXEL_CHEESE_HEIGHT * scale / 2.0f,      // y축 중심은 높이의 중간
        pivot_position.z                                // z축 중심은 시작 z 좌표
    );
    root.boundingBox = DirectX::BoundingBox(root.center, { root.halfSize, root.halfSize, root.halfSize });

    for (int i = 0; i < VOXEL_CHEESE_HEIGHT; ++i)
    {
        position.z = pivot_position.z - scale * (float)(VOXEL_CHEESE_DEPTH / 2);

        for (int j = 1; j <= VOXEL_CHEESE_DEPTH; ++j)
        {
            position.x = pivot_position.x - scale * (float)(VOXEL_CHEESE_WIDTH / 2);

            for (int k = 0; k <= j / 2; ++k) 
            {
                // TODO : 특정 치즈 삭제 가능시, 클라로 치즈 삭제 패킷 보내기
                //        모든 플레이어 접속후, 치즈 로드하게 로직 변경 필요함
                /*if ((i == y_value - 1 || j == z_value || k == 0 || k == j / 2) &&
                    !(uid(rd) % m_random_value)) {
                    position.x += scale;
                    continue;
                }*/

                // 옥트리에 삽입
                SubdivideVoxel(root, position, scale, detail_level);
                position.x += scale;
            }

            position.z += scale;
        }

        position.y += scale;
    }
    root.PrintNode();
}

// 복셀 분할 함수
void GameSession::SubdivideVoxel(OctreeNode& node, DirectX::XMFLOAT3 position, float scale, UINT detail_level) 
{
    if (detail_level == 0) 
    {
        // 더 이상 분할하지 않고 복셀 삽입
        node.InsertVoxel(position, detail_level + MAX_DEPTH, 0);
        return;
    }

    // 현재 스케일을 절반으로 줄이고, detail_level 만큼 새로운 중심점을 계산하여 8개의 하위 복셀 생성
    // 8 ^ detail_level
    float half = scale / 2.0f;
    float quarter = scale / 4.0f;

    for (int dx = -1; dx <= 1; dx += 2) 
    {
        for (int dy = -1; dy <= 1; dy += 2) 
        {
            for (int dz = -1; dz <= 1; dz += 2) 
            {
                // 각 방향으로 분할된 복셀의 중심점 계산
                DirectX::XMFLOAT3 new_position = {
                    position.x + quarter * dx,
                    position.y + quarter * dy,
                    position.z + quarter * dz
                };

                // 재귀적으로 하위 복셀 분할
                SubdivideVoxel(node, new_position, half, detail_level - 1);
            }
        }
    }
}

void GameSession::DeleteCheeseVoxel(const DirectX::XMFLOAT3& center)
{
    DirectX::BoundingSphere sphere{ center, 5.0f };

    for (auto& cheese : cheese_octree_)
    {
        if (true == cheese.RemoveVoxel(sphere))
        {
            std::cout << "치즈 삭제 성공" << std::endl;
            //cheese_octree_.PrintNode();

            BroadcastRemoveVoxelSphere(center);
        }
        else
        {
            std::cout << "치즈 삭제 실패" << std::endl;
        }
    }

}