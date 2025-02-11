#include "GameSession.h"
#include "Player.h" 
#include "CatPlayer.h"
#include "MousePlayer.h"
#include "AIPlayer.h"
#include "MapData.h"

void GameSession::Update()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_players = 0;
   
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // 움직인 플레이어의 Postion만 업데이트 하도록 int 5마리의 움직임 여부를 파싱해서 담음
    for (auto& pl : players_)
    {
        // 업데이트 요청이 없으면
        if(pl.second->needs_update_.load() == false) continue;

        if (true == pl.second->UpdatePosition(deltaTime))
        {
            need_update_send = true;
            move_players |= (1 << pl.first);
            //std::cout << "Player " << pl.first << " Move" << std::endl;
            //std::cout << "Position : " << pl.second->position_.x << ", " << pl.second->position_.y << ", " << pl.second->position_.z << std::endl;
        }
    }

    // 현재 세션 시간 업데이트
    lastupdatetime_ = current_time;

    // 움직임 업데이트가 필요할때만 클라이언트에게 브로드캐스팅
    if (true == need_update_send)
    {
        RequestSendPlayerUpdate(move_players);
    }

    // 게임 종료가 안되었으면 다시 업데이트
    if (true == CheckGameOver())
    {
        // 정해진 주기로 실행하기 위해 queue에 다시 담기
        TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(UPDATE_PERIOD_INT), session_num_ };
        commandQueue.push(ev);
    }
    
}


void GameSession::UpdateAI()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_AIs = 0;

    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // 움직인 AI의 Postion만 업데이트 하도록 int 4마리의 움직임 여부를 파싱해서 담음
    for (auto& pl : ai_players_)
    {
        if (pl.second->is_activate_.load() == false) continue;
        if (true == pl.second->UpdatePosition(deltaTime))
        {
            need_update_send = true;
            move_AIs |= (1 << pl.first);
            //std::cout << "Position : " << pl.second->position_.x << ", " << pl.second->position_.y << ", " << pl.second->position_.z << std::endl;
        }
    }

    // 현재 세션 시간 업데이트
    lastupdatetime_ = current_time;

    // 움직임 업데이트가 필요할때만 클라이언트에게 브로드캐스팅
    if (true == need_update_send)
    {
        RequestSendAIUpdate(move_AIs);
    }
    // 정해진 주기로 실행하기 위해 queue에 다시 담기
    TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(AI_UPDATE_PERIOD_INT / 2), session_num_ };
    AI_Queue.push(ev);
}


void GameSession::RequestSendPlayerUpdate(int move_players)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_MOVE;
    int* players = new int(move_players);
    CompletionKey* completion_key = new CompletionKey{ &session_num_, players };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}


void GameSession::RequestSendGameEvent(GAME_EVENT ge)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_GAME_EVENT;
    int* game_event = new int(static_cast<int>(ge));
    CompletionKey* completion_key = new CompletionKey{ &session_num_, game_event };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}


