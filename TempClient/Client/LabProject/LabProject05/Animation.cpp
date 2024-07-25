#include "stdafx.h"
#include "Animation.h"
#include "Object.h"

CAnimationController::CAnimationController()
{
	m_fTime = 0.f;
	animation_tracks_.reserve(0);
}

int CAnimationController::LoadAnimationFromFile(std::ifstream& InFile)
{
	std::string strToken;

	FBXLoad::ReadStringFromFile(InFile, strToken);

	int nAnimationSets = 0;
	if (strToken == "<AnimationSets>:")
		nAnimationSets = FBXLoad::ReadFromFile<int>(InFile);

	animation_tracks_.reserve(nAnimationSets);

	for (int i = 0; i < nAnimationSets; ++i)
	{
		animation_tracks_.emplace_back(InFile);
	}

	FBXLoad::ReadStringFromFile(InFile, strToken); // </AnimationSets>

	return nAnimationSets;
}

void CAnimationController::Animate(const float& elapsed_time, CGameObject* root_object)
{
	if (is_animation_chainging_)
	{
		if (is_end_time_blending_)
			animation_tracks_[prev_index_].set_position(animation_tracks_[prev_index_].end_time());
		animation_tracks_[prev_index_].AddWeight(-animation_blend_speed_ * elapsed_time);
		animation_tracks_[curr_index_].AddWeight(animation_blend_speed_ * elapsed_time);
		if (!animation_tracks_[prev_index_].IsEnable())
		{
			if (is_end_time_blending_)
			{
				animation_tracks_[prev_index_].set_position(0.f);
				animation_tracks_[prev_index_].set_auto_reset(true);
			}
			animation_tracks_[prev_index_].Start();
			animation_tracks_[curr_index_].Start();
			is_animation_chainging_ = false;
		}
	}

	for (auto& track : animation_tracks_)
	{
		if (track.IsEnable())
		{
			track.Animate(elapsed_time);
		}
	}

	for (auto& track : animation_tracks_)
	{
		if (track.IsEnable())
		{
			track.UpdateMatrix();
		}
	}

}

void CAnimationController::SetFrameCaches(CGameObject* pRootObject)
{
	for (auto& track : animation_tracks_)
		track.SetFrameCaches(pRootObject);
}

void CAnimationController::EnableTrack(const int& index)
{
	// 3개의 애니메이션이 동시에 활성화되는 것을 방지
	animation_tracks_[prev_index_].set_enable(false);

	prev_index_ = curr_index_;
	curr_index_ = index;
	if(is_blend_change_)
		animation_tracks_[prev_index_].set_enable(true);
	else
		animation_tracks_[prev_index_].set_enable(false);

	animation_tracks_[index].set_enable(true);
}

void CAnimationController::ChangeAnimation(const int& index)
{
	if (curr_index_ == index)
		return;


	// 해당 애니메이션 트랙으로 전환
	EnableTrack(index);
	if (!is_blend_change_)
	{
		return;
	}

	// 트랙 블렌딩간 일시정지
	animation_tracks_[prev_index_].Stop();
	animation_tracks_[index].Stop();

	if (is_end_time_blending_)
		animation_tracks_[prev_index_].set_auto_reset(false);

	// 현재 애니메이션을 이전 애니메이션 weight의 남은 부분 만큼 설정
	animation_tracks_[index].set_weight(1.f - animation_tracks_[prev_index_].weight());
	// 애니메이션 전환 트리거를 true로 설정
	is_animation_chainging_ = true;
}

void CAnimationController::SetLoopType(const int& index, const AnimationLoopType& type)
{
	animation_tracks_[index].set_loop_type(type);
}

void CAnimationController::SetCallbackKey(const int& index, const float& time, CAnimationCallbackFunc* callback_func)
{
	animation_tracks_[index].AddCallbackKey(time, callback_func);
}

void CAnimationController::SetTrackSpeed(const int& index, const float& speed)
{
	animation_tracks_[index].set_speed(speed);
}

const float CAnimationTrack::callback_check_time_ = 1.f / 120.f;

CAnimationTrack::CAnimationTrack()
{
	position_ = 0.f;
	speed_ = 1.f;
	enable_ = false;
	weight_ = 1.0f;

	loop_type_ = AnimationLoopType::Once;

}

CAnimationTrack::CAnimationTrack(std::ifstream& InFile) : CAnimationTrack()
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);
	if (strToken == "<AnimationSet>:")
	{
		FBXLoad::ReadFromFile<float>(InFile); // index

		FBXLoad::ReadStringFromFile(InFile, strToken); // animation name

		float start_time, end_time;
		start_time = FBXLoad::ReadFromFile<float>(InFile);
		end_time = FBXLoad::ReadFromFile<float>(InFile);


		animation_set_.reset(new CAnimationSet(start_time, end_time, strToken));
		animation_set_->LoadLayersFromFile(InFile);
	}

	FBXLoad::ReadStringFromFile(InFile, strToken); // </AnimationSet>
}

CAnimationTrack::~CAnimationTrack()
{
}

