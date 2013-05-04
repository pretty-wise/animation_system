#include "engine/animation/Animation.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
    
Animation::Animation()
: m_clip(nullptr)
, m_global_start_time_ms(0.f)
, m_looped(false)
, m_playback_rate(1.f)
{
    
}
    
//---------------------------------------------------------------------------------------
    
Animation::Animation( const Animation& rhs, float current_clock_ms )
: m_clip(rhs.m_clip)
, m_global_start_time_ms(current_clock_ms)
, m_looped(rhs.m_looped)
, m_playback_rate(rhs.m_playback_rate)
{
    
}
    
//---------------------------------------------------------------------------------------
    
Animation::Animation( AnimationClip* clip, float current_clock_ms, bool looped, float playback_rate )
: m_clip(clip)
, m_global_start_time_ms(current_clock_ms)
, m_looped(looped)
, m_playback_rate(playback_rate)
{
    
}

//---------------------------------------------------------------------------------------

Animation& Animation::operator=(const Animation& rhs )
{
    m_clip = rhs.m_clip;
    m_global_start_time_ms = rhs.m_global_start_time_ms;
    m_looped = rhs.m_looped;
    m_playback_rate = rhs.m_playback_rate;
    
    return *this;
}

//---------------------------------------------------------------------------------------

void Animation::GetJointPose(float current_global_time_ms, s16 joint_idx, AnimationClip::JointPose& out_pose) const
{
    float local_time = GetLocalAnimationTime( current_global_time_ms );
    ENGINE_ASSERT( local_time <= m_clip->GetDuration( m_looped ), "local time out of range" );
    
    m_clip->GetJointPose( local_time, joint_idx, out_pose, m_looped );
}
    
//---------------------------------------------------------------------------------------
    
float Animation::GetLocalAnimationTime(float global_time_ms) const
{
    float local_time = 0.f;
    
    // if animation is played once (N = 1):
    //      t = clamp(t = R(T - Tstart), 0, D).
    // if animation loops forever (N = infinity)
    //      t = (R(T - Tstart)) mod D.
    // if animation is looed finite number of times ( 1 < N < inf )
    //      t = clamp(R(T - Tstart), 0, N*D) mod D.
    
    float duration = m_clip->GetDuration( m_looped );
    
    if(m_looped)
    {
        // (m_playback_rate * (globalTime - m_startGlobalTime)) % duration;
        float timer = (m_playback_rate * (global_time_ms - m_global_start_time_ms));
        local_time = fmodf(timer, duration);
    }
    else
    {
        local_time = Engine::Math::Clamp<float>(m_playback_rate * (global_time_ms - m_global_start_time_ms), -duration, duration);
    }
    
    // fix negative playback rate.
    if(local_time < 0.f)
        local_time += duration;
    
    return local_time;
}

//---------------------------------------------------------------------------------------

bool Animation::IsValid() const
{
    return m_clip != nullptr;
}

//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

