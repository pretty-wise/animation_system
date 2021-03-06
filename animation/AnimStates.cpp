#include "engine/animation/AnimStates.h"
#include "engine/application/Application.h"
#include "engine/filesystem/Filesystem.h"

#include "rapidxml/rapidxml.hpp"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------
// State
//---------------------------------------------------------------------------------------

AnimStates::State::State()
: m_name(0)
, m_transitions(nullptr)
, m_num_transitions(0)
{
    
}

//---------------------------------------------------------------------------------------

AnimStates::State::~State()
{
}
    
//---------------------------------------------------------------------------------------
    
const AnimStates::Transition* AnimStates::State::FindTransition( StringId name ) const
{
    for( u16 i = 0; i < m_num_transitions; ++i )
    {
        if( m_transitions[i].GetName() == name )
        {
            return &m_transitions[i];
        }
    }
    
    return nullptr;
}
    
//---------------------------------------------------------------------------------------

const AnimBlendTree& AnimStates::State::GetBlendTree() const
{
    return m_tree;
}
    
//---------------------------------------------------------------------------------------
// Transitions
//---------------------------------------------------------------------------------------

AnimStates::Transition::Transition()
: m_name(0)
, m_destination(nullptr)
, m_blend_time_ms(0.f)
, m_sync(false)
{
    
}
    
//---------------------------------------------------------------------------------------
// AnimStates
//---------------------------------------------------------------------------------------

AnimStates::AnimStates()
: m_states(nullptr)
, m_transitions(nullptr)
, m_num_states(0)
, m_num_transitions(0)
{
    
}
    
//---------------------------------------------------------------------------------------
    
AnimStates::~AnimStates()
{
    delete [] m_states;
    delete [] m_transitions;
}

//---------------------------------------------------------------------------------------

u16 read_blend_tree( AnimBlendTree& tree, rapidxml::xml_node<>* node )
{
    const char* node_type = node->first_attribute("type") ? node->first_attribute("type")->value() : "";
    const char* node_value = node->value();
    bool looped = node->first_attribute("looped") ? !strcmp(node->first_attribute("looped")->value(), "true") : false;
    
    if( !strcmp(node_type, "clip") )
    {
        AnimationClip* clip_data;
        streamsize data_length;
        
        Application::GetFilesystem().ReadFile(node_value, (s8*&)clip_data, data_length);
        
        if( clip_data )
        {
            clip_data->FixPointers();
            
            if( tree.GetCount()+1 > tree.GetCapacity() )
                tree.Resize(tree.GetCount()+1);
            
            tree.m_nodes[tree.GetCount()] = AnimBlendTree::Node(clip_data, 0.f, looped, 1.f);
            tree.m_node_count++;
            return tree.GetCount()-1;
        }
    }
    else if( !strcmp(node_type, "additive") )
    {
        rapidxml::xml_node<>* leftxml = node->first_node("node");
        rapidxml::xml_node<>* rightxml = leftxml->next_sibling("node");
        
        const char* factor = node->first_attribute("factor-name") ? node->first_attribute("factor-name")->value() : "";
        float factor_value = node->first_attribute("factor-value") ? atof( node->first_attribute("factor-value")->value() ) : 1.f;
        
        StringId factor_name = COMPUTE_SID(factor);
        
        if( leftxml && rightxml )
        {
            u16 additive_index = tree.GetCount();
            
            if( tree.GetCount()+1 > tree.GetCapacity() )
                tree.Resize(tree.GetCount()+1);
            
            tree.m_node_count++;
            
            u16 left_index = read_blend_tree(tree, leftxml);
            u16 right_index = read_blend_tree(tree, rightxml);
            
            ENGINE_ASSERT(left_index != (u16)-1 && right_index != (u16)-1, "invalid tree");
            
            tree.m_nodes[additive_index] = AnimBlendTree::Node(BlendNodeType::Additive, left_index, right_index, factor_name, factor_value);
            return additive_index;
        }
    }
    else if( !strcmp(node_type, "lerp") )
    {
        rapidxml::xml_node<>* leftxml = node->first_node("node");
        rapidxml::xml_node<>* rightxml = leftxml->next_sibling("node");
        
        const char* factor = node->first_attribute("factor-name") ? node->first_attribute("factor-name")->value() : "";
        float factor_value = node->first_attribute("factor-value") ? atof( node->first_attribute("factor-value")->value() ) : 0.5f;
        
        StringId factor_name = COMPUTE_SID(factor);
        
        if( leftxml && rightxml )
        {
            u16 lerp_index = tree.GetCount();
            
            if( tree.GetCount()+1 > tree.GetCapacity() )
                tree.Resize(tree.GetCount()+1);
            
            tree.m_node_count++;
            
            u16 left_index = read_blend_tree(tree, leftxml);
            u16 right_index = read_blend_tree(tree, rightxml);
            
            ENGINE_ASSERT(left_index != (u16)-1 && right_index != (u16)-1, "invalid tree");
            
            tree.m_nodes[lerp_index] = AnimBlendTree::Node(BlendNodeType::Lerp, left_index, right_index, factor_name, factor_value);
            return lerp_index;
        }
    }
    
    return -1;
}
    
