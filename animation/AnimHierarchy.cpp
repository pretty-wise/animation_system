#include "engine/animation/AnimHierarchy.h"
#include "engine/animation/Skeleton.h"
#include "engine/renderer/DebugRenderer.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

u16 AnimTransformation::InvalidIndex = (u16)-1;
    
AnimTransformation::AnimTransformation()
    : m_name(0)
    , m_translation(Vec3::Zero)
    , m_scale(1.f)
    , m_rotation(Quaternion(0.f, 0.f, 0.f, 1.f))
    , m_parent_idx(AnimTransformation::InvalidIndex)
{
    
}

//---------------------------------------------------------------------------------------

AnimTransformation::AnimTransformation( StringId name, u16 parent_idx )
: m_name(name)
, m_parent_idx(parent_idx)
{
    
}
    
//---------------------------------------------------------------------------------------
    
AnimTransformation& AnimTransformation::operator=( const AnimTransformation& rhs )
{
    m_name = rhs.m_name;
    m_translation = rhs.m_translation;
    m_scale = rhs.m_scale;
    m_rotation = rhs.m_rotation;
    m_local_trans = rhs.m_local_trans;
    m_world_trans = rhs.m_world_trans;
    m_parent_idx = rhs.m_parent_idx;
    
    return *this;
}
    
//---------------------------------------------------------------------------------------

void AnimTransformation::CalculateLocalTransformation()
{
    m_local_trans.MakeScale(m_scale, m_scale, m_scale);
    
    Matrix4x4 rot;
    m_rotation.toMatrix(rot);
    
    m_local_trans *= rot;
    
    m_local_trans.matrix[03] = m_translation.x;
    m_local_trans.matrix[07] = m_translation.y;
    m_local_trans.matrix[11] = m_translation.z;
}
    
//---------------------------------------------------------------------------------------
    
void AnimTransformation::CalculateGlobalTransformation( const AnimTransformation& parent )
{
    m_world_trans = parent.m_world_trans;
    m_world_trans *= m_local_trans;
}
    
//---------------------------------------------------------------------------------------
    
void AnimTransformation::CalculateGlobalTransformation()
{
    m_world_trans = m_local_trans;
}
    
//---------------------------------------------------------------------------------------

AnimHierarchy* AnimHierarchy::CreateFromSkeleton( Skeleton* skeleton )
{
    AnimHierarchy* tree = new AnimHierarchy( skeleton->GetJointCount() );
    
    for( u16 i = 0; i < skeleton->GetJointCount(); ++i )
    {
        const SkeletonJoint& node = skeleton->GetJoint(i);
        
        tree->m_nodes[i] = AnimTransformation( node.GetName(), node.GetParentIndex() );
    }
    
    return tree;
}

//---------------------------------------------------------------------------------------

AnimHierarchy::AnimHierarchy( u16 node_count )
    : m_node_count(node_count)
{
    m_nodes = new AnimTransformation[node_count];
}

//---------------------------------------------------------------------------------------

AnimHierarchy::~AnimHierarchy()
{
    delete [] m_nodes;
}

//---------------------------------------------------------------------------------------

void AnimHierarchy::CalculateGlobalTransformation()
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        AnimTransformation& node = m_nodes[i];
        
        if( node.GetParentIndex() != AnimTransformation::InvalidIndex )
        {
            AnimTransformation& parent = m_nodes[node.GetParentIndex()];
            
            node.CalculateGlobalTransformation( parent );
        }
        else
        {
            node.CalculateGlobalTransformation();
        }
    }
}
    
//---------------------------------------------------------------------------------------
    
void AnimHierarchy::Draw( DebugRenderer& rend )
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        AnimTransformation& node = m_nodes[i];
        
        if( node.GetParentIndex() != AnimTransformation::InvalidIndex )
        {
            AnimTransformation& parent = m_nodes[node.GetParentIndex()];
            
            Vec3 from(node.GetWorldTransformation().matrix[03], node.GetWorldTransformation().matrix[07], node.GetWorldTransformation().matrix[11]);
            Vec3 to(parent.GetWorldTransformation().matrix[03], parent.GetWorldTransformation().matrix[07], parent.GetWorldTransformation().matrix[11]);
            
            rend.AddLine(from, to, ColorRGBA::Red);
        }
        
        rend.AddAxis(node.GetWorldTransformation(), 0.5f);
    }
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

