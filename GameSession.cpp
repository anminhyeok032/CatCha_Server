#include "GameSession.h"
#include "Player.h" 
#include "CatPlayer.h"
#include "MousePlayer.h"
#include "AIPlayer.h"

void GameSession::Update()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_players = 0;
   
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // ������ �÷��̾��� Postion�� ������Ʈ �ϵ��� int 5������ ������ ���θ� �Ľ��ؼ� ����
    for (auto& pl : players_)
    {
        // ������Ʈ ��û�� ������
        if(pl.second->needs_update_.load() == false) continue;

        if (true == pl.second->UpdatePosition(deltaTime))
        {
            need_update_send = true;
            move_players |= (1 << pl.first);
            //std::cout << "Player " << pl.first << " Move" << std::endl;
            //std::cout << "Position : " << pl.second->position_.x << ", " << pl.second->position_.y << ", " << pl.second->position_.z << std::endl;
        }
    }

    // ���� ���� �ð� ������Ʈ
    lastupdatetime_ = current_time;

    // ������ ������Ʈ�� �ʿ��Ҷ��� Ŭ���̾�Ʈ���� ��ε�ĳ����
    if (true == need_update_send)
    {
        SendPlayerUpdate(move_players);
    }
    // ������ �ֱ�� �����ϱ� ���� queue�� �ٽ� ���
    TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(UPDATE_PERIOD_INT), session_num_ };
    commandQueue.push(ev);
    
}

void GameSession::UpdateAI()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_AIs = 0;

    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // ������ AI�� Postion�� ������Ʈ �ϵ��� int 4������ ������ ���θ� �Ľ��ؼ� ����
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

    // ���� ���� �ð� ������Ʈ
    lastupdatetime_ = current_time;

    // ������ ������Ʈ�� �ʿ��Ҷ��� Ŭ���̾�Ʈ���� ��ε�ĳ����
    if (true == need_update_send)
    {
        SendAIUpdate(move_AIs);
    }
    // ������ �ֱ�� �����ϱ� ���� queue�� �ٽ� ���
    TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(AI_UPDATE_PERIOD_INT / 2), session_num_ };
    AI_Queue.push(ev);
}


void GameSession::SendPlayerUpdate(int move_players)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_MOVE;
    int* players = new int(move_players);
    CompletionKey* completion_key = new CompletionKey{ &session_num_, players };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}


