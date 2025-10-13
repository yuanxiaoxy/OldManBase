// ResourceTableRow.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ResourceTableRow.generated.h"

// ��Դ����ö��
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

// ��Դ���нṹ
USTRUCT(BlueprintType)
struct FResourceTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    // ��ԴID��Ψһ��ʶ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FName ResourceID;

    // ��Դ��ʾ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FText DisplayName;

    // ��Դ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceCategory Category;

    // ��Դ·��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourcePath;

    // ��Դ���ͣ���ѡ���������Ͱ�ȫ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    TSubclassOf<UObject> ResourceClass;

    // ��Դ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FText Description;

    // �Ƿ�Ԥ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    bool bPreload = false;

    // �������ȼ�����ֵԽС���ȼ�Խ�ߣ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 LoadPriority = 0;

    FResourceTableRow()
        : Category(EResourceCategory::Other)
        , bPreload(false)
        , LoadPriority(0)
    {
    }
};