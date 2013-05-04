#pragma once

#include "engine/animation/Animation.h"
#include <vector>

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class Skeleton;
class DebugRenderer;
    
class AnimTransformation
{
public:
    static u16 InvalidIndex;
    
public:
    AnimTransformation();
    AnimTransformation( StringId name, u16 parent_idx );
    AnimTransformation& operator=( const AnimTransformation& rhs );
    
    inline const Vec3& GetLocalTranslation() const { return m_translation; }
    
    inline float GetLocalScale() const { return m_scale; }
    
    inline const Quaternion& GetLocalRotation() const { return m_rotation; }
    
    inline const Matrix4x4& GetLocalTransformation() const { return m_local_trans; }
    
    inline const Matrix4x4& GetWorldTransformation() const { return m_world_trans; }
    
    inline void SetTranslation( const Vec3& t ) { m_translation = t; }
    
    inline void SetScale( float s ) { m_scale = s; }
    
    inline void SetRotation( const Quaternion& q ) { m_rotation = q; }
    
    inline u16 GetParentIndex() const { return m_parent_idx; }
    
    void CalculateLocalTransformation();
    
    void CalculateGlobalTransformation( const AnimTransformation& parent );
    void CalculateGlobalTransformation(/* no parent */);
    
private:
    //! node's name.
    StringId m_name;
    
    //! node's local translation.
    Vec3 m_translation;
    
    //! node's local uniform scale.
    float m_scale;
    
    //! node's local rotation.
    Quaternion m_rotation;
    
    //! node's local transformation.
    Matrix4x4 m_local_trans;
    
    //! node's world transformation.
    Matrix4x4 m_world_trans;
    
    //! parent index in the array.
    u16 m_parent_idx;
};
    
//---------------------------------------------------------------------------------------
    
class AnimHierarchy
{
public:
    static AnimHierarchy* CreateFromSkeleton( Skeleton* skeleton );
public:
    AnimHierarchy( u16 node_count );
    ~AnimHierarchy();
    
    inline u16 GetNodeCount() const { return m_node_count; }
    
    AnimTransformation& GetNode( u16 idx ) { ENGINE_ASSERT(idx < m_node_count, ""); return m_nodes[idx]; }
    
    void CalculateGlobalTransformation();
    
    void Draw( DebugRenderer& rend );
private:
    AnimTransformation* m_nodes;
    u16 m_node_count;
};
    
//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------