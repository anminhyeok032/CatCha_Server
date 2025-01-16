#include "GameSession.h"
#include "Player.h" 
#include "CatPlayer.h"
#include "MousePlayer.h"

void GameSession::Update()
{
    bool need_update_send = false;
    uint64_t current_time = GetServerTime();
    int move_players = 0;
   
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // ������ �÷��̾��� Postion�� ������Ʈ �ϵ��� int 8������ ������ ���θ� �Ľ��ؼ� ����
    for (auto& pl : players_)
    {
        // ������Ʈ ��û�� ������
        if(pl.second->needs_update_.load() == false) continue;

        if (true == pl.second->UpdatePosition(deltaTime))
        {
            need_update_send = true;
            move_players |= (1 << pl.second->comp_key_.player_index);
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
        | (cat_attacked_player_[pl->id_] ? 1 : 0);

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
    // ���� �ð� ��������
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}


void GameSession::SetCharacter(int room_num, int client_index, bool is_cat) 
{
    auto& player = players_[client_index];

    if (player->id_ == NUM_GHOST)
    {
        // ���ÿ� ���� �����忡�� ĳ���� ����
        std::lock_guard<std::mutex> lock(mt_session_state_);
        for (auto& pl : players_)
        {
            if(client_index == pl.second->comp_key_.player_index) continue;              // �ڱ� �ڽ�
            if(pl.second->id_ == NUM_GHOST) continue;       		                     // ���� ����    
            BroadcastAddCharacter(client_index, pl.second->comp_key_.player_index);
            BroadcastAddCharacter(pl.second->comp_key_.player_index, client_index);
        }
    }

    // ������� ��
    if (is_cat) 
    {
        player->SetState(std::make_unique<CatPlayer>());
        player->SetID(NUM_CAT);
        player->Set_OBB(player->state_->GetOBB());
    }
    // ���� ��
    else 
    {
        player->SetState(std::make_unique<MousePlayer>());
        player->SetID(GetMouseNum());
        player->Set_OBB(player->state_->GetOBB());
    }

    // ĳ���� ��ü ��ε�ĳ��Ʈ
    BroadcastChangeCharacter(client_index, players_[client_index]->id_);
    player->RequestUpdate();
}

int GameSession::GetMouseNum()
{
    // ��� ���� ���콺 ID Ȯ�ο� array
    std::array<bool, 4> mouse_ids_check = { false, false, false, false };

    // ���� �÷��̾���� ID Ȯ�� �� üũ
    for (const auto& pl : players_) 
    {
        int id = pl.second->id_; 
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

    // ��� ���콺 ID�� ��� ���̶�� -1 ��ȯ (����)
    std::cout << session_num_ << "�� ���� : ��� ���콺 ID�� ��� ��!!" << std::endl;
    return -1;
}


void GameSession::CheckAttackedMice()
{
    // ��� �� �˻�
    for (auto& mouse : players_)
    {
        if(mouse.second->id_ == NUM_CAT) continue; // ����̴� ����

        for (const auto& mouse_attacked : cat_attacked_player_)
        {
            // ���� ���� ��
            if (mouse_attacked.second == true)
            {
                // �ڱ� �ڽ��̸� �ߺ� ���� ����
                if (mouse.second->id_ == mouse_attacked.first)
                {
                    return;
                }
            }
        }

        // ������� ���� OBB�� ���� OBB �浹 �˻�
        if (state_)
        {
            if (true == cat_attack_obb_.Intersects(mouse.second->state_->GetOBB()))
            {

                mouse.second->velocity_vector_.x = cat_attack_direction_.x * CAT_PUNCH_POWER;
                mouse.second->velocity_vector_.y = CAT_PUNCH_POWER / 2.0f;
                mouse.second->velocity_vector_.z = cat_attack_direction_.z * CAT_PUNCH_POWER;
                std::cout << "Cat Attack Success : mouse - " << mouse.second->id_ << std::endl;
                cat_attacked_player_[mouse.second->id_] = true;
                mouse.second->RequestUpdate();
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
            std::cout << "ġ�� ���� ����" << std::endl;
            //cheese_octree_.PrintNode();

            BroadcastRemoveVoxelSphere(center);
        }
        else
        {
            std::cout << "ġ�� ���� ����" << std::endl;
        }
    }

}