void GameSession::BroadcastGameStart()
{
    SC_GAME_STATE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_GAME_START;

    // Timer ������ ����
    remaining_time_ = GAME_TIME + FREEZING_TIME;
    BroadcastTime();

    // �÷��̾� ���� �ʱ�ȭ
	for (auto& pl : players_)
	{
        // ĳ���� ���� �ٽ� �ʱ�ȭ
        pl.second->position_        = CHARACTER_POS[pl.second->character_id_];
        pl.second->rotation_quat_   = CHARACTER_ROTATION[pl.second->character_id_];
        pl.second->velocity_vector_ = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->force_vector_    = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->depth_delta_     = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        pl.second->dirty_ = true;
        pl.second->keyboard_input_.clear();
        pl.second->UpdateLookUpRight();

        // ĳ���� ���� �ʱ�ȭ
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
    // tick���� ���� pitch ����
    p.player_pitch = pl->total_pitch_;
    // ���� pitch �ʱ�ȭ
    pl->total_pitch_ = 0.0f;
    
    // state�� cat_attacked�� �Ľ��ؼ� unsigned char�� ��ȯ
    unsigned char state_value = static_cast<unsigned char>(pl->obj_state_);
    // cat_attacked�� ������ ��Ʈ�� ����
    p.state = (state_value << 1) 
        | (cat_attacked_player_[pl->character_id_] ? 1 : 0);

    if (pl->stop_skill_time_ > 0.001f)
    {
        // ��ų Ÿ�� ��������� ��ٸ�
    }
    // ���������� ������, ���� idle�� ����
    else if (pl->obj_state_ == Object_State::STATE_JUMP_START)
    {
        players_[player]->obj_state_ = Object_State::STATE_JUMP_IDLE;
    }
    // ���� end ������, ���� ������Ʈ (idle||MOVE)���� && �����ϼ� �ִ� ��Ȳ�϶�
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


    // ��� Ŭ���̾�Ʈ���� ��ε�ĳ����
    for (auto& pl : players_)
    {
        pl.second->DoSend(&p);
    }
    
}

void GameSession::BroadcastSync()
{
    for (auto& pl : players_) // ���ǿ� ���� Ŭ���̾�Ʈ ���
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

        for (auto& player : players_) // ���ǿ� ���� Ŭ���̾�Ʈ ���
        {
            if(pl.first == player.first) continue; // �ڱ� �ڽ� ����
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

void GameSession::BroadcastRemoveVoxelSphere(int cheese_num, const DirectX::XMFLOAT3& center)
{
    SC_REMOVE_VOXEL_SPHERE_PACKET p;
	p.size = sizeof(p);
	p.type = SC_REMOVE_VOXEL_SPHERE;
	p.cheese_num = cheese_num;
	p.center_x = center.x;
	p.center_y = center.y;
	p.center_z = center.z;

	for (auto& pl : players_)
	{
		pl.second->DoSend(&p);
	}
}

void GameSession::SendAIUpdate(int move_AIs)
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
    
    if (remaining_time_ > 0)
    {
        TIMER_EVENT ev{ std::chrono::system_clock::now() + std::chrono::milliseconds(1000), session_num_ };
        timer_queue.push(ev);
    }
    else
    {
        // TODO : ���� ���� ���� ����
    }
}

uint64_t GameSession::GetServerTime()
{
    // ���� �ð� ��������
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}


void GameSession::SetCharacter(int room_num, int client_index, bool is_cat) 
{
    auto& player = players_[client_index];

    // �ڽ��� �÷��̾� ��ȣ ����
    player->SendMyPlayerNumber();

    // ������� ��
    if (is_cat) 
    {
        player->SetState(std::make_unique<CatPlayer>());
        player->SetID(NUM_CAT);
        player->Set_OBB(player->character_state_->GetOBB());
    }
    // ���� ��
    else 
    {
        player->SetState(std::make_unique<MousePlayer>());
        player->SetID(GetMouseNum());
        player->Set_OBB(player->character_state_->GetOBB());
    }

    // ���� ������ �÷��̾��Ͻ�
    // - ���ο� ���� Ŭ���̾�Ʈ���� ���� Ŭ���̾�Ʈ ���� ����
    // - ���� Ŭ���̾�Ʈ ���� ���ο� Ŭ���̾�Ʈ���� ����
    {
        // ���ÿ� ���� �����忡�� ĳ���� ����
        std::lock_guard<std::mutex> lock(mt_session_state_);
        for (auto& pl : players_)
        {
            if (client_index == *pl.second->comp_key_.player_index) continue;              // �ڱ� �ڽ�
            if (pl.second->character_id_ == NUM_GHOST) continue;       		               // ���� ����    
            BroadcastAddCharacter(client_index, *pl.second->comp_key_.player_index);
            BroadcastAddCharacter(*pl.second->comp_key_.player_index, client_index);
        }
    }

    // ĳ���� ��ü ��ε�ĳ��Ʈ
    BroadcastChangeCharacter(client_index, players_[client_index]->character_id_);

    // ���� �� ���� ���� ����
    {
        std::lock_guard <std::mutex> lg{ mt_session_state_ };
        if (session_state_ == SESSION_STATE::SESSION_FULL)
        {
            // ���� ����
            BroadcastGameStart();
            // AI ����
            InitializeSessionAI();
        }
    }

    player->RequestUpdate();
}

int GameSession::GetMouseNum()
{
    // ��� ���� ���콺 ID Ȯ�ο� array
    std::array<bool, 4> mouse_ids_check = { false, false, false, false };

    {
        std::lock_guard<std::mutex> lock(mt_session_state_);
        // ���� �÷��̾���� ID Ȯ�� �� üũ
        for (const auto& pl : players_)
        {
            int id = pl.second->character_id_;
            if (id >= NUM_MOUSE1 && id <= NUM_MOUSE4)
            {
                mouse_ids_check[id] = true;
            }
        }

        // ��� ������ ���콺 ID return
        for (int i = 0; i < 4; ++i)
        {
            if (false == mouse_ids_check[i])
            {
                return i;
            }
        }
    }

    // ��� ���콺 ID�� ��� ���̶�� -1 ��ȯ (����)
    std::cout << session_num_ << "�� ���� : ��� ���콺 ID�� ��� ��!!" << std::endl;
    return -1;
}


void GameSession::CheckAttackedMice()
{
    // ��� �� �˻�
    for (auto& mouse : players_)
    {
        if(mouse.second->character_id_ == NUM_CAT) continue; // ����̴� ����

        for (const auto& mouse_attacked : cat_attacked_player_)
        {
            // ���� ���� ��
            if (mouse_attacked.second == true)
            {
                // �ڱ� �ڽ��̸� �ߺ� ���� ����
                if (mouse.second->character_id_ == mouse_attacked.first)
                {
                    return;
                }
            }
        }

        // ������� ���� OBB�� ���� OBB �浹 �˻�
        if (mouse.second->character_state_)
        {
            if (true == cat_attack_obb_.Intersects(mouse.second->character_state_->GetOBB()))
            {

                mouse.second->velocity_vector_.x = cat_attack_direction_.x * CAT_PUNCH_POWER;
                mouse.second->velocity_vector_.y = CAT_PUNCH_POWER / 2.0f;
                mouse.second->velocity_vector_.z = cat_attack_direction_.z * CAT_PUNCH_POWER;
                std::cout << "Cat Attack Success : mouse - " << mouse.second->character_id_ << std::endl;
                cat_attacked_player_[mouse.second->character_id_] = true;

                if (mouse.second->curr_hp_ > 0)
                {
                    mouse.second->curr_hp_ -= CAT_ATTACK_DAMAGE;
                }

                mouse.second->RequestUpdate();
            }
        }
    }
}



void GameSession::DeleteCheeseVoxel(const DirectX::XMFLOAT3& center)
{
    int cheese_num = 0;
    DirectX::BoundingSphere sphere{ center, 5.0f };
    bool is_removed = false;

    for (auto& cheese : cheese_octree_)
    {
        if (true == cheese.RemoveVoxel(sphere))
        {
            is_removed = true;
            BroadcastRemoveVoxelSphere(cheese_num, center);
        }
        cheese_num++;
    }

    if (true == is_removed)
    {
        std::cout << "ġ�� ���� ����" << std::endl;
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
	// AI�� ����
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