void GameSession::BroadcastGameStart()
{
    SC_GAME_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GAME_START;

    // 게임 시작으로 변경
    is_game_start_ = true;

    // Timer 스레드 시작
    remaining_time_ = GAME_TIME + FREEZING_TIME;
    BroadcastTime();

    // 플레이어 정보 초기화
	for (auto& pl : players_)
	{
        // 캐릭터 정보 다시 초기화
        pl.second->position_        = CHARACTER_POS[pl.second->character_id_];
        pl.second->rotation_quat_   = CHARACTER_ROTATION[pl.second->character_id_];
        pl.second->velocity_vector_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->force_vector_    = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->depth_delta_     = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->dirty_ = true;
        pl.second->keyboard_input_.clear();
        pl.second->UpdateLookUpRight();

        // 캐릭터 상태 초기화
        pl.second->moveable_ = false;
        pl.second->stop_skill_time_ = static_cast<float>(FREEZING_TIME);
        pl.second->obj_state_ = Object_State::STATE_IDLE;
        pl.second->curr_hp_ = 100;
        pl.second->needs_update_.store(true);

		pl.second->DoSend(&p);
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
        | (cat_attacked_player_[pl->character_id_] ? 1 : 0);

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
    p.player_num = player_num;
    p.prev_character_num = static_cast<uint8_t>(players_[player_num]->character_id_);
    players_[player_num]->SetID(CHARACTER_NUM);
    p.new_character_num = static_cast<uint8_t>(players_[player_num]->character_id_);
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
    p.character_num = static_cast<uint8_t>(players_[player_num]->character_id_);
    p.x = players_[player_num]->position_.x;
    p.y = players_[player_num]->position_.y;
    p.z = players_[player_num]->position_.z;
    p.quat_x = players_[player_num]->rotation_quat_.x;
    p.quat_y = players_[player_num]->rotation_quat_.y;
    p.quat_z = players_[player_num]->rotation_quat_.z;
    p.quat_w = players_[player_num]->rotation_quat_.w;

    players_[recv_index]->DoSend(&p);
}

void GameSession::BroadcastRemoveVoxelSphere(int cheese_num, const DirectX::XMFLOAT3& center, bool is_removed_all)
{
    SC_REMOVE_VOXEL_SPHERE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_REMOVE_VOXEL_SPHERE;
	p.cheese_num = (static_cast<unsigned char>(cheese_num) << 1) | (is_removed_all ? 1 : 0);    // 비트 맨끝에 모두 삭제됐는지 파싱
	p.center_x = center.x;
	p.center_y = center.y;
	p.center_z = center.z;

	for (auto& pl : players_)
	{
		pl.second->DoSend(&p);
	}
}

void GameSession::RequestSendAIUpdate(int move_AIs)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_AI_MOVE;
    int* ai = new int(move_AIs);
    CompletionKey* completion_key = new CompletionKey{ &session_num_, ai };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}

void GameSession::BroadcastAIPostion(int num)
{
    SC_AI_MOVE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_AI_MOVE;
    p.id = num;
    p.x = ai_players_[num]->position_.x;
    p.z = ai_players_[num]->position_.z;

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }
}

void GameSession::BroadcastTime()
{
    SC_TIME_PACKET p;
    p.size = sizeof(p);
    p.type = SC_TIME;
    p.time = remaining_time_--;

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }
    
    if (remaining_time_ > 0 && true == is_game_start_)
    {
        TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(1000), session_num_ };
        timer_queue.push(ev);
    }
    else if (remaining_time_ <= 0 && true == is_game_start_)
    {
        is_game_start_ = false;
        // 게임 종료
        RequestSendGameEvent(GAME_EVENT::GE_TIME_OVER);
    }
    else 
    {
        std::cout << "[ " << session_num_ << " ] - Time Thread 정지" << std::endl;
    }
}

void GameSession::BroadcastDoorOpen()
{
    SC_GAME_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_GAME_OPEN_DOOR;
    
    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }

    std::cout << "[ " << session_num_ << " ] - 치즈 다먹어서 문 열림" << std::endl;
    is_door_open_ = true;
}

void GameSession::BroadcastCatWin()
{
    std::cout << "[ " << session_num_ << " ] - CAT PLAYER WIN" << std::endl;
    SC_GAME_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_GAME_WIN_CAT;
    for (const auto& pl : players_)
    {
        if (pl.second->character_id_ == NUM_CAT)
        {
            std::cout << "player win - " << pl.first << std::endl;
            p.winner = pl.first;
        }
    }

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }
    
    // 플레이어 모두 대기 서버로 이동
    MovePlayerToWaitngSession();
}

void GameSession::BroadcastMouseWin()
{
    std::cout << "[ " << session_num_ << " ] - MOUSE PLAYER WIN" << std::endl;
    SC_GAME_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_GAME_WIN_MOUSE;

    p.winner = 0;
    for (const auto& pl : escape_mouse_)
    {
        std::cout << "player win - " << pl << std::endl;
        p.winner |= (1 << pl);      // 탈출한 쥐(player_index) 파싱해서 담음
    }

    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }

    // 플레이어 모두 대기 서버로 이동
    MovePlayerToWaitngSession();
}

