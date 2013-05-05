#pragma once

#include "engine/animation/Animation.h"
#include <vector>

#include "rapidxml/rapidxml.hpp"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class AnimationClip;
    
namespace BlendNodeType{
    enum Enum{
        Undefined = -1,
        Value,
        Lerp,
        Additive
    };
}

//---------------------------------------------------------------------------------------

class AnimBlendTree
{
public:
    // single animation node is a part of a blend tree.
    // value node stores animation data.
    // other nodes store left, right child indexes and named blend factor.
    class Node
    {
    public:
        Node();
        Node( AnimationClip* clip, float globa_clock_ms, bool looped, float playback_rate );
        Node( BlendNodeType::Enum type, u16 left_index, u16 right_index, StringId factor_name, float factor_value );
        
        inline BlendNodeType::Enum GetType() const { return m_type; }
        
        inline u16 GetLeftIndex() const { return m_left_index; }
        
        inline u16 GetRightIndex() const { return m_right_index; }
        
        inline StringId GetFactorName() const { return m_factor_name; }
        
        inline float GetFactor() const { return m_blend_factor; }
        
        inline void SetFactor( float value ) { m_blend_factor = value; }
        
        inline const Animation& GetAnimation() const { return m_animation; }
        
        inline Animation& GetAnimation() { return m_animation; }
    private:
        // left tree node.
        u16 m_left_index;
        
        // right tree node.
        u16 m_right_index;
        
        // type of node.
        BlendNodeType::Enum m_type;
        
        // current blend factor. valid if Lerp, Additive or CrossFade node.
        float m_blend_factor;
        
        // name of blend node.
        StringId m_factor_name;
        
        // animation. valid if Value node.
        Animation m_animation;
    };
    
public:
    AnimBlendTree();
    AnimBlendTree( u16 capacity );
    ~AnimBlendTree();
    
    AnimBlendTree& operator=( const AnimBlendTree& rhs );
    
    inline u16 GetCount() const { return m_node_count; }
    
    inline u16 GetCapacity() const { return m_capacity; }
    
    inline bool IsValid() const { return m_node_count > 0; }
    
    void Clear();
    
    void Resize( u16 capacity );
    
    void Start( float current_time_ms );
    
    bool SetNodeFactor( StringId factor_name, float value );
    
    bool GetNodeFactor( StringId factor_name, float& value ) const;
    
    // returns local animation time of first animation in the tree.
    float GetLocalAnimationTime(float global_time) const;
    
    // returns true if pose has been calculated for this subtree.
    bool GetJointPose( float global_time_ms, s16 joint_idx, AnimationClip::JointPose& pose ) const;
    
    // used to prevent timer overflow.
    void FixAnimationStartTime( float rewind_time_ms );
    
    friend u16 read_blend_tree( AnimBlendTree& tree, rapidxml::xml_node<>* node );
private:
    // called recursivly to get final joint pose.
    bool GetJointPose( u16 current_index, float clock_time_ms, s16 joint_idx, AnimationClip::JointPose& pose ) const;
    
    // capacity size array of nodes.
    Node* m_nodes;
    
    // current numder of nodes.
    u16 m_node_count;
    
    // maximum number of nodes.
    u16 m_capacity;
};

//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------