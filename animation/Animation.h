#pragma once

#include "engine/core/StringId.h"
#include "engine/animation/AnimationClip.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class Animation
{
public:
    Animation();
    Animation( const Animation& rhs, float current_clock_ms );
    Animation( AnimationClip* clip, float current_clock_ms, bool looped, float playback_rate );
    Animation& operator=(const Animation& rhs );
    
    void GetJointPose(float global_time_ms, s16 joint_idx, AnimationClip::JointPose& outPose) const;
    
    inline bool	HasJointPose(s16 joint_idx) const { return m_clip->HasJointPose(joint_idx); }
    
    inline float GetPlaybackRate() const { return m_playback_rate; }
    
    inline void	SetPlaybackRate(float rate) { m_playback_rate = rate; }
    
    // calculates the local time of the animation, up to clip duration (in milliseconds).
    float GetLocalAnimationTime(float global_time_ms) const;
    
    // animation start time, relative to AnimLayer timer.
    inline float GetStartTime() const { return m_global_start_time_ms; }
  
    inline void SetStartTime( float time_ms ) { m_global_start_time_ms = time_ms; }
    
    bool IsValid() const;
private:
    // AnimLayer related animation start time.
    float m_global_start_time_ms;
    
    // animation clip is looped if true.
    bool m_looped;
    
    // playback rate scale factor. 1.0 by default.
    float m_playback_rate;
    
    // animation frame data.
    AnimationClip* m_clip;
};

//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------