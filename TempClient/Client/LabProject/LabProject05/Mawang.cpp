#include "stdafx.h"
#include "Mawang.h"
#include "Animation.h"

const std::string CMawang::mawang_model_file_name_ = "../Resource/Model/Mawang_Model.bin";
//const std::string CMawang::mawang_model_file_name_ = "../Resource/Model/Player_Model.bin";

CMawang::CMawang()
{
}

CMawang::CMawang(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, const CModelInfo& model) : CRootObject(model)
{
	CreateBoneTransformMatrix(device, command_list);

	axis_transform_matrix_ = new XMFLOAT4X4
	(1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1);

	SetShader(4);
}

CMawang::~CMawang()
{
}