void GameSession::BroadcastEscape()
{
    SC_PLAYER_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_PLAYER_ESCAPE;

    for (auto& mouse : escape_mouse_)
    {
        if (false == players_[mouse]->request_send_escape_)    continue;  // 보내기 요청 있는 애만
        players_[mouse]->request_send_escape_ = false;
        for (auto& pl : players_)
        {
            p.id = mouse;
            pl.second->DoSend(&p);
        }
    }
}

void GameSession::BroadcastReborn()
{
    SC_PLAYER_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_PLAYER_REBORN;

    // 자기 자신에게만 보냄
    for (auto& pl : players_)
    {
        if (false == pl.second->request_send_reborn_)    continue;  // 보내기 요청 있는 애만
        pl.second->request_send_reborn_ = false;
        p.id = pl.first;
        pl.second->DoSend(&p);
    }
}

void GameSession::BroadcastDead()
{
    SC_PLAYER_STATE_PACKET p;
    p.size = sizeof(p);
    p.type = SC_PLAYER_DEAD;

    // 자기 자신에게만 보냄
    for (auto& pl : players_)
    {
        if (false == pl.second->request_send_dead_)    continue;  // 보내기 요청 있는 애만
        pl.second->request_send_dead_ = false;
        p.id = pl.first;
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
    auto& player = players_[client_index];

    // 자신의 플레이어 번호 설정
    player->SendMyPlayerNumber();

    // 고양이일 때
    if (is_cat) 
    {
        player->SetState(std::make_unique<CatPlayer>());
        player->SetID(NUM_CAT);
        player->Set_OBB(player->character_state_->GetOBB());
    }
    // 쥐일 때
    else 
    {
        player->SetState(std::make_unique<MousePlayer>());
        player->SetID(GetMouseNum());
        player->Set_OBB(player->character_state_->GetOBB());

        // 살아있는 쥐 목록에 추가
        alive_mouse_.emplace(client_index, false);
    }

    // 새로 접속할 플레이어일시
    // - 새로운 접속 클라이언트에게 기존 클라이언트 정보 전송
    // - 기존 클라이언트 정보 새로운 클라이언트에게 전송
    {
        // 동시에 여러 스레드에서 캐릭터 변경
        std::lock_guard<std::mutex> lock(mt_session_state_);
        for (auto& pl : players_)
        {
            if (client_index == *pl.second->comp_key_.player_index) continue;              // 자기 자신
            if (pl.second->character_id_ == NUM_GHOST) continue;       		               // 유령 상태    
            BroadcastAddCharacter(client_index, *pl.second->comp_key_.player_index);
            BroadcastAddCharacter(*pl.second->comp_key_.player_index, client_index);
        }
    }

    // 캐릭터 교체 브로드캐스트
    BroadcastChangeCharacter(client_index, players_[client_index]->character_id_);

    // 세션 꽉 차면 게임 시작
    {
        std::lock_guard <std::mutex> lg{ mt_session_state_ };
        if (session_state_ == SESSION_STATE::SESSION_FULL)
        {
            // 게임 시작
            BroadcastGameStart();
            // AI 생성
            InitializeSessionAI();
        }
    }

    player->RequestUpdate();
}

int GameSession::GetMouseNum()
{
    // 사용 중인 마우스 ID 확인용 array
    std::array<bool, 4> mouse_ids_check = { false, false, false, false };

    {
        std::lock_guard<std::mutex> lock(mt_session_state_);
        // 현재 플레이어들의 ID 확인 후 체크
        for (const auto& pl : players_)
        {
            int id = pl.second->character_id_;
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
    }

    // 모든 마우스 ID가 사용 중이라면 -1 반환 (에러)
    std::cout << session_num_ << "번 세션 : 모든 마우스 ID가 사용 중!!" << std::endl;
    return -1;
}


void GameSession::CheckAttackedMice()
{
    // 아직 게임 시작 안되었을때,
    if (false == is_game_start_)
    {
        return;
    }

    // 모든 쥐 검사
    for (auto& mouse : alive_mouse_)
    {
        if (mouse.second == true)                                               continue;           // 탈출한 쥐는 제외
        if (true == cat_attacked_player_[players_[mouse.first]->character_id_]) continue;           // 이미 맞은 쥐에 속하면
        if (players_[mouse.first]->obj_state_ == Object_State::STATE_DEAD)      continue;           // 죽은 쥐 제외
        

        // 고양이의 공격 OBB와 쥐의 OBB 충돌 검사
        if (players_[mouse.first]->character_state_)
        {
            if (true == cat_attack_obb_.Intersects(players_[mouse.first]->character_state_->GetOBB()))
            {

                players_[mouse.first]->velocity_vector_.x = cat_attack_direction_.x * CAT_PUNCH_POWER;
                players_[mouse.first]->velocity_vector_.y = CAT_PUNCH_POWER / 2.0f;
                players_[mouse.first]->velocity_vector_.z = cat_attack_direction_.z * CAT_PUNCH_POWER;
                std::cout << "Cat Attack Success : mouse - " << players_[mouse.first]->character_id_ << std::endl;
                cat_attacked_player_[players_[mouse.first]->character_id_] = true;

                if (players_[mouse.first]->curr_hp_ > 0)
                {
                    players_[mouse.first]->curr_hp_ -= CAT_ATTACK_DAMAGE;
                }

                players_[mouse.first]->RequestUpdate();
            }
        }
    }
}



void GameSession::DeleteCheeseVoxel(const DirectX::XMFLOAT3& center)
{
    // 아직 게임 시작 안되었을때,
    if (false == is_game_start_)
    {
        return;
    }

    int cheese_num = 0;
    DirectX::BoundingSphere sphere{ center, MOUSE_BITE_SIZE };
    bool is_removed = false;

    for (auto& cheese : cheese_octree_)
    {
        if (true == cheese.RemoveVoxel(sphere))
        {
            is_removed = true;

            // 해당 치즈 복셀이 전부 삭제 되었을시,
            if (true == cheese.IsEmpty())
            {
                std::cout << "해당 치즈 전부 삭제 - " << cheese_num << std::endl;
                broken_cheese_num_.emplace_back(cheese_num);
                BroadcastRemoveVoxelSphere(cheese_num, center, true);
            }
            else
            {
                BroadcastRemoveVoxelSphere(cheese_num, center, false);
            }
        }
        cheese_num++;
    }

    if (true == is_removed)
    {
        std::cout << "치즈 삭제 성공" << std::endl;
        if (broken_cheese_num_.size() >= CHEESE_NUM)
        {
            std::cout << "모든 치즈 전부 삭제!" << std::endl;
            RequestSendGameEvent(GAME_EVENT::GE_OPEN_DOOR);
        }
    }
}


void GameSession::InitializeSessionAI()
{
    for(int i = NUM_AI1; i <= NUM_AI4; ++i)
	{
        ai_players_.emplace(i, std::make_unique<AIPlayer>());
		ai_players_[i]->SetID(i);
        ai_players_[i]->position_ = CHARACTER_POS[i];
        ai_players_[i]->SetBoundingSphere();
	}
    std::cout << "Create Session AI - " << session_num_ << std::endl;
    TIMER_EVENT ev{ std::chrono::system_clock::now(), session_num_ };
    AI_Queue.push(ev);
}


bool GameSession::RebornToAI(int player_num)
{
    bool is_reborn = false;
	// AI로 변경
    for (auto& ai : ai_players_)
    {
        if (true == ai.second->is_activate_.load())
        {
            is_reborn = true;
            ai.second->is_activate_.store(false);
            ai.second->path_.clear();
            std::cout << "Reborn to AI - " << ai.second->character_id_ << std::endl;
            players_[player_num]->curr_hp_ = 100;
            //players_[player_num]->position_ = ai.second->position_;
            players_[player_num]->reborn_ai_character_id_ = ai.second->character_id_;

            break;
        }
    }

    return is_reborn;
}

bool GameSession::CheckGameOver()
{
    bool need_update = true;
    // 아직 게임 시작 안되었을때,
    if (false == is_game_start_)
    {
        return need_update;
    }

    bool is_everymouse_dead = true;
    // 살아있는 생쥐가 더이상 없을때
    for (const auto& mouse : alive_mouse_)
    {
        if (mouse.second == false)
        {
            is_everymouse_dead = false;
        }
    }
    if (true == is_everymouse_dead)
    {
        // 탈출한 쥐가 없으면 고양이 승리
        if (escape_mouse_.empty())
        {
            RequestSendGameEvent(GAME_EVENT::GE_WIN_CAT);
            need_update = false;
            is_game_start_ = false;
        }
        // 탈출한 쥐 존재시 생쥐 승리
        else
        {
            RequestSendGameEvent(GAME_EVENT::GE_WIN_MOUSE);
            need_update = false;
            is_game_start_ = false;
        }
    }
    else
    {
        // 문 열리고 나서 탈출 확인
        if (true == is_door_open_)
        {

            bool escape_all = true;
            for (auto& mouse : alive_mouse_)
            {
                if (true == mouse.second)    continue;      // 탈출한 인원은 pass

                DirectX::BoundingSphere sphere{ players_[mouse.first]->position_, 5.0f };

                // 탈출구로 탈출 성공시
                if (true == g_EscapeOBB.Intersects(sphere))
                {
                    std::cout << "player escape - " << mouse.first << std::endl;
                    mouse.second = true;
                    players_[mouse.first]->moveable_ = false;
                    players_[mouse.first]->needs_update_.store(false);
                    players_[mouse.first]->request_send_escape_ = true;
                    escape_mouse_.push_back(mouse.first);
                    RequestSendGameEvent(GAME_EVENT::GE_ESCAPE);
                }
                else
                {
                    escape_all = false;
                }

            }

            // 살아있는 모든 생쥐가 탈출했을시
            if (true == escape_all)
            {
                RequestSendGameEvent(GAME_EVENT::GE_WIN_MOUSE);
                need_update = false;
                is_game_start_ = false;
            }

        }
    }
    return need_update;
}

void GameSession::CheckResult()
{
    // 탈출한 쥐가 없으면, 고양이 승리
    if (escape_mouse_.empty())
    {
        BroadcastCatWin();
    }
    // 탈출한 쥐가 있으면, 쥐 승리
    else
    {
        BroadcastMouseWin();
    }
}

void GameSession::MovePlayerToWaitngSession()
{
    std::cout << "[ " << session_num_ << " ] - 대기 세션으로 이동" << std::endl;
    for (int i = 0; i < SESSION_MAX_USER; ++i)
    {
        {
            std::lock_guard<std::mutex> lg{ players_[i]->mt_player_server_state_ };
            players_[i]->player_server_state_ = PLAYER_STATE::PS_ALLOC;
            players_[i]->ResetPlayer();
        }
        int* temp_session_num = new int(-1);
        int* client_id = new int(GetWaitingPlayerNum());
        if (*client_id != -1)
        {
            // 새로운 세션으로 임시 세션 플레이어 정보 옮기
            g_sessions[*temp_session_num].players_.emplace(*client_id, std::move(players_[i]));
            CompletionKey* completion_key = new CompletionKey{ temp_session_num, client_id };
            g_sessions[*temp_session_num].players_[*client_id]->SetCompletionKey(*completion_key);


            // 대기서버 플레이어 receive
            g_sessions[*temp_session_num].players_[*client_id]->prev_packet_.clear();
            g_sessions[*temp_session_num].players_[*client_id]->DoReceive();
        }
    }
}
