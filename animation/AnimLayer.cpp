#include "engine/animation/AnimLayer.h"
#include "engine/animation/AnimStates.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimLayer::AnimLayer()
: m_state_data(nullptr)
, m_paused(false)
, m_global_clock(0.f)
, m_type(LayerType::Lerp)
, m_blend_factor(1.f)
, m_current_state(0)
{
    
}

//---------------------------------------------------------------------------------------

AnimLayer::~AnimLayer()
{
}
    
//---------------------------------------------------------------------------------------

void AnimLayer::SetStateData( AnimStates* data )
{
    m_state_data = data;
    
    u16 max_node_count = 0;
    if( data )
        max_node_count = data->GetMaxNodeCount();

    m_current_tree.Resize(max_node_count);
    m_previous_tree.Resize(max_node_count);
}
    
//---------------------------------------------------------------------------------------

bool AnimLayer::Play( StringId state_name, float blend_ms )
{
    if( !m_state_data )
        return false;
    
    const AnimStates::State* state = m_state_data->FindState( state_name );
    
    if( !state )
        return false;
    
    const AnimBlendTree& tree = state->GetBlendTree();
    
    if( !tree.IsValid() )
        return false;
    
    bool res = Play( tree, blend_ms, m_global_clock );
    
    if( res )
        m_current_state = state_name;
    
    return res;
}

//---------------------------------------------------------------------------------------

bool AnimLayer::Transition( StringId transition_name )
{
    if( !m_state_data )
        return false;
    
    const AnimStates::State* current = m_state_data->FindState( m_current_state );
    
    if( !current )
        return false;
    
    const AnimStates::Transition* transition = current->FindTransition( transition_name );
    
    if( !transition )
        return false;
    
    const AnimStates::State* destination = transition->GetDestinationState();
    ENGINE_ASSERT(destination, "invalid destination");
    
    float start_time = m_global_clock;
    
    if( transition->IsSynced() )
    {
        start_time = m_global_clock - m_current_tree.GetLocalAnimationTime( m_global_clock );
    }
    
    const AnimBlendTree& tree = destination->GetBlendTree();
    
    if( !tree.IsValid() )
        return false;
    
    bool res = Play( tree, transition->GetBlendTime(), start_time );
    
    if( res )
        m_current_state = destination->GetName();
    
    return res;
}

//---------------------------------------------------------------------------------------
    
bool AnimLayer::Play( const AnimBlendTree& tree, float blend_ms, float start_time_ms )
{
    ENGINE_ASSERT(tree.IsValid(), "invalid tree specified");
    
    if( blend_ms > 0.f && m_current_tree.IsValid() )
    {
        if( m_previous_tree.IsValid() )
            m_previous_tree.Clear();
        
        // todo: more blend types.
        m_crossfade_type = BlendType::Linear;
        m_crossfade_duration = blend_ms;
        m_crossfade_timer = 0.f;
        
        m_previous_tree = m_current_tree;
        
        m_current_tree = tree;
    }
    else
    {
        m_current_tree.Clear();
        m_current_tree = tree;
    }
    
    m_current_tree.Start( start_time_ms );
    
    return true;
}
    
//---------------------------------------------------------------------------------------
    
void AnimLayer::Stop()
{
    m_previous_tree.Clear();
    
    m_crossfade_duration = 0.f;
    m_crossfade_timer = 0.f;
    m_crossfade_type = BlendType::None;
    
    m_current_tree.Clear();

    m_paused = false;
}
    
//---------------------------------------------------------------------------------------
    
bool AnimLayer::GetJointPose( u32 joint_idx, AnimationClip::JointPose& pose )
{
    bool res = m_current_tree.GetJointPose( m_global_clock, joint_idx, pose );
    
    if( res && m_previous_tree.IsValid() )
    {
        // blend in factor (blend out should be 1.f - factor).
        float factor = m_crossfade_timer / m_crossfade_duration;
        
        // get previous animation tree pose.
        AnimationClip::JointPose previous_pose;
        m_previous_tree.GetJointPose( m_global_clock, joint_idx, previous_pose);
        
        // lerp with new pose.
        pose.MakeLerp(previous_pose, pose, factor);
    }
    
    return res;
}
    
//---------------------------------------------------------------------------------------

void AnimLayer::Update( float fDeltaMs )
{
    if( !Paused() )
    {
        m_global_clock += fDeltaMs;
        
        // update crossfade.
        if( m_crossfade_type != BlendType::None )
        {
            m_crossfade_timer += fDeltaMs;
            
            // check if crossfade ended.
            if( m_crossfade_timer >= m_crossfade_duration )
            {
                // remove previous tree.
                // glitch happens here if already blending with previous tree.
                m_previous_tree.Clear();
                
                // reset crossfade data.
                m_crossfade_type = BlendType::None;
                m_crossfade_duration = 0.f;
                m_crossfade_timer = 0.f;
            }
        }
        
        // prevent float overflow.
        const float max_clock_value = std::numeric_limits<float>::max() * 0.5f;        
        if( m_global_clock > max_clock_value )
        {
            m_global_clock -= max_clock_value;
            
            if( m_current_tree.IsValid() )
                m_current_tree.FixAnimationStartTime( max_clock_value );
            
            if( m_previous_tree.IsValid() )
                m_previous_tree.FixAnimationStartTime( max_clock_value );
        }
    }
}

//---------------------------------------------------------------------------------------

bool AnimLayer::SetNodeFactor( StringId name, float value )
{
    return m_current_tree.SetNodeFactor( name, value );
}

//---------------------------------------------------------------------------------------

bool AnimLayer::GetNodeFactor( StringId name, float& value ) const
{
    return m_current_tree.GetNodeFactor( name, value );
}

//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

