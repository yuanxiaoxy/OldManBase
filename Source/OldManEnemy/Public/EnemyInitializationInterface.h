// 文件：EnemyInitializationInterface.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FEnemyLocationInfo.h" // 包含你的结构体
#include "EnemyInitializationInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyInitializationInterface : public UInterface
{
    GENERATED_BODY()
};

class OLDMANENEMY_API IEnemyInitializationInterface
{
    GENERATED_BODY()

public:
    // 声明一个初始化方法，参数是你的EnemyInfo结构体
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Enemy")
    void InitializeEnemy(const FEnemyLocationInfo& EnemyInfo);
};
