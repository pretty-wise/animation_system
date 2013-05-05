#include "engine/animation/AnimBlendTree.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimBlendTree::Node::Node()
: m_left_index(-1)
, m_right_index(-1)
, m_type(BlendNodeType::Undefined)
, m_blend_factor(0.f)
, m_factor_name(0)
{
    
}

//---------------------------------------------------------------------------------------

AnimBlendTree::Node::Node( AnimationClip* clip, float globa_clock_ms, bool looped, float playback_rate )
: m_left_index(-1)
, m_right_index(-1)
, m_type(BlendNodeType::Value)
, m_blend_factor(1.f)
, m_factor_name(0)
{
    m_animation = Animation(clip, globa_clock_ms, looped, playback_rate);
}

//---------------------------------------------------------------------------------------

AnimBlendTree::Node::Node( BlendNodeType::Enum type, u16 left_index, u16 right_index, StringId factor_name, float factor_value )
: m_left_index(left_index)
, m_right_index(right_index)
, m_type(type)
, m_blend_factor(factor_value)
, m_factor_name(factor_name)
{
    
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

AnimBlendTree::AnimBlendTree()
: m_nodes(nullptr)
, m_node_count(0)
, m_capacity(0)
{
    
}

//---------------------------------------------------------------------------------------

AnimBlendTree::AnimBlendTree( u16 capacity )
: m_nodes(nullptr)
, m_node_count(0)
, m_capacity(capacity)
{
    m_nodes = new Node[m_capacity];
}

//---------------------------------------------------------------------------------------

AnimBlendTree::~AnimBlendTree()
{
    delete [] m_nodes;
}

//---------------------------------------------------------------------------------------

AnimBlendTree& AnimBlendTree::operator=( const AnimBlendTree& rhs )
{
    ENGINE_ASSERT(m_capacity >= rhs.GetCapacity(), "too many nodes to merge");
    
    m_node_count = rhs.m_node_count;
    
    for( u16 i = 0; i < rhs.m_node_count; ++i )
        m_nodes[i] = rhs.m_nodes[i];
    
    return *this;
}

//---------------------------------------------------------------------------------------

void AnimBlendTree::Clear()
{
    m_node_count = 0;
}

//---------------------------------------------------------------------------------------

void AnimBlendTree::Resize( u16 capacity )
{
    u16 old_capacity = m_capacity;
    Node* old_nodes = m_nodes;
    m_nodes = nullptr;
    
    // create new nodes.
    if( capacity > 0 )
        m_nodes = new Node[capacity];
    m_capacity = capacity;
    
    // save old nodes data.
    u16 i = 0;
    while( i < capacity && i < old_capacity )
    {
        m_nodes[i] = old_nodes[i];
        i++;
    }
    
    // remove old nodes.
    delete [] old_nodes;
}

//---------------------------------------------------------------------------------------

void AnimBlendTree::Start( float current_time_ms )
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        Animation& anim = m_nodes[i].GetAnimation();
        
        if( anim.IsValid() )
            anim.SetStartTime(current_time_ms);
    }
}

//---------------------------------------------------------------------------------------

bool AnimBlendTree::SetNodeFactor( StringId factor_name, float value )
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        if( m_nodes[i].GetFactorName() == factor_name )
        {
            m_nodes[i].SetFactor(value);
            return true;
        }
    }
    
    return false;
}

//---------------------------------------------------------------------------------------

bool AnimBlendTree::GetNodeFactor( StringId factor_name, float& value ) const
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        if( m_nodes[i].GetFactorName() == factor_name )
        {
            value = m_nodes[i].GetFactor();
            return true;
        }
    }
    
    return false;
}

//---------------------------------------------------------------------------------------

float AnimBlendTree::GetLocalAnimationTime(float global_time) const
{
    for( u32 i = 0; i < m_node_count; ++i )
    {
        Node& node = m_nodes[i];
        
        const Animation& anim = node.GetAnimation();
        
        if( anim.IsValid() )
            return anim.GetLocalAnimationTime(global_time);
    }
    
    return 0.f;
}

//---------------------------------------------------------------------------------------

bool AnimBlendTree::GetJointPose( u16 current_index, float clock_time_ms, s16 joint_idx, AnimationClip::JointPose& pose ) const
{
    const Node& node = m_nodes[current_index];
    
    switch( node.GetType() )
    {
        case BlendNodeType::Value:
        {
            ENGINE_ASSERT(node.GetAnimation().IsValid(), "invalid animation node");
            
            if( node.GetAnimation().HasJointPose(joint_idx) )
            {
                node.GetAnimation().GetJointPose( clock_time_ms, joint_idx, pose );
                return true;
            }
        }
        break;
        case BlendNodeType::Lerp:
        {
            AnimationClip::JointPose pose_a, pose_b;
            
            bool has_left_subtree = GetJointPose( node.GetLeftIndex(), clock_time_ms, joint_idx, pose_a );
            bool has_right_subtree = GetJointPose( node.GetRightIndex(), clock_time_ms, joint_idx, pose_b );
            
            if( has_right_subtree && has_left_subtree )
                pose.MakeLerp(pose_a, pose_b, node.GetFactor());
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
            
            bool has_left_subtree = GetJointPose( node.GetLeftIndex(), clock_time_ms, joint_idx, pose );
            bool has_right_subtree = GetJointPose( node.GetRightIndex(), clock_time_ms, joint_idx, pose_b );
            
            ENGINE_ASSERT(has_left_subtree, "left subtree has to be valid for additive animation");
            
            if( has_right_subtree )
                pose.AdditiveAdd(pose_b, node.GetFactor());
            
            return true;
        }
        break;
        default:
        ENGINE_ASSERT(0, "undefined node type");
    }
    
    return false;
}

//---------------------------------------------------------------------------------------

bool AnimBlendTree::GetJointPose( float global_time_ms, s16 joint_idx, AnimationClip::JointPose& pose ) const
{
    ENGINE_ASSERT(IsValid(), "animation tree not valid");
    
    return GetJointPose(0, global_time_ms, joint_idx, pose);
}

//---------------------------------------------------------------------------------------

void AnimBlendTree::FixAnimationStartTime( float rewind_time_ms )
{
    for( u16 i = 0; i < m_node_count; ++i )
    {
        Animation& anim = m_nodes[i].GetAnimation();
        
        if( anim.IsValid() )
            anim.SetStartTime( anim.GetStartTime() - rewind_time_ms );
    }
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

