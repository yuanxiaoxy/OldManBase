// ADeathAreaVolume.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "DeathAreaVolume.generated.h"

UCLASS(BlueprintType, Blueprintable, meta = (ShowCategories = "Physics|Collision"))
class OLDMANITEM_API ADeathAreaVolume : public AVolume
{
    GENERATED_BODY()

public:
    ADeathAreaVolume();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathArea")
    FString AreaName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathArea")
    FColor DebugColor = FColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathArea")
    bool bIsActive = true;

    UFUNCTION(BlueprintCallable, Category = "DeathArea")
    FBox GetDeathBounds() const;

    UFUNCTION(BlueprintCallable, Category = "DeathArea")
    void UpdateDeathAreaInManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    class UDeathSafeAreaManagerComponent* GetManager();
    void RegisterToManager();
    void UnregisterFromManager();

private:
    TWeakObjectPtr<class UDeathSafeAreaManagerComponent> CachedManager;
};