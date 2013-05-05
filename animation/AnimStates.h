#pragma once

#include "engine/animation/AnimBlendTree.h"

//---------------------------------------------------------------------------------------
namespace Engine{
//---------------------------------------------------------------------------------------

class AnimStates
{
public:
    class State;
    
    class Transition
    {
    public:
        Transition();
        
        inline StringId GetName() const { return m_name; }
        
        inline float GetBlendTime() const { return m_blend_time_ms; }
        
        inline bool IsSynced() const { return m_sync; }
        
        inline const State* GetDestinationState() const { return m_destination; }
        
        friend class AnimStates;
    private:
        // transition name.
        StringId m_name;
        
        // destination state.
        const State* m_destination;
        
        // transition fade time in milliseconds.
        float m_blend_time_ms;
        
        // if true, animation tree time should be synced with start of the animation blended with.
        bool m_sync;
    };
    
    class State{
    public:
        State();
        ~State();
        
        inline StringId GetName() const { return m_name; }
        
        const Transition* FindTransition( StringId name ) const;
        
        const AnimBlendTree& GetBlendTree() const;
        
        friend class AnimStates;
    private:
        // states name.
        StringId m_name;
        
        // points to the first transition in m_transitions within AnimStates (may be null).
        Transition* m_transitions;
        
        // number of transitions from this state.
        u16 m_num_transitions;
        
        // states animation tree.
        AnimBlendTree m_tree;
    };
    
public:
    // creates AnimStates instance from xml data.
    static AnimStates* CreatFromData( s8* data, streamsize length );
    
public:
    AnimStates();
    ~AnimStates();
    
    const State* FindState( StringId name ) const;
    
    // returns maximum size of a blend tree (number of nodes) in any state.
    u16 GetMaxNodeCount() const;
    
private:
    // all state transitions. refferenced from state objects.
    Transition* m_transitions;
    
    // animation states.
    State* m_states;
    
    // total number of defined transitions.
    u16 m_num_transitions;
    
    // number of states.
    u16 m_num_states;
};
    
//---------------------------------------------------------------------------------------
} // namespace Engine
//---------------------------------------------------------------------------------------