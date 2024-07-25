#pragma once

#define ASPECT_RATIO				(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

enum class CameraMode{ GHOST, FIRST_PERSON, THIRD_PERSON };

struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;
	XMFLOAT3						m_xmf3Position;
};

class CPlayer;

class CCamera
{
protected:
	XMFLOAT3						m_xmf3Position;
	XMFLOAT3						m_xmf3Right;
	XMFLOAT3						m_xmf3Up;
	XMFLOAT3						m_xmf3Look;

	float           				m_fPitch;
	float           				m_fRoll;
	float           				m_fYaw;

	CameraMode						m_CameraMode;

	XMFLOAT3						m_xmf3LookAtWorld;

	XMFLOAT4X4						m_xmf4x4View;
	XMFLOAT4X4						m_xmf4x4Projection;

	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;

	CPlayer							*m_pPlayer = NULL;

	ID3D12Resource					*m_pd3dcbCamera = NULL;
	VS_CB_CAMERA_INFO				*m_pcbMappedCamera = NULL;

public:
	CCamera();
	CCamera(CCamera *pCamera);
	virtual ~CCamera();

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList);

	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	void RegenerateViewMatrix();

	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList *pd3dCommandList);

	void SetPlayer(CPlayer *pPlayer) { m_pPlayer = pPlayer; }
	CPlayer *GetPlayer() { return(m_pPlayer); }

	void SetMode(CameraMode Mode) { m_CameraMode = Mode; }
	CameraMode GetMode() { return(m_CameraMode); }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return(m_xmf3Position); }

	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) { m_xmf3LookAtWorld = xmf3LookAtWorld; }
	XMFLOAT3& GetLookAtPosition() { return(m_xmf3LookAtWorld); }

	XMFLOAT3& GetRightVector() { return(m_xmf3Right); }
	XMFLOAT3& GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3& GetLookVector() { return(m_xmf3Look); }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }

	XMFLOAT4X4 GetViewMatrix() { return(m_xmf4x4View); }
	XMFLOAT4X4 GetProjectionMatrix() { return(m_xmf4x4Projection); }
	D3D12_VIEWPORT GetViewport() { return(m_d3dViewport); }
	D3D12_RECT GetScissorRect() { return(m_d3dScissorRect); }

	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }
	virtual void Rotate(float fPitch, float fYaw, float fRoll) { }
	virtual void Update(float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& xmf3LookAt) { }
};

// 카메라 가속 수치 cm/s
#define CAMERA_ACCELERATION 100.f

class CGhostCamera : public CCamera
{
private:
	float m_fMovingSpeed = 100.f;

public:
	CGhostCamera(CCamera *pCamera);
	virtual ~CGhostCamera() { }

	float GetMovingSpeed() const { return m_fMovingSpeed; }
	
	void SetMovingSpeed(const float& fValue) { m_fMovingSpeed = fValue; }

	void Move(const DWORD& dwDirection, const float& fElapsedTime);
	virtual void Rotate(float fPitch, float fYaw, float fRoll);
};


class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera *pCamera);
	virtual ~CFirstPersonCamera() { }

	virtual void Rotate(float fPitch, float fYaw, float fRoll);
};

class CThirdPersonCamera : public CCamera
{
private:
	const float kTpsDefaultDistanceOffset = 500.f;
	const float kTpsDefaultPitchOffset = 40.f;
	const float kHeightOffset = 85.f;

	float m_fOffsetDistance;
	float m_fOffsetPitch;
	XMFLOAT3 m_xmf3Offset; 

	float m_fMovingLagSpeed = 0.f; // cm/s
	float m_fRotatingLagSpeed = 0; // degree/s

	float m_fZoomScale = 20.f;

public:
	CThirdPersonCamera();
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { }

	float GetOffsetDistance() const { return m_fOffsetDistance; }
	float GetOffsetPitch() const { return m_fOffsetPitch; }
	float GetZoomScale() const { return m_fZoomScale; }

	void SetOffsetDistance(const float& fDistance) { m_fOffsetDistance = fDistance; }
	void SetOffsetPitch(const float& fPitch) { m_fPitch = fPitch; }
	void SetOffset(const float& fDistance, const float& fAngle); // 카메라 오프셋 벡터를 각 거리와 각도(Degree)를 이용하여 설정한다.
	void SetOffset(const XMFLOAT3& xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	void SetMovingLagSpeed(const float& fSpeed) { m_fMovingLagSpeed = fSpeed; }
	void SetRotatingLagSpeed(const float& fSpeed) { m_fRotatingLagSpeed = fSpeed; }

	void AddOffsetDistance(const float& fValue) { m_fOffsetDistance += fValue; }

	virtual void Update(float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
	virtual void Rotate(float fPitch, float fYaw, float fRoll);

	void ResetFromPlayer();
};

