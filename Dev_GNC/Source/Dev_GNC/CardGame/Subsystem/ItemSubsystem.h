// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemSubsystem.generated.h"

class UDEquipment;

/**
 * UItemSubsystem
 * 플레이어가 보유한 장비 목록을 관리하는 게임인스턴스 서브시스템.
 * Unity ItemManager (MonoSingleton) 대응.
 */
UCLASS()
class UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Item")
	void AddEquip(UDEquipment* Equip);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Item")
	void RemoveEquip(UDEquipment* Equip);

	UFUNCTION(BlueprintPure, Category = "CardGame|Item")
	int32 GetEquipCount() const { return EquipList.Num(); }

	const TArray<TObjectPtr<UDEquipment>>& GetEquips() const { return EquipList; }

	UFUNCTION(BlueprintCallable, Category = "CardGame|Item")
	void ResetAll();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Item")
	TArray<TObjectPtr<UDEquipment>> EquipList;
};
