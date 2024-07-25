#include "stdafx.h"
#include "RotationComponent.h"
#include "Object.h"

void CRotationComponent::Rotate(const float& pitch, const float& yaw, const float& roll)
{
	pitch_ += pitch;
	yaw_ += yaw;
	roll_ += roll;
}

void CRotationComponent::Update(const float& elapsed_time)
{
	XMMATRIX P = XMMatrixIdentity(), Y = XMMatrixIdentity(), R = XMMatrixIdentity();
	XMFLOAT3 x_axis = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3 y_axis = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 z_axis = XMFLOAT3(0.f, 0.f, 1.f);

	if (use_pitch_)
	{
		P = XMMatrixRotationAxis(XMLoadFloat3(&x_axis), XMConvertToRadians(pitch_));
	}
	if (use_yaw_)
	{
		Y = XMMatrixRotationAxis(XMLoadFloat3(&y_axis), XMConvertToRadians(yaw_));
	}
	if (use_roll_)
	{
		R = XMMatrixRotationAxis(XMLoadFloat3(&z_axis), XMConvertToRadians(roll_));
	}

	XMMATRIX rotation_matrix = XMMatrixMultiply(XMMatrixMultiply(P, Y), R);
	owner_->set_look_vector(Vector3::TransformNormal(z_axis, rotation_matrix));
	owner_->set_right_vector(Vector3::CrossProduct(y_axis, owner_->look_vector(), true));
	owner_->set_up_vector(Vector3::CrossProduct(owner_->look_vector(), owner_->right_vector(), true));
	

}

void CRotationComponent::UpdateAnothers(int player_num)
{
	XMMATRIX P = XMMatrixIdentity(), Y = XMMatrixIdentity(), R = XMMatrixIdentity();
	XMFLOAT3 x_axis = XMFLOAT3(1.f, 0.f, 0.f);
	XMFLOAT3 y_axis = XMFLOAT3(0.f, 1.f, 0.f);
	XMFLOAT3 z_axis = XMFLOAT3(0.f, 0.f, 1.f);

	if (use_pitch_)
	{
		P = XMMatrixRotationAxis(XMLoadFloat3(&x_axis), XMConvertToRadians(pitch_));
	}
	if (use_yaw_)
	{
		// TODO : 회전 관련 여기 Yaw 확인할것
		Y = XMMatrixRotationAxis(XMLoadFloat3(&y_axis), XMConvertToRadians(yaw_));
	}
	if (use_roll_)
	{
		R = XMMatrixRotationAxis(XMLoadFloat3(&z_axis), XMConvertToRadians(roll_));
	}

	XMMATRIX rotation_matrix = XMMatrixMultiply(XMMatrixMultiply(P, Y), R);
	owner_->set_look_vector(Vector3::TransformNormal(z_axis, rotation_matrix));
	owner_->set_right_vector(Vector3::CrossProduct(y_axis, owner_->look_vector(), true));
	owner_->set_up_vector(Vector3::CrossProduct(owner_->look_vector(), owner_->right_vector(), true));


}


