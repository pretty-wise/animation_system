#include "engine/animation/AnimController.h"
#include "engine/animation/Skeleton.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

AnimController::AnimController()
: m_skeleton(nullptr)
, m_hierarchy(nullptr)
, m_skinning_palette(nullptr)
, m_layers(nullptr)
, m_layer_count(0)
{
    
}
    
//---------------------------------------------------------------------------------------
    
void AnimController::Initialize(Skeleton *skeleton, u32 layer_count)
{
    m_skeleton = skeleton;
    
    m_layers = new AnimLayer[layer_count];
    m_layer_count = layer_count;
    
    m_hierarchy = AnimHierarchy::CreateFromSkeleton(m_skeleton);
    
    m_skinning_palette = new Matrix4x4[m_skeleton->GetJointCount()];
}
    
//---------------------------------------------------------------------------------------
    
void AnimController::Release()
{
    delete m_hierarchy;
    
    m_skeleton = nullptr;
    
    delete [] m_layers;
    m_layer_count = 0;
    
    delete [] m_skinning_palette;
    m_skinning_palette = nullptr;
}
    
//---------------------------------------------------------------------------------------
    
void AnimController::Update( float delta_ms )
{
    for( u32 i = 0 ; i < m_layer_count; ++i )
        m_layers[i].Update( delta_ms );
}
    
//---------------------------------------------------------------------------------------
    
AnimLayer& AnimController::GetLayer( u16 index )
{
    ENGINE_ASSERT(index < m_layer_count, "layer out of bounds");
    return m_layers[index];
}
    
//---------------------------------------------------------------------------------------
    
void AnimController::GetJointPose( u16 joint_idx, AnimationClip::JointPose& output_pose )
{
    // layer 0 has to produce full skeletal pose. 
    if( m_layer_count > 0 && m_layers[0].Active() )
    {
        m_layers[0].GetJointPose(joint_idx, output_pose);
        
        AnimationClip::JointPose layer_pose;
        
        // blend all other layers on top layer 0.
        for( u32 i = 1; i < m_layer_count; ++i )
        {
            AnimLayer& layer = m_layers[i];
            
            if( layer.Active() )
            {
                bool layer_has_pose = layer.GetJointPose(joint_idx, layer_pose);
                
                if( layer_has_pose )
                {
                    switch( layer.GetType() )
                    {
                        case LayerType::Lerp:
                            output_pose.MakeLerp(output_pose, layer_pose, layer.GetBlendFactor());
                            break;
                        case LayerType::Additive:
                            output_pose.AdditiveAdd(layer_pose, layer.GetBlendFactor());
                            break;
                    }
                }
            }
        }
    }
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