//---------------------------------------------------------------------------------------
    
AnimStates* AnimStates::CreatFromData( s8* data, streamsize length )
{
    // rapidxml expects a null terminated string, we have to fix data for that.
    s8* nulled_data = new s8[length+1];
    nulled_data[length] = '\0';
    memcpy(nulled_data, data, length);
    
    rapidxml::xml_document<> doc;
    
    doc.parse<0>(nulled_data);
    
    rapidxml::xml_node<>* state_node = doc.first_node("state");
    
    u16 num_states = 0;
    u16 num_transitions = 0;
    
    // calculate state count and transition count.
    while( state_node && state_node->first_attribute("name") )
    {
        rapidxml::xml_node<>* tree_node = state_node->first_node("node");
        
        if( tree_node )
        {
            num_states++;
            
            rapidxml::xml_node<>* transitions_node = state_node->first_node("transitions");
            
            rapidxml::xml_node<>* transition_node = transitions_node->first_node("transition");
            
            while( transition_node )
            {
                num_transitions++;
                
                transition_node = transition_node->next_sibling("transition");
            }
        }
        
        state_node = state_node->next_sibling("state");
    }
    
    AnimStates* result = nullptr;
    
    if( num_states > 0 )
    {
        // create state object.
        result = new AnimStates();
        result->m_states = new State[num_states];
        result->m_num_states = num_states;
        result->m_transitions = num_transitions > 0 ? new Transition[num_transitions] : nullptr;
        result->m_num_transitions = num_transitions;
        
        // fill the data.
        rapidxml::xml_node<>* state_node = doc.first_node("state");
        
        // read state data. have to read states first to lookup transition destinations afterwards.
        u16 cur_state_idx = 0;
        while( state_node )
        {
            State& state = result->m_states[cur_state_idx];
            
            state.m_name = COMPUTE_SID( state_node->first_attribute("name")->value() );
            
            read_blend_tree(state.m_tree, state_node->first_node("node"));
            
            cur_state_idx++;
            state_node = state_node->next_sibling("state");
        }
        
        // read transition data.
        state_node = doc.first_node("state");
        u16 cur_transition_idx = 0;
        cur_state_idx = 0;
        
        while( state_node )
        {
            State& state = result->m_states[cur_state_idx];
            
            rapidxml::xml_node<>* transitions_node = state_node->first_node("transitions");
            rapidxml::xml_node<>* transition_node = transitions_node ? transitions_node->first_node("transition") : nullptr;
            
            u16 num_state_transitions = 0;
            
            while( transition_node )
            {
                Transition& transition = result->m_transitions[cur_transition_idx];
                
                transition.m_name = transition_node->first_attribute("name") ? COMPUTE_SID( transition_node->first_attribute("name")->value() ) : 0;
                
                transition.m_blend_time_ms = transition_node->first_attribute("fade-time") ? atof(transition_node->first_attribute("fade-time")->value()) : 0.f;
                
                transition.m_sync = transition_node->first_attribute("sync") ? !strcmp(transition_node->first_attribute("sync")->value(), "true") : false;
                
                StringId dest = transition_node->first_attribute("target") ? COMPUTE_SID( transition_node->first_attribute("target")->value() ) : 0;
                transition.m_destination = result->FindState( dest );
                ENGINE_CHECK(transition.m_destination, "destination not found");
                
                num_state_transitions++;
                cur_transition_idx++;
                transition_node = transition_node->next_sibling("transition");
            }
            
            u16 total_transition_count = cur_transition_idx;
            
            state.m_transitions = nullptr;
            
            if( num_state_transitions > 0 )
                state.m_transitions = &result->m_transitions[total_transition_count-num_state_transitions];
            
            state.m_num_transitions = num_state_transitions;
            
            cur_state_idx++;
            state_node = state_node->next_sibling("state");
        }
    }
    
    delete [] nulled_data;

    return result;
}
    
//---------------------------------------------------------------------------------------
    
const AnimStates::State* AnimStates::FindState( StringId name ) const
{
    for( u16 i = 0; i < m_num_states; ++i )
    {
        if( m_states[i].GetName() == name )
            return &m_states[i];
    }
    
    return nullptr;
}
    
//---------------------------------------------------------------------------------------
    
u16 AnimStates::GetMaxNodeCount() const
{
    u16 max = 0;
    for( u16 i = 0; i < m_num_states; ++i )
    {
        u16 state_node_count = m_states[i].GetBlendTree().GetCount();
        
        if( state_node_count > max )
            max = state_node_count;
    }
    
    return max;
}
    
//---------------------------------------------------------------------------------------
}; //namespace Engine
//---------------------------------------------------------------------------------------

