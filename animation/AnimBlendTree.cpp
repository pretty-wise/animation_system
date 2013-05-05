#include "engine/animation/AnimBlendTree.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
    
AnimBlendNode::AnimBlendNode( AnimationClip* clip, float global_clock_ms, bool looped, float playback_rate )
: m_left(nullptr)
, m_right(nullptr)
, m_type(BlendNodeType::Value)
, m_blend_factor(1.f)
, m_blend_time_ms(0.f)
, m_factor_name(0)
{
    m_animation = Animation(clip, global_clock_ms, looped, playback_rate);
}
    
//---------------------------------------------------------------------------------------
    
AnimBlendNode::AnimBlendNode( AnimBlendNode* left, AnimBlendNode* right, StringId factor_name )
: m_left(left)
, m_right(right)
, m_type(BlendNodeType::Additive)
, m_blend_factor(1.f)
, m_blend_time_ms(0.f)
, m_factor_name(factor_name)
{
    ENGINE_ASSERT(m_left, "left node invalid");
    ENGINE_ASSERT(m_right, "right node invalid");
}

//---------------------------------------------------------------------------------------
    
AnimBlendNode::AnimBlendNode( const AnimBlendNode& rhs, float current_time_ms )
: m_left(nullptr)
, m_right(nullptr)
, m_type(rhs.m_type)
, m_blend_factor(rhs.m_blend_factor)
, m_blend_time_ms(rhs.m_blend_time_ms)
, m_factor_name(rhs.m_factor_name)
{
    if( rhs.m_animation.IsValid() )
        m_animation = Animation(rhs.m_animation, current_time_ms);
    
    if( rhs.m_left )
        m_left = new AnimBlendNode(*rhs.m_left, current_time_ms);
    
    if( rhs.m_right )
        m_right = new AnimBlendNode(*rhs.m_right, current_time_ms);
}
    
//---------------------------------------------------------------------------------------
    
bool AnimBlendNode::GetJointPose( float global_time_ms, s16 joint_idx, AnimationClip::JointPose& pose )
{
    switch( m_type )
    {
        case BlendNodeType::Value:
        {
            ENGINE_ASSERT(m_animation.IsValid(), "invalid animation node");
            
            if( m_animation.HasJointPose(joint_idx) )
            {
                m_animation.GetJointPose( global_time_ms, joint_idx, pose );
                return true;
            }
        }
        break;
        case BlendNodeType::Lerp:
        case BlendNodeType::CrossFade:
        {
            AnimationClip::JointPose pose_a, pose_b;
            
            bool has_left_subtree = m_left->GetJointPose( global_time_ms, joint_idx, pose_a );
            bool has_right_subtree = m_right->GetJointPose( global_time_ms, joint_idx, pose_b );
            
            if( has_right_subtree && has_left_subtree )
                pose.MakeLerp(pose_a, pose_b, m_blend_factor);
            else if( has_right_subtree )
                pose = pose_a;
            else if( has_left_subtree )
                pose = pose_b;
            else
                return false;
            
            return true;
        }
        break;
        case BlendNodeType::Additive:
        {
            AnimationClip::JointPose pose_b;
            
            bool has_left_subtree = m_left->GetJointPose( global_time_ms, joint_idx, pose );
            bool has_right_subtree = m_right->GetJointPose( global_time_ms, joint_idx, pose_b );
            
            ENGINE_ASSERT(has_left_subtree, "left subtree has to be valid for additive animation");
            
            if( has_right_subtree )
                pose.AdditiveAdd(pose_b, m_blend_factor);
            
            return true;
        }
        break;
    }
    
    return false;
}

//---------------------------------------------------------------------------------------

float AnimBlendNode::GetLocalAnimationTime(float current_time) const
{
    if( m_animation.IsValid() )
        return m_animation.GetLocalAnimationTime(current_time);
    
    float local_anim_time = 0.f;
    
    if( m_left )
        local_anim_time = m_left->GetLocalAnimationTime(current_time);
    
    if( local_anim_time > 0.f )
        return local_anim_time;
    
    if( m_right )
        m_right->GetLocalAnimationTime(current_time);
    
    return 0.f;
}
    
//---------------------------------------------------------------------------------------
    
void AnimBlendNode::FixAnimationStartTime(float rewind_time_ms)
{
    if( m_animation.IsValid() )
        m_animation.SetStartTime( m_animation.GetStartTime() - rewind_time_ms );
    
    if( m_left )
        m_left->FixAnimationStartTime(rewind_time_ms);
    
    if( m_right )
        m_right->FixAnimationStartTime(rewind_time_ms);
}
    
//---------------------------------------------------------------------------------------

AnimBlendNode* AnimBlendNode::FindNode( StringId factor_name )
{
    if( m_factor_name == factor_name )
        return this;
    
    AnimBlendNode* result = nullptr;
    
    if( m_left )
        result = m_left->FindNode( factor_name );
    
    if( !result && m_right )
        result = m_right->FindNode( factor_name );
    
    return result;
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

