// ASafeAreaVolume.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "SafeAreaVolume.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (ShowCategories = "Physics|Collision"))
class OLDMANITEM_API ASafeAreaVolume : public AVolume
{
    GENERATED_BODY()

public:
    ASafeAreaVolume();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeArea")
    FString AreaName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeArea")
    FColor DebugColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SafeArea")
    bool bIsActive = true;

    UFUNCTION(BlueprintCallable, Category = "SafeArea")
    FBox GetSafeBounds() const;

    UFUNCTION(BlueprintCallable, Category = "SafeArea")
    void UpdateSafeAreaInManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    class UDeathSafeAreaManagerComponent* GetManager();
    void RegisterToManager();
    void UnregisterFromManager();

private:
    TWeakObjectPtr<class UDeathSafeAreaManagerComponent> CachedManager;
};