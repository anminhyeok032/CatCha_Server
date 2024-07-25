#pragma once
#include "Component.h"

class CCubeMesh;

class CObbComponent :
    public CComponent
{
private:
    BoundingOrientedBox obb_;
    BoundingOrientedBox animated_obb_;
    XMFLOAT4X4 offset_matrix_ = Matrix4x4::Identity();
    XMFLOAT4X4 world_matrix_ = Matrix4x4::Identity();
    CGameObject* parent_socket_ = NULL;
    CCubeMesh* debug_cube_ = NULL;

    bool is_active_ = true;

public:
    CObbComponent() {}
    CObbComponent(const CObbComponent&);
    CObbComponent(CGameObject* owner, const BoundingBox& aabb, CGameObject* parent_socket);
    ~CObbComponent();

    //setter
    void set_offset_matrix(const XMFLOAT4X4& value) { offset_matrix_ = value; }

    //getter
    bool is_active() const { return is_active_; }
    BoundingOrientedBox obb() const { return obb_; }
    BoundingOrientedBox animated_obb() const { return animated_obb_; }

    CObbComponent& operator=(const CObbComponent& rhs)
    {
        obb_ = rhs.obb_;
        animated_obb_ = rhs.animated_obb_;
        world_matrix_ = rhs.world_matrix_;
        parent_socket_ = rhs.parent_socket_;
        debug_cube_ = NULL;

        return *this;
    }

    void On() { is_active_ = true; }
    void Off() { is_active_ = false; }

    void CreateDebugCubeMesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list);

    void Update();
    void Render(ID3D12GraphicsCommandList* command_list);

    bool Intersects(const BoundingOrientedBox& other);

};

