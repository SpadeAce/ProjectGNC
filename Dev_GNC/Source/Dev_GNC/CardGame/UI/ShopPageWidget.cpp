// Copyright GNC Project. All Rights Reserved.

#include "ShopPageWidget.h"
#include "ShopCardItemWidget.h"
#include "CardGameHUD.h"
#include "DCard.h"
#include "CardGameRowTypes.h"
#include "LobbySubsystem.h"
#include "PlayerDataSubsystem.h"
#include "DeckSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogShopPage, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UShopPageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		LobbySubRef  = GI->GetSubsystem<ULobbySubsystem>();
		PlayerSubRef = GI->GetSubsystem<UPlayerDataSubsystem>();
		DeckSubRef   = GI->GetSubsystem<UDeckSubsystem>();
	}

	if (ButtonClose)   ButtonClose->OnClicked.AddDynamic(this, &UShopPageWidget::HandleCloseClicked);
	if (ButtonLevelUp) ButtonLevelUp->OnClicked.AddDynamic(this, &UShopPageWidget::HandleLevelUpClicked);

	if (ULobbySubsystem* LobbySub = LobbySubRef.Get())
	{
		LobbySub->OnShopListChanged.AddDynamic(this, &UShopPageWidget::HandleShopListChanged);
	}
}

void UShopPageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();

	RefreshShopDisplay();
	RefreshGold();
}

void UShopPageWidget::NativePreClose()
{
	if (ButtonClose)   ButtonClose->OnClicked.RemoveAll(this);
	if (ButtonLevelUp) ButtonLevelUp->OnClicked.RemoveAll(this);

	if (ULobbySubsystem* LobbySub = LobbySubRef.Get())
	{
		LobbySub->OnShopListChanged.RemoveDynamic(this, &UShopPageWidget::HandleShopListChanged);
	}

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Handlers
// ────────────────────────────────────────────────────────────

void UShopPageWidget::HandleCloseClicked()
{
	RequestClose();
}

void UShopPageWidget::HandleLevelUpClicked()
{
	ULobbySubsystem* LobbySub = LobbySubRef.Get();
	UPlayerDataSubsystem* PlayerSub = PlayerSubRef.Get();
	if (!LobbySub || !PlayerSub) return;

	if (!PlayerSub->SpendGold(LevelUpCost))
	{
		UE_LOG(LogShopPage, Warning, TEXT("상점 레벨업 실패: 골드 부족"));
		return;
	}

	if (!LobbySub->LevelUpShop())
	{
		// 최대 레벨 → 골드 환불
		PlayerSub->AddGold(LevelUpCost);
		UE_LOG(LogShopPage, Warning, TEXT("상점 레벨업 실패: 최대 레벨"));
		return;
	}

	LobbySub->GenerateShopList();
	RefreshShopDisplay();
	RefreshGold();
}

void UShopPageWidget::HandleBuyCard(UDCard* Card)
{
	if (!Card || !Card->Data) return;

	UPlayerDataSubsystem* PlayerSub = PlayerSubRef.Get();
	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	ULobbySubsystem* LobbySub = LobbySubRef.Get();
	if (!PlayerSub || !DeckSub || !LobbySub) return;

	if (!PlayerSub->SpendGold(Card->Data->Price))
	{
		UE_LOG(LogShopPage, Warning, TEXT("카드 구매 실패: 골드 부족 (필요: %d)"), Card->Data->Price);
		return;
	}

	DeckSub->AddCard(Card);
	LobbySub->RemoveShopCard(Card);

	RefreshShopDisplay();
	RefreshGold();

	UE_LOG(LogShopPage, Log, TEXT("카드 구매: %s (-%dG)"), *Card->Data->IdAlias, Card->Data->Price);
}

void UShopPageWidget::HandleShopListChanged()
{
	RefreshShopDisplay();
}

// ────────────────────────────────────────────────────────────
// Refresh
// ────────────────────────────────────────────────────────────

void UShopPageWidget::RefreshShopDisplay()
{
	if (!ScrollShopCards) return;
	ScrollShopCards->ClearChildren();

	ULobbySubsystem* LobbySub = LobbySubRef.Get();
	if (!LobbySub || !ShopCardItemClass) return;

	for (UDCard* Card : LobbySub->ShopCards)
	{
		if (!Card) continue;

		UShopCardItemWidget* Item = CreateWidget<UShopCardItemWidget>(this, ShopCardItemClass);
		if (!Item) continue;

		Item->SetData(Card);
		Item->OnBuyRequested.AddDynamic(this, &UShopPageWidget::HandleBuyCard);
		ScrollShopCards->AddChild(Item);
	}

	if (TextShopLevel && LobbySub)
	{
		TextShopLevel->SetText(FText::FromString(FString::Printf(TEXT("Lv.%d"), LobbySub->ShopLevel)));
	}
}

void UShopPageWidget::RefreshGold()
{
	if (TextGold && PlayerSubRef.IsValid())
	{
		TextGold->SetText(FText::AsNumber(PlayerSubRef->Gold));
	}
}
