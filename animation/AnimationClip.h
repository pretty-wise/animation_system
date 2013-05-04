#pragma once

#include "engine/core/StringId.h"
#include "engine/math/Quaternion.h"
#include "engine/math/Vec3.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class AnimationClip
{
public:
	AnimationClip();
	~AnimationClip();
    
    // used to generate additive clip (refference - source clip).
    bool operator-=( const AnimationClip& source );
    
	void BreakPointers();
	void FixPointers();
	
	//---------------------------------------------------------------------------------------

	// joint poses stored translation vector/quaternion rotation/uniform scale format.
	// uniform scale is used to reduce amount memory needed to store AnimationClips.
	// each JointPose stores local joint pose what enables us to blend different AnimationClips.
	struct JointPose
	{
		JointPose();
		inline JointPose( const JointPose& p );
        inline JointPose& operator-=( const JointPose& rhs );
        
		inline void toMatrix(Matrix4x4& out) const;
        
        inline void MakeLerp( const JointPose& poseA, const JointPose& poseB, float factor );
        inline void AdditiveAdd( const JointPose& poseB, float factor );
        
		Quaternion rotation;
		Vec3 translation;
		float scale; // uniform scale.
	};
	
	//---------------------------------------------------------------------------------------

	// animations name.
	inline StringId GetName() const;

	// returns duration of the animation in milliseconds. (m_frame_count [- 1] / m_frames_per_ms).
    // looped animations last frame is skipped (same as first frame).
	inline float GetDuration( bool looped ) const;

	// returns joint pose in between frames for specified joint index. JointPose is inTQS format for further blending.
	inline void	GetJointPose(float localTimeMs, s16 jointIdx, JointPose& outPose, bool looped) const;
	
    // returns true if joint is animated in this clip.
	inline bool HasJointPose(s16 joint_idx) const;

private:
	//! returns sample time i.e. 3.3 - frame between frame 3 and frame 4. used to get blended in between joint pose.
	inline float GetSampleTime(float local_time_ms) const;
    
    // returns joint pose at given frame for given joint index.
    inline const JointPose& GetJointPose( u32 frame, u32 joint_idx ) const;

public:
	//! name of the animation.
	StringId m_name;

	//! total number of skeleton joints. (joint_remap array size).
	s16 m_skeleton_joint_count;
    
    //! joints animated in this animation clip. (joint_poses array size).
    s16 m_animated_joint_count;

	//! frames per second * 0.001f. milliseconds used to avoid float precision errors.
	float m_frames_per_ms;

	//! number of baked frames.
	u32 m_frame_count;

	//! baked animation poses. array of m_frame_count*m_joint_count poses.
	JointPose* m_joint_poses;

	//! joint remap array. stores actual index of joint data. -1 if joint doesn't participate in this animation.
	s16* m_joint_remap;
};
    
//---------------------------------------------------------------------------------------

inline AnimationClip::JointPose& AnimationClip::JointPose::operator-=( const JointPose& rhs )
{
    translation -= rhs.translation;
    scale -= rhs.scale;
    
    // Q1 * Q2 = Q3
    // Q2 = Q3 * Q1^-1
    Quaternion inv = rhs.rotation.inverse();
    inv.normalize();
    rotation = inv * rotation;
    
    return *this;
}

//---------------------------------------------------------------------------------------

inline void AnimationClip::JointPose::AdditiveAdd(const JointPose &pose_b, float factor)
{
    translation += pose_b.translation * factor;
    rotation = Quaternion::SLERP(rotation, rotation * pose_b.rotation, factor);
    scale += pose_b.scale * factor;
}

//---------------------------------------------------------------------------------------
    
inline AnimationClip::JointPose::JointPose()
{
}
    
//---------------------------------------------------------------------------------------
    
inline AnimationClip::JointPose::JointPose(const JointPose& p)
{
    translation = p.translation;
    rotation = p.rotation;
    scale = p.scale;
}

//---------------------------------------------------------------------------------------
    
inline void AnimationClip::JointPose::toMatrix(Matrix4x4& out) const
{
    out.MakeScale(scale, scale, scale);
    
    Matrix4x4 rot;
    rotation.toMatrix(rot);
    
    out *= rot;
    
    out.matrix[12] = translation.x;
    out.matrix[13] = translation.y;
    out.matrix[14] = translation.z;
}
    
//---------------------------------------------------------------------------------------
    
inline void AnimationClip::JointPose::MakeLerp( const JointPose& poseA, const JointPose& poseB, float factor )
{
    translation = Vec3::LERP(poseA.translation, poseB.translation, factor);
    rotation = Quaternion::SLERP(poseA.rotation, poseB.rotation, factor);
    scale = Math::Lerp(poseA.scale, poseB.scale, factor);
}
	
//---------------------------------------------------------------------------------------

inline StringId AnimationClip::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------

inline float AnimationClip::GetDuration(bool looped) const
{
    return looped ? m_frame_count / m_frames_per_ms : (m_frame_count-1) / m_frames_per_ms;
}

//---------------------------------------------------------------------------------------

inline float AnimationClip::GetSampleTime(float local_time_ms) const
{
    return local_time_ms * m_frames_per_ms;
}

//---------------------------------------------------------------------------------------

inline const AnimationClip::JointPose& AnimationClip::GetJointPose( u32 frame, u32 joint_idx ) const
{
    u32 array_index = frame * m_animated_joint_count + joint_idx;
    
    ENGINE_ASSERT(array_index < m_animated_joint_count * m_frame_count, "joint out of bounds");
    return m_joint_poses[ array_index ];
}
    
//---------------------------------------------------------------------------------------
    
inline bool	AnimationClip::HasJointPose(s16 joint_idx) const
{
    return m_joint_remap[joint_idx] != -1;
}
    
//---------------------------------------------------------------------------------------
    
inline void AnimationClip::GetJointPose(float local_time_ms, s16 jointIdx, JointPose& outPose, bool looped) const
{
    ENGINE_ASSERT(HasJointPose(jointIdx), "");
    
    s16 realJointIdx = m_joint_remap[jointIdx];
    
    float sample_time = GetSampleTime(local_time_ms);
    
    // blended samples.
    u32 lower_sample = (u32)Math::Floor<float>(sample_time);
    u32 upper_sample = (u32)Math::Ceil<float>(sample_time);
    
    // blend factor.
    float factor = sample_time - (float)lower_sample;
    
    if( looped )
    {
        // for looped clip - blend last frame with the first one.
        if( upper_sample == m_frame_count )
            upper_sample = 0;
    }
    
    const JointPose& lower_pose = GetJointPose(lower_sample, realJointIdx);
    const JointPose& upper_pose = GetJointPose(upper_sample, realJointIdx);
    
    // sample blending.
    outPose.MakeLerp(lower_pose, upper_pose, factor);
}
    
//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------
