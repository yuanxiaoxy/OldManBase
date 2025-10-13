// ResourceTableRow.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ResourceTableRow.generated.h"

// 资源分类枚举
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
    UI UMETA(DisplayName = "UIResource"),
    Character UMETA(DisplayName = "CharacterResource"),
    Weapon UMETA(DisplayName = "WeaponResource"),
    Environment UMETA(DisplayName = "EnvironmentResource"),
    Effect UMETA(DisplayName = "EffectResource"),
    Audio UMETA(DisplayName = "VoiceResource"),
    Other UMETA(DisplayName = "OtherResource")
};

// 资源表行结构
USTRUCT(BlueprintType)
struct FResourceTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 资源ID（唯一标识）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FName ResourceID;

    // 资源显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FText DisplayName;

    // 资源分类
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceCategory Category;

    // 资源路径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourcePath;

    // 资源类型（可选，用于类型安全）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    TSubclassOf<UObject> ResourceClass;

    // 资源描述
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FText Description;

    // 是否预加载
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bPreload = false;

    // 加载优先级（数值越小优先级越高）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 LoadPriority = 0;

    FResourceTableRow()
        : Category(EResourceCategory::Other)
        , bPreload(false)
        , LoadPriority(0)
    {
    }
};