#pragma once
#include "StateMachine.h"
#include "Object.h"

class CMovementComponent;
class CRotationComponent;
class CObbComponent;
class CAnimationCallbackFunc;
class CWeapon;
class CShader;

enum class PlayerAnimationState { Idle = 0, Roll, Run, Walk, 
	SwordIdle, SwordAttack11, SwordAttack12, SwordAttack13, SwordAttack21, SwordAttack22, SwordAttack23, SwordAttack30, SwordAttack40,
	SphereIdle, SphereAttack11, SphereAttack12, SphereAttack20, SphereAttack31, SphereAttack32, SphereAttack33, SphereAttack40
};

//�÷��̾� ���� 4���� None Ÿ���� ���� ���°� �ƴ��� ��Ÿ��
enum class PlayerAttackType { None = 0, LeftAttack, RightAttack, BothAttack, ControlAttack };

class CPlayer : public CRootObject
{
protected:
	float           			m_fPitch;
	float           			m_fYaw;
	float           			m_fRoll;

	float						speed_; // ����: m/s

	bool is_move_allow_ = true;

	XMFLOAT3 direction_vector_{ 0.f,0.f,0.f };

	LPVOID						m_pPlayerUpdatedContext;
	LPVOID						m_pCameraUpdatedContext;

	CCamera* camera_ = NULL;

	//Components
	CMovementComponent* movement_component_ = NULL;
	CRotationComponent* rotation_component_ = NULL;

	bool orient_rotation_to_movement_ = true;

	PlayerAnimationState animation_state_ = PlayerAnimationState::Idle;

	// ���� �÷��̾��� ������¸� ��Ÿ�� None�� ����
	WeaponType current_weapon_type_ = WeaponType::None;

	CGameObject* weapon_socket_ = NULL;

	//���׸� �ع� ���� ����
	bool is_ether_ = false;
	std::array<CGameObject*, 4> ether_weapon_sockets_;


	std::vector<CWeapon*> weapons_;

	PlayerAttackType attack_type_ = PlayerAttackType::None;
	StateMachine<CPlayer>* state_machine_;


	//[CS] �ڽ��� ��ȣ
	// ** �ϴ� 0������ �ʱ�ȭ��. �ʱ�ȭ ������ ã���ָ� �ش� ��ȣ�� set�ϴ� �ڵ带 ã�Ƽ� ������
	int my_id_ = 0;

public:
	CPlayer();
	CPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual ~CPlayer();

	//setter
	void set_is_ether(const bool& value) { is_ether_ = value; }
	void set_is_move_allow(const bool& value) { is_move_allow_ = value; }
	void set_attack_type(const PlayerAttackType& value) { attack_type_ = value; }
	void set_current_weapon(const WeaponType& value) { current_weapon_type_ = value; }
	void set_animation_state(const PlayerAnimationState& value) { animation_state_ = value; }
	void set_weapon_socket(CGameObject* value) { weapon_socket_ = value; }

	//getter
	bool is_ether() const { return is_ether_; }
	WeaponType current_weapon() const { return current_weapon_type_; }
	CGameObject* weapon_socket() const { return weapon_socket_; }
	StateMachine<CPlayer>* state_machine() const { return state_machine_; }
	CAnimationController* animation_controller() const { return animation_controller_; }
	float speed() const { return speed_; }
	// �߰�+ -  another���� ���� ���� ���� const ��
	CMovementComponent* movement_component() { return movement_component_; }
	//float rotation_component_yaw() const { return rotation_component_->yaw(); }
	bool orient_rotation_to_movement() const { return orient_rotation_to_movement_; }
	PlayerAnimationState animation_state() const { return animation_state_; }
	float GetYaw() const { return(m_fYaw); }
	float GetPitch() const { return(m_fPitch); }
	float GetRoll() const { return(m_fRoll); }

	// �Է� ó��
	void InputActionMove(const DWORD& dwDirection, const float& elapsed_time);
	void InputActionRotate(const XMFLOAT2& delta_xy, const float& elapsed_time);
	void InputActionRoll(const DWORD& direction);
	void InputActionAttack(const PlayerAttackType& attack_type);

	void AddWeapon(CWeapon* ptr) { weapons_.push_back(ptr); }
	void EquipWeapon(const std::string& name);

	CCamera *GetCamera() { return(camera_); }
	void SetCamera(CCamera *pCamera) { camera_ = pCamera; }

	virtual void Update(float fTimeElapsed);

	// �ð��� ���� ȸ��
	virtual void OrientRotationToMove(float fTimeElapsed);
	// ���� ȸ�� ������ŭ ��� ȸ��
	void OrientRotationToMove();

	void OrientRotationAnothers (int anther_num);

	void UpdateAnothers(float elapsed_time, int player_num);

	virtual void HandleCollision(CRootObject* other, const CObbComponent& my_obb, const CObbComponent& other_obb) override;

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) { }
	void SetPlayerUpdatedContext(LPVOID pContext) { m_pPlayerUpdatedContext = pContext; }

	virtual void OnCameraUpdateCallback(float fTimeElapsed) { }
	void SetCameraUpdatedContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);

	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) { return(NULL); }
	virtual void OnPrepareRender() {}
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera = NULL);

	void SetAnimationCallbackKey(const float& index, const float& time, CAnimationCallbackFunc* func);

	// ���׸� ���� ���� �Լ�
	void SetEtherWeaponSocketByShader(CShader* shader);
	void SpawnEtherWeapon();
	void DespawnEtherWeapon();
	void UpdateEtherWeapon(float elapsed_time);

	void SendInput(uint8_t& input);
	void SendSkill(bool skillend);

	void set_my_id(int id) { my_id_ = id; }

	int GetMyId() { return my_id_; }
};
