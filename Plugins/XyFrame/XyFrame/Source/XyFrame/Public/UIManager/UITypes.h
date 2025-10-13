// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UITypes.generated.h"

// UIͼ��ö��
UENUM(BlueprintType)
enum class EUIPanelLayer : uint8
{
    None        UMETA(DisplayName = "None"),
    Rearmost    UMETA(DisplayName = "Rearmost"),
    Rear        UMETA(DisplayName = "Rear"),
    Middle      UMETA(DisplayName = "Middle"),
    Front       UMETA(DisplayName = "Front"),
    ForeFront   UMETA(DisplayName = "ForeFront")
};

// UI״̬
UENUM(BlueprintType)
enum class EUIState : uint8
{
    Hidden      UMETA(DisplayName = "Hidden"),
    Showing     UMETA(DisplayName = "Showing"),
    Visible     UMETA(DisplayName = "Visible"),
    Hiding      UMETA(DisplayName = "Hiding")
};

// �ؼ�����ö��
UENUM(BlueprintType)
enum class EUIControlType : uint8
{
    Button      UMETA(DisplayName = "Button"),
    Text        UMETA(DisplayName = "Text"),
    Image       UMETA(DisplayName = "Image"),
    Slider      UMETA(DisplayName = "Slider"),
    CheckBox    UMETA(DisplayName = "CheckBox"),
    ProgressBar UMETA(DisplayName = "ProgressBar")
};