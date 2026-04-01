// DeathSafeAreaTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Math/GenericOctree.h"
#include "DeathSafeAreaTypes.generated.h"

UENUM(BlueprintType)
enum class EOctreeMode : uint8
{
    DefaultSafe      UMETA(DisplayName = "默认安全（需标记死亡区域）"),
    DefaultDeath     UMETA(DisplayName = "默认死亡（需标记安全区域）")
};

USTRUCT(BlueprintType)
struct FAreaElement
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Area")
    FBoxSphereBounds Bounds;

    UPROPERTY(BlueprintReadWrite, Category = "Area")
    FColor DebugColor;

    UPROPERTY(BlueprintReadWrite, Category = "Area")
    FString AreaName;

    FAreaElement() : DebugColor(FColor::White) {}
    FAreaElement(const FBoxSphereBounds& InBounds, const FColor& InColor, const FString& InName)
        : Bounds(InBounds), DebugColor(InColor), AreaName(InName) {
    }

    FBoxSphereBounds GetBounds() const { return Bounds; }
};

struct FAreaOctreeSemantics
{
    using ElementType = FAreaElement;
    typedef FDefaultAllocator ElementAllocator;

    static const FBoxSphereBounds& GetBoundingBox(const ElementType& Element) { return Element.Bounds; }
    static const FVector& GetCenter(const ElementType& Element) { return Element.Bounds.Origin; }
    static float GetRadius(const ElementType& Element) { return Element.Bounds.SphereRadius; }
    static bool AreEqual(const ElementType& A, const ElementType& B) { return A.Bounds.GetBox() == B.Bounds.GetBox(); }

    // 必须提供 SetElementId，即使为空实现
    static void SetElementId(const ElementType& Element, FOctreeElementId2 Id)
    {
        // 不需要存储 ID，留空即可
    }

    enum { MaxElementsPerLeaf = 16 };
    enum { MaxNodeDepth = 12 };          // 添加最大深度常量
    enum { UseFoldableBounds = 0 };
};

using FAreaOctree = TOctree2<FAreaElement, FAreaOctreeSemantics>;

USTRUCT(BlueprintType)
struct FAreaQueryResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Area")
    bool bIsInArea = false;

    UPROPERTY(BlueprintReadOnly, Category = "Area")
    FString AreaName;

    UPROPERTY(BlueprintReadOnly, Category = "Area")
    FBox AffectedBounds;
};