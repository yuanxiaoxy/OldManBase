// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonManager.h"  // ֱ�Ӱ���ͷ�ļ�
#include "SingletonBase.generated.h"

/**
 * �������� - ���е����඼Ӧ�ü̳������
 */
UCLASS(Abstract)
class XYFRAME_API USingletonBase : public UObject
{
    GENERATED_BODY()

public:
    virtual ~USingletonBase() {}

    // �麯��������������д�Խ��г�ʼ��
    virtual void InitializeSingleton() {}

    // �麯��������������д�Խ�������
    UFUNCTION()
    virtual void DestroyCurSingleton() {}
};

/**
 * ��ǿ�ĵ���ģ�� - �Զ�ע�ᵽ������
 */
template <typename T>
class TSingleton
{
public:
    // ��ȡ����ʵ��
    static T* GetInstance()
    {
        if (!SingletonInstance || !IsValid(SingletonInstance))
        {
            // ��������ʵ��
            SingletonInstance = NewObject<T>();
            if (SingletonInstance)
            {
                SingletonInstance->AddToRoot(); // ��ֹGC

                // �Զ�ע�ᵽ����������
                if (USingletonManager* Manager = USingletonManager::GetInstance())
                {
                    Manager->RegisterSingleton(SingletonInstance);
                }

                // ���ó�ʼ������
                SingletonInstance->InitializeSingleton();

                UE_LOG(LogTemp, Log, TEXT("Singleton created and registered: %s"), *SingletonInstance->GetClass()->GetName());
            }
        }
        return SingletonInstance;
    }

    // ���ٵ���ʵ��
    static void DestroyInstance()
    {
        if (SingletonInstance && IsValid(SingletonInstance))
        {
            // �ӹ�����ע��
            if (USingletonManager* Manager = USingletonManager::GetInstance())
            {
                Manager->UnregisterSingleton(SingletonInstance->GetClass());
            }

            SingletonInstance->RemoveFromRoot();
            SingletonInstance->ConditionalBeginDestroy();
            SingletonInstance = nullptr;

            UE_LOG(LogTemp, Log, TEXT("Singleton destroyed and unregistered"));
        }
    }

    // ��鵥���Ƿ����
    static bool IsInstanceValid()
    {
        return SingletonInstance != nullptr && IsValid(SingletonInstance);
    }

protected:
    TSingleton() = default;

private:
    static T* SingletonInstance;
};

// ��̬��Ա����
template <typename T>
T* TSingleton<T>::SingletonInstance = nullptr;

/**
 * ���������� - ������ͼ�ɵ��÷���
 */
#define DECLARE_SINGLETON(ClassName) \
public: \
    /* C++��̬���� */ \
    static ClassName* GetInstance() { return TSingleton<ClassName>::GetInstance(); } \
    static void DestroyInstance() { TSingleton<ClassName>::DestroyInstance(); } \
    static bool IsInstanceValid() { return TSingleton<ClassName>::IsInstanceValid(); } 