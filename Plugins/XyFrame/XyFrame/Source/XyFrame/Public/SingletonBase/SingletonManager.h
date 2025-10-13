// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonManager.generated.h"

// ǰ������
class USingletonBase;

/**
 * ���������� - ���̳�USingletonBase
 */
UCLASS()
class XYFRAME_API USingletonManager : public UObject
{
    GENERATED_BODY()

public:
    // ��ȡ������ʵ������ͳ����ģʽ��
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static USingletonManager* GetInstance();

    // ��ʼ��������
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Initialize();

    // �رչ�����
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Shutdown();

    // ע�ᵥ��
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void RegisterSingleton(UObject* Singleton);

    // ע������
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void UnregisterSingleton(UClass* SingletonClass);

    // ��ȡ����
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    UObject* GetSingleton(UClass* SingletonClass) const;

    // ����ָ�����͵ĵ���
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroySingleton(UClass* SingletonClass);

    // �������е���
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroyAllSingletons();

    // ��ȡ��������
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    int32 GetSingletonCount() const;

    // ��ӡ���е�����Ϣ
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void PrintAllSingletons() const;

private:
    // �洢����ʵ��
    UPROPERTY()
    TMap<UClass*, UObject*> SingletonInstances;

    // ������ʵ��
    static USingletonManager* ManagerInstance;
};