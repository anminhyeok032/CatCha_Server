#pragma once
#include "State.h"

template<class entity_type>
class StateMachine
{
    entity_type* owner_;

    State<entity_type>* current_state_;

    State<entity_type>* previous_state_;

public:

    StateMachine(entity_type* owner) :
        owner_(owner),
        current_state_(NULL),
        previous_state_(NULL)
    {}

    virtual ~StateMachine() {}

    void SetCurrentState(State<entity_type>* s) { current_state_ = s; }
    void SetPreviousState(State<entity_type>* s) { previous_state_ = s; }

    //현재 상태의 Execute 실행
    void  Update(float elapsed)const
    {
        if (current_state_)
        {
            current_state_->Execute(owner_, elapsed);
        }
    }

    //상태 변경: 기존 상태의 exit 호출 후 변경한 뒤 새로운 상태의 enter를 호출한다.
    void  ChangeState(State<entity_type>* pNewState)
    {
        previous_state_ = current_state_;

        current_state_->Exit(owner_);

        current_state_ = pNewState;

        current_state_->Enter(owner_);
    }

    //이전 상태로 되돌린다.
    void  RevertToPreviousState()
    {
        ChangeState(previous_state_);
    }

    // 현재 상태가 인자로 들어온 상태와 동일한지 체크
    bool  isInState(const State<entity_type>& st)const
    {
        return typeid(*current_state_) == typeid(st);
    }

    State<entity_type>* CurrentState()  const { return current_state_; }
    State<entity_type>* PreviousState() const { return previous_state_; }


};
