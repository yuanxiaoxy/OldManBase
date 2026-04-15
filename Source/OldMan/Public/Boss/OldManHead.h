// OldManHead.h
// 定义老人头类，是 Boss 战的核心实体。负责旋转控制、随机晃动、进度管理、阶段切换。

#pragma once


#include "OldManHead.generated.h"



/**
 * 老人头类。
 * - 接受摇杆输入，转换为 Yaw 和 Pitch 旋转。
 * - 随机晃动影响旋转。
 * - 管理当前进度和阶段，触发阶段切换事件。
 * - 作为进度条管理器，持有 Boss 和摇杆引用，控制全局光束标志。
 */
UCLASS()
class OLDMAN_API AOldManHead : public AActor
{
	GENERATED_BODY()

public:

private:

};