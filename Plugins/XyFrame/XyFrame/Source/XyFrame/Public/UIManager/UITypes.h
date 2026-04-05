// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UITypes.generated.h"

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

UENUM(BlueprintType)
enum class EUIState : uint8
{
    Hidden      UMETA(DisplayName = "Hidden"),
    Showing     UMETA(DisplayName = "Showing"),
    Visible     UMETA(DisplayName = "Visible"),
    Hiding      UMETA(DisplayName = "Hiding")
};

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

UENUM(BlueprintType)
enum class EUIInputMode : uint8
{
    UIOnly      UMETA(DisplayName = "UI Only"),
    GameOnly    UMETA(DisplayName = "Game Only"),
    UIAndGame   UMETA(DisplayName = "UI and Game"),
    None        UMETA(DisplayName = "None")
};

UENUM(BlueprintType)
enum class EUIInputEvent : uint8
{
    Started     UMETA(DisplayName = "Started"),
    Triggered   UMETA(DisplayName = "Triggered"),
    Completed   UMETA(DisplayName = "Completed"),
    Canceled    UMETA(DisplayName = "Canceled")
};

UENUM(BlueprintType)
enum class EUIOpenPolicy : uint8
{
    Additive    UMETA(DisplayName = "Additive"),
    Replace     UMETA(DisplayName = "Replace"),
    Exclusive   UMETA(DisplayName = "Exclusive")
};

UENUM(BlueprintType)
enum class EUIPanelType : uint8
{
    MainPanel   UMETA(DisplayName = "Main Panel"),
    PopUp       UMETA(DisplayName = "PopUp"),
    Notification UMETA(DisplayName = "Notification"),
    Other       UMETA(DisplayName = "Other")
};