// Copyright GNC Project. All Rights Reserved.

#include "LobbyPageWidget.h"
#include "CardGameHUD.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "DeckSubsystem.h"
#include "PawnSubsystem.h"
#include "LobbySubsystem.h"
#include "PlayerDataSubsystem.h"
#include "ItemSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogLobbyPage, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void ULobbyPageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	// 서브시스템 캐시
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		DeckSubRef   = GI->GetSubsystem<UDeckSubsystem>();
		PawnSubRef   = GI->GetSubsystem<UPawnSubsystem>();
		LobbySubRef  = GI->GetSubsystem<ULobbySubsystem>();
		PlayerSubRef = GI->GetSubsystem<UPlayerDataSubsystem>();
	}

	// 버튼 바인딩
	if (ButtonStartBattle) ButtonStartBattle->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandleStartBattleClicked);
	if (ButtonDeckSetting) ButtonDeckSetting->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandleDeckSettingClicked);
	if (ButtonPawnManage)  ButtonPawnManage->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandlePawnManageClicked);
	if (ButtonShop)        ButtonShop->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandleShopClicked);
	if (ButtonRecruit)     ButtonRecruit->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandleRecruitClicked);
	if (ButtonExit)        ButtonExit->OnClicked.AddDynamic(this, &ULobbyPageWidget::HandleExitClicked);
}

void ULobbyPageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();

	EnsureTestData();

	// 상점/모집 데이터 보장
	if (ULobbySubsystem* LobbySub = LobbySubRef.Get())
	{
		LobbySub->EnsureLobbyData();
	}

	RefreshInfoDisplay();
}

void ULobbyPageWidget::NativePreClose()
{
	if (ButtonStartBattle) ButtonStartBattle->OnClicked.RemoveAll(this);
	if (ButtonDeckSetting) ButtonDeckSetting->OnClicked.RemoveAll(this);
	if (ButtonPawnManage)  ButtonPawnManage->OnClicked.RemoveAll(this);
	if (ButtonShop)        ButtonShop->OnClicked.RemoveAll(this);
	if (ButtonRecruit)     ButtonRecruit->OnClicked.RemoveAll(this);
	if (ButtonExit)        ButtonExit->OnClicked.RemoveAll(this);

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Handlers
// ────────────────────────────────────────────────────────────

void ULobbyPageWidget::HandleStartBattleClicked()
{
	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (!DeckSub) return;

	TArray<UDPawn*> ActivePawns = DeckSub->GetActivePawns();
	if (ActivePawns.Num() == 0)
	{
		UE_LOG(LogLobbyPage, Warning, TEXT("StartBattle: 편성된 폰이 없습니다"));
		return;
	}

	UGameplayStatics::OpenLevel(this, FName("Stage"));
}

void ULobbyPageWidget::HandleDeckSettingClicked()
{
	if (ACardGameHUD* HUD = OwningHUD.Get())
	{
		HUD->OpenPage(DeckSettingPageClass);
	}
}

void ULobbyPageWidget::HandlePawnManageClicked()
{
	if (ACardGameHUD* HUD = OwningHUD.Get())
	{
		HUD->OpenPage(PawnManagePageClass);
	}
}

void ULobbyPageWidget::HandleShopClicked()
{
	if (ACardGameHUD* HUD = OwningHUD.Get())
	{
		HUD->OpenPage(ShopPageClass);
	}
}

void ULobbyPageWidget::HandleRecruitClicked()
{
	if (ACardGameHUD* HUD = OwningHUD.Get())
	{
		HUD->OpenPage(RecruitPageClass);
	}
}

void ULobbyPageWidget::HandleExitClicked()
{
	// 서브시스템 리셋 후 타이틀로 복귀
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (auto* Sub = GI->GetSubsystem<UPlayerDataSubsystem>()) Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UPawnSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UDeckSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UItemSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<ULobbySubsystem>())      Sub->ResetAll();
	}

	UE_LOG(LogLobbyPage, Log, TEXT("Exit clicked — returning to title"));
	UGameplayStatics::OpenLevel(this, FName("Title"));
}

// ────────────────────────────────────────────────────────────
// Private
// ────────────────────────────────────────────────────────────

void ULobbyPageWidget::RefreshInfoDisplay()
{
	if (TextGold && PlayerSubRef.IsValid())
	{
		TextGold->SetText(FText::AsNumber(PlayerSubRef->Gold));
	}
	if (TextPawnCount && PawnSubRef.IsValid())
	{
		TextPawnCount->SetText(FText::AsNumber(PawnSubRef->GetPawnCount()));
	}
}

void ULobbyPageWidget::EnsureTestData()
{
	// Unity LobbyScript.OnEnterScene() 대응: Pawn이 없으면 테스트 데이터 생성
	UPawnSubsystem* PawnSub = PawnSubRef.Get();
	if (!PawnSub || PawnSub->GetPawnCount() > 0) return;

	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (DeckSub)
	{
		DeckSub->InitTestData();
		UE_LOG(LogLobbyPage, Log, TEXT("EnsureTestData: 테스트 데이터 생성 완료"));
	}
}