void CAnimationTrack::AddWeight(const float& value)
{
	weight_ += value;
	if (weight_ < 0.f)
	{
		weight_ = 1.f;
		enable_ = false;
	}
	else if (weight_ > 1.f) 
		weight_ = 1.f;
}

void CAnimationTrack::AddCallbackKey(const float& time, CAnimationCallbackFunc* func)
{
	callback_keys_.emplace_back(time, func);
}

void CAnimationTrack::Animate(const float& fElapsedTime)
{
	UpdatePosition(fElapsedTime, auto_reset_);

	for (auto& callback_key : callback_keys_)
	{
		if (callback_check_time_ + callback_key.time > position_ && position_ > callback_key.time - callback_check_time_)
			callback_key();
	}

	animation_set_->Animate(position_, weight_);
}

void CAnimationTrack::SetFrameCaches(CGameObject* pRootObject)
{
	animation_set_->SetFrameCaches(pRootObject);
}

void CAnimationTrack::UpdateMatrix()
{
	animation_set_->UpdateMatrix();
}

void CAnimationTrack::UpdatePosition(const float& fElapsedTime, bool reset)
{
	position_ += (fElapsedTime * speed_);

	if (!reset) return;

	if (animation_set_->end_time() < position_)
	{
		
		if (loop_type_ == AnimationLoopType::Once) 
		{
			enable_ = false;
		}
		position_ = 0.f;
	}
}



CAnimationSet::CAnimationSet()
{
	start_time_ = 0.f;
	end_time_ = 0.f;

	m_Layers.reserve(0);

}

CAnimationSet::CAnimationSet(const float& fStartTime, const float& fEndTime, const std::string& strAnimationName)
	: start_time_(fStartTime), end_time_(fEndTime), animation_name_(strAnimationName)
{
}

void CAnimationSet::LoadLayersFromFile(std::ifstream& InFile)
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);
	if (strToken == "<AnimationLayers>:")
	{
		int nLayers = FBXLoad::ReadFromFile<int>(InFile);
		m_Layers.reserve(nLayers);
		for (int i = 0; i < nLayers; ++i)
		{
			m_Layers.emplace_back(InFile);
		}
	}

	FBXLoad::ReadStringFromFile(InFile, strToken); // </AnimationLayers>
}

void CAnimationSet::SetFrameCaches(CGameObject* pRootObject)
{
	for (auto& layer : m_Layers)
		layer.SetFrameCaches(pRootObject);
}

void CAnimationSet::Animate(const float& fPosition, const float& fTrackWeight)
{
	for (auto& Layer : m_Layers)
	{
		Layer.UpdateFrameCachesSRT(fPosition);
	}

	//Blending
	for (auto& Layer : m_Layers)
	{
		for (auto& Frame : Layer.GetFrameCaches())
		{
			if (!Frame) continue;
			Frame->SetBlendedScale(Vector3::Add(Frame->GetBlendedScale(), Frame->GetScale(), fTrackWeight));
			Frame->SetBlendedRotation(Vector3::Add(Frame->GetBlendedRotation(), Frame->GetRotation(), fTrackWeight));
			Frame->SetBlendedTranslation(Vector3::Add(Frame->GetBlendedTranslation(), Frame->GetTranslation(), fTrackWeight));
		}
	}

}

void CAnimationSet::UpdateMatrix()
{
	//Make Matrix By SRT
	for (auto& Layer : m_Layers)
	{
		for (auto& Frame : Layer.GetFrameCaches())
		{
			if (!Frame) continue;
			Frame->UpdateMatrixByBlendedSRT();
		}
	}
	
}

CAnimationLayer::CAnimationLayer()
{
	weight_ = 1.f;

	m_Curves.reserve(0);
}

CAnimationLayer::CAnimationLayer(std::ifstream& InFile) : CAnimationLayer()
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);

	if (strToken == "<AnimationLayer>:")
	{
		FBXLoad::ReadFromFile<int>(InFile); // index

		int nCurves = FBXLoad::ReadFromFile<int>(InFile);

		weight_ = FBXLoad::ReadFromFile<float>(InFile);

		m_Curves.reserve(nCurves);
		m_FrameCaches.resize(nCurves);

		for (int i = 0; i < nCurves; ++i)
		{
			m_Curves.emplace_back(InFile);
		}

	}
	FBXLoad::ReadStringFromFile(InFile, strToken); // </AnimationLayer>

}

void CAnimationLayer::SetFrameCaches(CGameObject* pRootObject)
{
	for (int i =0; i < m_Curves.size(); ++i)
	{
		m_FrameCaches[i] = pRootObject->FindFrame(m_Curves[i].m_strFrameName);
	}
}

void CAnimationLayer::UpdateFrameCachesSRT(const float& fPosition)
{
	for (int i = 0 ; i < m_Curves.size(); ++i)
	{
		CGameObject* pFrameCache = m_FrameCaches[i];
		if (!pFrameCache) continue;
		XMFLOAT3 S, R, T;
		m_Curves[i].GetAnimatedSRT(fPosition, weight_, &S, &R, &T);
		pFrameCache->SetScale(S);
		pFrameCache->SetRotation(R);
		pFrameCache->SetTranslation(T);
	}
}

