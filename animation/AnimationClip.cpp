#include "engine/animation/AnimationClip.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimationClip::AnimationClip()
{
    
}

//---------------------------------------------------------------------------------------

AnimationClip::~AnimationClip()
{
    
}

//---------------------------------------------------------------------------------------

void AnimationClip::FixPointers()
{
    m_joint_poses = (JointPose*)( (u8*)this + (int)m_joint_poses + sizeof(AnimationClip) );
    m_joint_remap = (s16*)( (u8*)this + (int)m_joint_remap + sizeof(AnimationClip) );
}

//---------------------------------------------------------------------------------------

void AnimationClip::BreakPointers()
{
    m_joint_poses = (JointPose*)( (u8*)m_joint_poses - (int)this - sizeof(AnimationClip) );
    m_joint_remap = (s16*)( (u8*)m_joint_remap - (int)this - sizeof(AnimationClip) );
}

//---------------------------------------------------------------------------------------

bool AnimationClip::operator-=( const AnimationClip& source )
{
    if( m_skeleton_joint_count != source.m_skeleton_joint_count
       || m_frames_per_ms != source.m_frames_per_ms
       || m_frame_count != source.m_frame_count )
        return false;
  
    for( u32 f = 0; f < m_frame_count; ++f )
    {
        for( u32 i = 0; i < m_skeleton_joint_count; ++i )
        {
            if( HasJointPose(i) )
            {
                JointPose pose = GetJointPose(f, i);
                
                if( source.HasJointPose(i) )
                {
                    const JointPose& source_pose = source.GetJointPose(f, i);
                    pose -= source_pose;
                }
                
                // update joint position.
                u32 array_index = f * m_animated_joint_count + i;
                m_joint_poses[array_index] = pose;
            }
        }
    }
    
    return true;
}

//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

