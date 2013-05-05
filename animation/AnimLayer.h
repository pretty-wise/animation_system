#pragma once

#include "engine/animation/AnimationClip.h"
#include "engine/animation/AnimBlendTree.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
    
class AnimStates;
    
//---------------------------------------------------------------------------------------

// enumerates type of animation layer.
// defines how a layer pose gets blended with previously generated pose.
namespace LayerType{
    enum Enum{
        Additive,
        Lerp
    };
}

// enumerates type of cross fade.
// deines how blend factor changes during animation cross fade.
namespace BlendType{
    enum Enum{
        None = -1,
        Linear,
        EaseIn, //todo: support
        EaseOut //todo: support
    };
}

//---------------------------------------------------------------------------------------
    
class AnimLayer
{
public:
    AnimLayer();
    ~AnimLayer();
    
    // looks for named state in state data and transitions with specified blend time.
    bool Play( StringId state_name, float blend_ms );
    
    // looks for named transition in state data and transitions to specified blend tree.
    bool Transition( StringId transition_name );
    
    inline void Pause() { m_paused = true; }
    
    inline void Resume() { m_paused = false; }
    
    void Stop();
    
    inline bool Active() { return m_current_tree.IsValid(); }
    
    bool Paused() const { return m_paused; }
    
    bool GetJointPose( u32 joint_idx, AnimationClip::JointPose& pose );
    
    void Update( float fDeltaMs );
    
    inline LayerType::Enum GetType() const { return m_type; }
    
    inline void SetType( LayerType::Enum t ) { m_type = t; }
    
    bool SetNodeFactor( StringId name, float value );
    
    bool GetNodeFactor( StringId name, float& value ) const;
    
    inline float GetBlendFactor() const { return m_blend_factor; }
    
    inline void SetBlendFactor( float f ) { m_blend_factor = f; }
    
    void SetStateData( AnimStates* data );
    
private:
    bool Play( const AnimBlendTree& tree, float blend_ms, float start_time_ms );
    
private:
    // layer state and transition data.
    AnimStates* m_state_data;
    
    // current state played.
    StringId m_current_state;
    
    // layer timer.
    float m_global_clock;
    
    // layer animation pause state.
    bool m_paused;
    
    // root node of animation blend tree.
    AnimBlendTree m_current_tree;
    
    // used while cross-blending.
    AnimBlendTree m_previous_tree;
    
private:
    // type of layer, used while blending layers.
    LayerType::Enum m_type;
    
    // layer blend factor (used when blending between layers occur).
    float m_blend_factor;
    
private:
    // duration of tree cross fading.
    float m_crossfade_duration;
    
    // crossfade timer.
    float m_crossfade_timer;
    
    // type of current crossfade.
    BlendType::Enum m_crossfade_type;
};

//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------