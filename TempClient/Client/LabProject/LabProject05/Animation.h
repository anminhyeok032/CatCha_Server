#pragma once

#include "AnimationCallbackFunc.h"

class CGameObject;

class CAnimationCurve
{
public:
	CAnimationCurve();
	CAnimationCurve(std::ifstream& InFile);
	~CAnimationCurve() {}

	void GetAnimatedSRT(const float& fPosition, const float& fLayerWeight,
		XMFLOAT3* pScale, XMFLOAT3* pRotate, XMFLOAT3* pTranslation);

public:
	std::string m_strFrameName;

private:
	void LoadKeys(std::ifstream& InFile, std::vector<XMFLOAT3>& vecKeyValues);

	std::vector<float> m_KeyTimes;
	std::vector<XMFLOAT3> m_Scales;
	std::vector<XMFLOAT3> m_Rotates;
	std::vector<XMFLOAT3> m_Translations;
};

class CAnimationLayer
{
public:
	CAnimationLayer();
	CAnimationLayer(std::ifstream& InFile);
	~CAnimationLayer() {}

	void SetFrameCaches(CGameObject* pRootObject);
	std::vector<CGameObject*>& GetFrameCaches() { return m_FrameCaches; }

	void UpdateFrameCachesSRT(const float& fPosition);

private:
	float weight_;

	std::vector<CAnimationCurve> m_Curves;
	std::vector<CGameObject*> m_FrameCaches;
};

class CAnimationSet
{
public:
	CAnimationSet();
	CAnimationSet(const float& fStartTime, const float& fEndTime, const std::string& strAnimationName);
	~CAnimationSet(){}

	void LoadLayersFromFile(std::ifstream& InFile);

	//setter
	void set_start_time(const float& value) { start_time_ = value; }
	void set_end_time(const float& value) { end_time_ = value; }
	void set_animation_name(const std::string& value) { animation_name_ = value; }
	void SetFrameCaches(CGameObject* pRootObject);

	float end_time() const { return end_time_; }

	void Animate(const float& fPosition, const float& fTrackWeight);
	void UpdateMatrix();

private:
	float start_time_;
	float end_time_;

	std::string animation_name_;

	std::vector<CAnimationLayer> m_Layers;
};

enum class AnimationLoopType{ Once, Repeat };

struct CallbackKey
{
	float time = 0.f;
	CAnimationCallbackFunc* callback_func;

	CallbackKey(const float& t, CAnimationCallbackFunc* func) { time = t, callback_func = func; }

	void operator()() { callback_func->Util(); }
};

class CAnimationTrack
{
public:
	CAnimationTrack();
	CAnimationTrack(std::ifstream& InFile);
	~CAnimationTrack();

	bool IsEnable() { return enable_; }

	//setter
	void set_auto_reset(const bool& value) { auto_reset_ = value; }
	void set_position(const float& value) { position_ = value; }
	void set_enable(const bool& value) { enable_ = value; }
	void set_weight(const float& value) { weight_ = value; }
	void set_loop_type(const AnimationLoopType& type) { loop_type_ = type; }
	void set_speed(const float& value) { speed_ = value; }

	//getter
	float end_time() const { return animation_set_->end_time(); }
	float weight() const { return weight_; }

	void AddWeight(const float& value);
	void AddCallbackKey(const float& time, CAnimationCallbackFunc* callback_func);

	void Animate(const float& fElapsedTime);

	void SetFrameCaches(CGameObject* pRootObject);
	void UpdateMatrix();

	void Stop() { speed_ = 0.f; }
	void Start() { speed_ = 1.f; }

private:
	void UpdatePosition(const float& fElapsedTime, bool reset);

	// 콜백을 체크하는 시간 callback_key.time +- callback_check_time
	static const float callback_check_time_;

	float position_;
	float speed_;
	float weight_;

	bool enable_;
	bool auto_reset_ = true;

	AnimationLoopType loop_type_;

	//05.09 수정: 컨트롤러를 복사할때 애니메이션 set 정보는 복사될 필요없음
	std::shared_ptr<CAnimationSet> animation_set_;

	std::vector<CallbackKey> callback_keys_;
};

class CAnimationController
{
private:
	const float animation_blend_speed_ = 5.f; // 애니메이션 교체시 교체 속도 단위 once/s

	bool is_animation_chainging_ = false;
	bool is_blend_change_ = true; // 애니메이션 교체시 블렌딩 여부
	bool is_end_time_blending_ = false; // end 프레임과 블렝딩

	int prev_index_ = 0;
	int curr_index_ = 0;

	float m_fTime = 0;

	std::vector<CAnimationTrack> animation_tracks_;

public:
	CAnimationController();
	~CAnimationController() {}

	//setter
	void set_is_blend_change(const bool& value) { is_blend_change_ = value; }
	void set_is_end_time_blending(const bool& value) { is_end_time_blending_ = value; }
	
	//getter
	bool is_blend_change() const { return is_blend_change_; }
	bool is_animation_chainging() const { return is_animation_chainging_; }

	//로드한 애니메이션의 수를 반환
	int LoadAnimationFromFile(std::ifstream& InFile);

	void Animate(const float& fElapsedTime, CGameObject* pRootObject);

	void SetFrameCaches(CGameObject* pRootObject);

	void EnableTrack(const int& index);

	void ChangeAnimation(const int& index);

	void SetCallbackKey(const int& index, const float& time, CAnimationCallbackFunc* callback_func);

	void SetLoopType(const int& index, const AnimationLoopType& type);

	bool IsEnableTrack(const int& index) { return animation_tracks_[index].IsEnable(); }

	void PositionReset(const int& index) { animation_tracks_[index].set_position(0.f); }

	void SetTrackSpeed(const int& index, const float& speed);

};

