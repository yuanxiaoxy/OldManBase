// Fill out your copyright notice in the Description page of Project Settings.


#include "PopUpPanel/OldManPopUpUIBase.h"
#include "UIManager/UIManager.h"

void UOldManPopUpUIBase::InternalShowUI(UObject* Data)
{
	Super::InternalShowUI(Data);

    if (PopupAnimation)
    {
        // 第1步：创建一个委托对象
        FWidgetAnimationDynamicEvent AnimationEvent;

        // 第2步：使用 BindDynamic 绑定回调函数
        AnimationEvent.BindDynamic(this, &UOldManPopUpUIBase::OnPopupAnimationFinished);

        // 第3步：将委托绑定到动画的结束事件上
        BindToAnimationFinished(PopupAnimation, AnimationEvent);
    }

    PlayAnimation(PopupAnimation);
}

void UOldManPopUpUIBase::OnPopupAnimationFinished()
{
    CloseUI();
}
