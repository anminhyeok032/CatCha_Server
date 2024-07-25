#pragma once
#include "Object.h"

class CMawang : public CRootObject
{

public:
	static const std::string mawang_model_file_name_;

	CMawang();
	CMawang(ID3D12Device* device, ID3D12GraphicsCommandList* command_list, const CModelInfo& model);
	~CMawang();


};