CAnimationCurve::CAnimationCurve()
{
	m_strFrameName = "-";
	m_KeyTimes.reserve(0);
	m_Scales.reserve(0);
	m_Rotates.reserve(0);
	m_Translations.reserve(0);

}

CAnimationCurve::CAnimationCurve(std::ifstream& InFile): CAnimationCurve()
{
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken);

	if (strToken == "<AnimationCurve>:")
	{
		FBXLoad::ReadFromFile<int>(InFile); //index

		FBXLoad::ReadStringFromFile(InFile, strToken);

		m_strFrameName = strToken;

		FBXLoad::ReadStringFromFile(InFile, strToken); 

		while (strToken != "</AnimationCurve>")
		{
			if (strToken == "<TX>:") LoadKeys(InFile, m_Translations);
			if (strToken == "<RX>:") LoadKeys(InFile, m_Rotates);
			if (strToken == "<SX>:") LoadKeys(InFile, m_Scales);

			FBXLoad::ReadStringFromFile(InFile, strToken);
		}
	}
}

void CAnimationCurve::GetAnimatedSRT(const float& fPosition, const float& fLayerWeight, XMFLOAT3* pScale, XMFLOAT3* pRotate, XMFLOAT3* pTranslation)
{
	for (int i = 0; i < m_KeyTimes.size() - 1; ++i)
	{
		if (m_KeyTimes[i] <= fPosition && m_KeyTimes[i + 1] > fPosition)
		{
			float t = (fPosition - m_KeyTimes[i]) / (m_KeyTimes[i + 1] - m_KeyTimes[i]);

			// (S1 * (1.0 - t) + S2 * t) * LayerWeight
			if (m_KeyTimes.size() == m_Scales.size()) 
				*pScale = Vector3::ScalarProduct(Vector3::Add(Vector3::ScalarProduct(m_Scales[i], 1.0f - t, false), Vector3::ScalarProduct(m_Scales[i + 1], t, false)), fLayerWeight, false);
			if (m_KeyTimes.size() == m_Rotates.size()) 
				*pRotate = Vector3::ScalarProduct(Vector3::Add(Vector3::ScalarProduct(m_Rotates[i], 1.0f - t, false), Vector3::ScalarProduct(m_Rotates[i + 1], t, false)), fLayerWeight, false);
			if (m_KeyTimes.size() == m_Translations.size()) 
				*pTranslation = Vector3::ScalarProduct(Vector3::Add(Vector3::ScalarProduct(m_Translations[i], 1.0f - t, false), Vector3::ScalarProduct(m_Translations[i + 1], t, false)), fLayerWeight, false);
			
			return;
		}
	}

	// S * LayerWeight
	if (m_KeyTimes.size() == m_Scales.size()) *pScale = Vector3::ScalarProduct(m_Scales[m_Scales.size() - 1], fLayerWeight, false);
	if (m_KeyTimes.size() == m_Rotates.size()) *pRotate = Vector3::ScalarProduct(m_Rotates[m_Rotates.size() - 1], fLayerWeight, false);
	if (m_KeyTimes.size() == m_Translations.size()) *pTranslation = Vector3::ScalarProduct(m_Translations[m_Translations.size() - 1], fLayerWeight, false);

}

void CAnimationCurve::LoadKeys(std::ifstream& InFile, std::vector<XMFLOAT3>& vecKeyValues)
{
	int nKeys = FBXLoad::ReadFromFile<int>(InFile); 

	vecKeyValues.reserve(nKeys);

	float* pBuffer = new float[nKeys * 2]; // fkeytimes, fkeyvalues

	//X
	FBXLoad::ReadFromFile(InFile, pBuffer, nKeys * 2);
	if (nKeys != m_KeyTimes.size())
	{
		m_KeyTimes.reserve(nKeys);
		for (int i = 0; i < nKeys; ++i)
			m_KeyTimes.emplace_back(pBuffer[i]);
	}

	for (int i = nKeys; i < nKeys * 2; ++i) vecKeyValues.emplace_back(pBuffer[i], 0.f, 0.f);


	//Y
	std::string strToken;
	FBXLoad::ReadStringFromFile(InFile, strToken); //<TY>
	FBXLoad::ReadFromFile<int>(InFile);

	FBXLoad::ReadFromFile(InFile, pBuffer, nKeys * 2);

	for (int i = nKeys; i < nKeys * 2; ++i) vecKeyValues[i - nKeys].y = pBuffer[i];
	


	//Z
	FBXLoad::ReadStringFromFile(InFile, strToken); //<TZ>
	FBXLoad::ReadFromFile<int>(InFile);

	FBXLoad::ReadFromFile(InFile, pBuffer, nKeys * 2);

	for (int i = nKeys; i < nKeys * 2; ++i) vecKeyValues[i - nKeys].z = pBuffer[i];

}

