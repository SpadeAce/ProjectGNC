// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DObject.generated.h"

/**
 * UDObject
 * 모든 데이터 오브젝트의 베이스 클래스.
 * 고유 InstanceId를 자동 부여한다.
 */
UCLASS(Abstract, BlueprintType)
class UDObject : public UObject
{
	GENERATED_BODY()

public:
	UDObject();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int64 InstanceId = 0;

private:
	static int64 NextInstanceId;
};

/**
 * UDItem
 * 아이템 베이스 클래스. DCard가 상속.
 */
UCLASS(BlueprintType)
class UDItem : public UDObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int32 ItemId = 0;
};
