#include "engine/animation/AnimLayer.h"
#include "engine/animation/AnimStates.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimLayer::AnimLayer()
: m_state_data(nullptr)
, m_current_tree(nullptr)
, m_previous_tree(nullptr)
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
    delete m_current_tree;
    delete m_previous_tree;
}

//---------------------------------------------------------------------------------------

bool AnimLayer::Play( StringId state_name, float blend_ms )
{
    if( !m_state_data )
        return false;
    
    const AnimStates::State* state = m_state_data->FindState( state_name );
    
    if( !state )
        return false;
    
    AnimBlendNode* tree = state->CopyBlendTree( m_global_clock );
    
    if( !tree )
        return false;
    
    bool res = Play( tree, blend_ms );
    
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
        start_time = m_global_clock - m_current_tree->GetLocalAnimationTime( m_global_clock );
    }
    
    AnimBlendNode* tree = destination->CopyBlendTree( start_time );
    
    if( !tree )
        return false;
    
    bool res = Play( tree, transition->GetBlendTime()  );
    
    if( res )
        m_current_state = destination->GetName();
    
    return res;
}

//---------------------------------------------------------------------------------------
    
bool AnimLayer::Play( AnimBlendNode* tree, float blend_ms )
{
    ENGINE_ASSERT(tree, "null tree specified");
    
    // todo: support blend
    
    if( blend_ms > 0.f && m_current_tree )
    {
        if( m_previous_tree )
            delete m_previous_tree;
        
        // todo: more blend types.
        m_crossfade_type = BlendType::Linear;
        m_crossfade_duration = blend_ms;
        m_crossfade_timer = 0.f;
        
        m_previous_tree = m_current_tree;
        
        m_current_tree = tree;
    }
    else
    {
        delete m_current_tree;
        m_current_tree = tree;
    }
    
    return true;
}
    
//---------------------------------------------------------------------------------------
    
void AnimLayer::Stop()
{
    delete m_previous_tree;
    m_previous_tree = nullptr;
    
    m_crossfade_duration = 0.f;
    m_crossfade_timer = 0.f;
    m_crossfade_type = BlendType::None;
    
    delete m_current_tree;
    m_current_tree = nullptr;

    m_paused = false;
}
    
//---------------------------------------------------------------------------------------
    
bool AnimLayer::GetJointPose( u32 joint_idx, AnimationClip::JointPose& pose )
{
    bool res = m_current_tree->GetJointPose( m_global_clock, joint_idx, pose );
    
    if( res && m_previous_tree )
    {
        // blend in factor (blend out should be 1.f - factor).
        float factor = m_crossfade_timer / m_crossfade_duration;
        
        // get previous animation tree pose.
        AnimationClip::JointPose previous_pose;
        m_previous_tree->GetJointPose( m_global_clock, joint_idx, previous_pose);
        
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
                delete m_previous_tree;
                m_previous_tree = nullptr;
                
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
            
            if( m_current_tree )
                m_current_tree->FixAnimationStartTime( max_clock_value );
            
            if( m_previous_tree )
                m_previous_tree->FixAnimationStartTime( max_clock_value );
        }
    }
}

//---------------------------------------------------------------------------------------

bool AnimLayer::SetNodeFactor( StringId name, float value )
{
    if( m_current_tree )
    {
        AnimBlendNode* node = m_current_tree->FindNode(name);
        
        if( node )
        {
            node->SetFactor(value);
            return true;
        }
        
    }
    
    return false;
}

//---------------------------------------------------------------------------------------

bool AnimLayer::GetNodeFactor( StringId name, float& value ) const
{
    if( m_current_tree )
    {
        AnimBlendNode* node = m_current_tree->FindNode(name);
        
        if( node )
        {
            value = node->GetFactor();
            return true;
        }
    }
    
    return false;
}

//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

