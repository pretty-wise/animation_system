#include "engine/animation/Skeleton.h"
#include "engine/renderer/DebugRenderer.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
    
Skeleton::Skeleton()
: m_name(0)
, m_joint_count(0)
, m_joints(nullptr)
{
    
}
    
//---------------------------------------------------------------------------------------
    
void Skeleton::FixPointers()
{
    m_joints = (SkeletonJoint*)( (u8*)this + (int)m_joints + sizeof(Skeleton) );
}

//---------------------------------------------------------------------------------------

void Skeleton::BreakPointers()
{
    m_joints = (SkeletonJoint*)( (u8*)m_joints - (int)this - sizeof(Skeleton) );
}
    
//---------------------------------------------------------------------------------------

void Skeleton::Draw( DebugRenderer& rend )
{
    ColorRGBA color = ColorRGBA::Green;
    
    for( u32 i = 0; i < m_joint_count; ++i )
    {
        SkeletonJoint& joint = m_joints[i];
        
        Matrix4x4 bind_pose = joint.GetInvBindPose();
        bind_pose.InverseIt();
        
        Vec4 pos = bind_pose * Vec4(0.f, 0.f, 0.f, 1.f);
        rend.AddAxis(bind_pose, 0.5f);
        
        // draw parent-child connection (if applicable).
        if( joint.GetParentIndex() != (u32)-1 && joint.GetParentIndex() < m_joint_count )
        {
            SkeletonJoint& parent = m_joints[joint.GetParentIndex()];
            
            Matrix4x4 parent_bind_pose = parent.GetInvBindPose();
            parent_bind_pose.InverseIt();
            
            Vec4 parent_pos = parent_bind_pose * Vec4(0.f, 0.f, 0.f, 1.f);
            
            rend.AddLine(Vec3(pos.x, pos.y, pos.z), Vec3(parent_pos.x, parent_pos.y, parent_pos.z), color);
        }
    }
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

