#pragma once

#include "CoreMinimal.h"
#include "EnemyPatrolPoint.h"
#include "FEnemyLocationInfo.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FEnemyLocationInfo
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
    AEnemyPatrolPoint* SpawnPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
    TArray<AEnemyPatrolPoint*> PatrolPath;  // 巡逻路径点蓝图类数组

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
    bool bIsGenerateOnce = false;

    UPROPERTY(BlueprintReadWrite, Category = "Enemy Info")
    int32 maxCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Info")
    int32 ID = -1;

    UPROPERTY()
    bool isActive = true;

    void DrawDebugInfo(UWorld* World, float lastTime = -1.0f) const
    {
        if (!World) return;
        FColor pointColor;
        bool bisLast = false;
        FColor pathColor;
        FColor spawnColor;
        if (bIsGenerateOnce)
        {
            pointColor = FColor::Red;
            pathColor = FColor::Yellow;
            spawnColor = FColor::Orange;
        }
        else
        {
            pointColor = FColor::Blue;
            pathColor = FColor::Purple;
            spawnColor = FColor::Green;
        }
        // 绘制生成点圆圈（绿色）
        if (SpawnPoint)
        {
            FVector SpawnLocation = SpawnPoint->GetActorLocation();
            
            // 绘制一个绿色圆圈表示生成点
            DrawDebugCircle(
                World,
                SpawnLocation,
                100.0f,  // 圆圈半径
                32,      // 分段数
                spawnColor,
                bisLast,   // 不持久化
                lastTime,   // 持续到下一帧
                0,       // 深度优先级
                3.0f,    // 线条粗细
                FVector(1, 0, 0),  // X轴
                FVector(0, 1, 0),  // Y轴（法线方向）
                false    // 不绘制坐标轴
            );

            // 在生成点位置绘制一个点
            DrawDebugPoint(
                World,
                SpawnLocation,
                15.0f,   // 点的大小
                spawnColor,
                true,
                lastTime
            );
        }

        // 绘制巡逻路径连线
        if (PatrolPath.Num() > 1)
        {
            for (int32 i = 0; i < PatrolPath.Num(); ++i)
            {
                if (!PatrolPath[i]) continue;

                FVector CurrentLocation = PatrolPath[i]->GetActorLocation();

                // 绘制路径点（蓝色点）
                DrawDebugPoint(
                    World,
                    CurrentLocation,
                    10.0f,
                    pointColor,
                    bisLast,
                    lastTime
                );

                // 绘制路径点序号
                DrawDebugString(
                    World,
                    CurrentLocation + FVector(0, 0, 50),  // 在点上方向偏移
                    FString::Printf(TEXT("%d"), i),
                    nullptr,
                    FColor::White,
                    lastTime,  // 持续到下一帧
                    true,    // 绘制阴影
                    1.5f     // 字体缩放
                );

                // 绘制连线
                if (i < PatrolPath.Num() - 1 && PatrolPath[i + 1])
                {
                    FVector NextLocation = PatrolPath[i + 1]->GetActorLocation();
                    DrawDebugLine(
                        World,
                        CurrentLocation,
                        NextLocation,
                        pathColor,
                        bisLast,  // 不持久化
                        lastTime,  // 持续到下一帧
                        0,       // 深度优先级
                        2.0f     // 线条粗细
                    );
                }

                // 绘制从最后一个点回到第一个点的连线（形成闭环）
               /* if (i == PatrolPath.Num() - 1 && PatrolPath[0])
                {
                    FVector FirstLocation = PatrolPath[0]->GetActorLocation();
                    DrawDebugLine(
                        World,
                        CurrentLocation,
                        FirstLocation,
                        pathColor,
                        false,
                        lastTime,
                        0,
                        2.0f
                    );
                }*/
            }
        }
        else if (PatrolPath.Num() == 1 && PatrolPath[0])
        {
            // 只有一个路径点时，只绘制点
            FVector Location = PatrolPath[0]->GetActorLocation();
            DrawDebugPoint(
                World,
                Location,
                10.0f,
                pointColor,
                bisLast,
                lastTime
            );
        }
    }

    // 检查数据有效性的辅助函数
    bool IsValid() const
    {
        return SpawnPoint != nullptr && PatrolPath.Num() > 0;
    }
};