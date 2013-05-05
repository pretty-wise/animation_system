#pragma once

#include "engine/animation/Animation.h"
#include <vector>

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class AnimationClip;

namespace BlendNodeType{
    enum Enum{
        Value,
        Lerp,
        Additive,
        CrossFade
    };
}
    
//---------------------------------------------------------------------------------------

// todo: rewrite blend tree to use less dynamic memory allocations. similar as AnimHierarchy.
class AnimBlendNode
{
public:
    AnimBlendNode( AnimationClip* clip, float globa_clock_ms, bool looped, float playback_rate );
    AnimBlendNode( AnimBlendNode* left, AnimBlendNode* right, StringId factor_name );
    AnimBlendNode( const AnimBlendNode& rhs, float current_time_ms );
    
    inline BlendNodeType::Enum GetType() const { return m_type; }
    
    inline AnimBlendNode* GetLeft() const { return m_left; }
    
    inline AnimBlendNode* GetRight() const { return m_right; }
    
    inline float GetFactor() const { return m_blend_factor; }
    
    inline void SetFactor( float value ) { m_blend_factor = value; }
    
    // returns true if pose has been calculated for this subtree.
    bool GetJointPose( float global_time_ms, s16 joint_idx, AnimationClip::JointPose& pose );
    
    // returns local animation time of first animation in the tree.
    float GetLocalAnimationTime(float global_time) const;
    
    //
    void FixAnimationStartTime( float rewind_time_ms );
    
    AnimBlendNode* FindNode( StringId factor_name );
    
private:
    // left tree node.
    AnimBlendNode* m_left;
    
    // right tree node.
    AnimBlendNode* m_right;
    
    // type of node.
    BlendNodeType::Enum m_type;
    
    // current blend factor. valid if Lerp, Additive or CrossFade node.
    float m_blend_factor;
    
    // total blend time. valid if Lerp, Additive or CrossFade node.
    float m_blend_time_ms;
    
    // name of blend node.
    StringId m_factor_name;
    
    // animation. valid if Value node.
    Animation m_animation;
};

//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------