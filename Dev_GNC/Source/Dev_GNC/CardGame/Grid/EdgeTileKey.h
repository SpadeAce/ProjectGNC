// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdgeTileKey.generated.h"

/**
 * FEdgeTileKey
 * 두 인접 타일 사이의 Edge를 식별하는 정규화된 좌표 쌍 키.
 * 순서 무관하게 동일한 Edge를 가리킨다: (A,B) == (B,A).
 */
USTRUCT(BlueprintType)
struct FEdgeTileKey
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	FIntPoint PosA = FIntPoint::ZeroValue;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	FIntPoint PosB = FIntPoint::ZeroValue;

	FEdgeTileKey() = default;

	FEdgeTileKey(FIntPoint A, FIntPoint B)
	{
		// 정규화: X 우선, Y 차순으로 작은 쪽이 PosA
		if (Compare(A, B) <= 0)
		{
			PosA = A;
			PosB = B;
		}
		else
		{
			PosA = B;
			PosB = A;
		}
	}

	bool operator==(const FEdgeTileKey& Other) const
	{
		return PosA == Other.PosA && PosB == Other.PosB;
	}

	bool operator!=(const FEdgeTileKey& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FEdgeTileKey& Key)
	{
		uint32 Hash = GetTypeHash(Key.PosA.X);
		Hash = HashCombine(Hash, GetTypeHash(Key.PosA.Y));
		Hash = HashCombine(Hash, GetTypeHash(Key.PosB.X));
		Hash = HashCombine(Hash, GetTypeHash(Key.PosB.Y));
		return Hash;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("Edge(%d,%d - %d,%d)"), PosA.X, PosA.Y, PosB.X, PosB.Y);
	}

private:
	static int32 Compare(FIntPoint A, FIntPoint B)
	{
		if (A.X != B.X) return A.X < B.X ? -1 : 1;
		if (A.Y != B.Y) return A.Y < B.Y ? -1 : 1;
		return 0;
	}
};
