// Copyright GNC Project. All Rights Reserved.

#include "CardGameHUD.h"
#include "ViewBase.h"
#include "PageWidget.h"
#include "PopupWidget.h"
#include "FloatingTextWidget.h"
#include "Components/PanelWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "CardGame/CardGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogCardGameHUD, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void ACardGameHUD::BeginPlay()
{
	Super::BeginPlay();

	InitRootWidget();
	InitFloatingTextPool();

	// GameMode에서 초기 페이지 클래스를 가져와 자동 열기
	if (const ACardGameMode* GM = Cast<ACardGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GM->InitialPageClass)
		{
			UE_LOG(LogCardGameHUD, Log, TEXT("InitialPageClass: %s"), *GM->InitialPageClass->GetName());
			OpenPage(GM->InitialPageClass);
		}
		else
		{
			UE_LOG(LogCardGameHUD, Warning, TEXT("GameMode found but InitialPageClass is not set"));
		}
	}
	else
	{
		UE_LOG(LogCardGameHUD, Warning, TEXT("GameMode is not ACardGameMode (actual: %s)"),
			*GetWorld()->GetAuthGameMode()->GetClass()->GetName());
	}

	UE_LOG(LogCardGameHUD, Log, TEXT("ACardGameHUD Initialized"));
}

// ────────────────────────────────────────────────────────────
// Page Stack
// ────────────────────────────────────────────────────────────

UPageWidget* ACardGameHUD::OpenPage(TSubclassOf<UPageWidget> PageClass, const FViewParam& Param)
{
	if (!PageClass || !PageLayer)
	{
		UE_LOG(LogCardGameHUD, Warning, TEXT("OpenPage: PageClass or PageLayer is null"));
		return nullptr;
	}

	// 이미 스택에 같은 클래스가 있으면 반환
	for (const auto& Page : PageStack)
	{
		if (Page && Page->GetClass() == PageClass)
		{
			UE_LOG(LogCardGameHUD, Log, TEXT("OpenPage: %s already in stack"), *PageClass->GetName());
			return Page;
		}
	}

	// 이전 페이지 숨김 처리
	if (PageStack.Num() > 0)
	{
		UPageWidget* PrevPage = PageStack.Last();
		if (PrevPage && !PrevPage->bKeepPreviousVisible)
		{
			PrevPage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 새 페이지 생성
	UPageWidget* NewPage = CreateWidget<UPageWidget>(GetOwningPlayerController(), PageClass);
	if (!NewPage)
	{
		UE_LOG(LogCardGameHUD, Error, TEXT("OpenPage: Failed to create widget %s"), *PageClass->GetName());
		return nullptr;
	}

	NewPage->SetOwningHUD(this);
	PageLayer->AddChild(NewPage);

	// CanvasPanel 슬롯을 전체 화면으로 설정
	if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(NewPage->Slot))
	{
		Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		Slot->SetOffsets(FMargin(0.f));
	}

	PageStack.Add(NewPage);

	NewPage->NativePreOpen(Param);
	NewPage->NativeOnOpened();

	UE_LOG(LogCardGameHUD, Log, TEXT("OpenPage: %s (Stack: %d)"), *PageClass->GetName(), PageStack.Num());
	return NewPage;
}

void ACardGameHUD::ClosePage()
{
	if (PageStack.Num() == 0) return;

	UPageWidget* TopPage = PageStack.Pop();
	if (TopPage)
	{
		TopPage->NativePreClose();
		TopPage->NativeOnClosed();
		TopPage->RemoveFromParent();

		UE_LOG(LogCardGameHUD, Log, TEXT("ClosePage: %s (Stack: %d)"), *TopPage->GetClass()->GetName(), PageStack.Num());
	}

	// 이전 페이지 복원
	if (PageStack.Num() > 0)
	{
		UPageWidget* PrevPage = PageStack.Last();
		if (PrevPage)
		{
			PrevPage->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ACardGameHUD::CloseAllPages()
{
	while (PageStack.Num() > 0)
	{
		UPageWidget* Page = PageStack.Pop();
		if (Page)
		{
			Page->NativePreClose();
			Page->NativeOnClosed();
			Page->RemoveFromParent();
		}
	}

	UE_LOG(LogCardGameHUD, Log, TEXT("CloseAllPages"));
}

UPageWidget* ACardGameHUD::GetTopPage() const
{
	return PageStack.Num() > 0 ? PageStack.Last() : nullptr;
}

// ────────────────────────────────────────────────────────────
// Popup
// ────────────────────────────────────────────────────────────

UPopupWidget* ACardGameHUD::OpenPopup(TSubclassOf<UPopupWidget> PopupClass, const FViewParam& Param)
{
	if (!PopupClass || !PopupLayer)
	{
		UE_LOG(LogCardGameHUD, Warning, TEXT("OpenPopup: PopupClass or PopupLayer is null"));
		return nullptr;
	}

	UPopupWidget* NewPopup = CreateWidget<UPopupWidget>(GetOwningPlayerController(), PopupClass);
	if (!NewPopup)
	{
		UE_LOG(LogCardGameHUD, Error, TEXT("OpenPopup: Failed to create widget %s"), *PopupClass->GetName());
		return nullptr;
	}

	NewPopup->SetOwningHUD(this);
	PopupLayer->AddChild(NewPopup);

	// CanvasPanel 슬롯을 전체 화면으로 설정
	if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(NewPopup->Slot))
	{
		Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		Slot->SetOffsets(FMargin(0.f));
	}

	ActivePopups.Add(NewPopup);

	NewPopup->NativePreOpen(Param);
	NewPopup->NativeOnOpened();

	UE_LOG(LogCardGameHUD, Log, TEXT("OpenPopup: %s (Active: %d)"), *PopupClass->GetName(), ActivePopups.Num());
	return NewPopup;
}

void ACardGameHUD::ClosePopup(UPopupWidget* Popup)
{
	if (!Popup) return;

	Popup->NativePreClose();
	Popup->NativeOnClosed();
	Popup->RemoveFromParent();
	ActivePopups.Remove(Popup);

	UE_LOG(LogCardGameHUD, Log, TEXT("ClosePopup: %s (Active: %d)"), *Popup->GetClass()->GetName(), ActivePopups.Num());
}

void ACardGameHUD::CloseAllPopups()
{
	for (int32 i = ActivePopups.Num() - 1; i >= 0; --i)
	{
		if (UPopupWidget* Popup = ActivePopups[i])
		{
			Popup->NativePreClose();
			Popup->NativeOnClosed();
			Popup->RemoveFromParent();
		}
	}
	ActivePopups.Empty();

	UE_LOG(LogCardGameHUD, Log, TEXT("CloseAllPopups"));
}

// ────────────────────────────────────────────────────────────
// Floating Text
// ────────────────────────────────────────────────────────────

void ACardGameHUD::ShowFloatingText(EFloatingTextType Type, const FString& Text, FVector WorldPos)
{
	if (FloatingTextPool.Num() == 0) return;

	UFloatingTextWidget* Widget = FloatingTextPool[FloatingTextNextIndex];
	FloatingTextNextIndex = (FloatingTextNextIndex + 1) % FloatingTextPoolSize;

	if (Widget)
	{
		Widget->Show(Type, Text, WorldPos);
	}
}

// ────────────────────────────────────────────────────────────
// RequestCloseView (ViewBase에서 호출)
// ────────────────────────────────────────────────────────────

void ACardGameHUD::RequestCloseView(UViewBase* View)
{
	if (!View) return;

	// 팝업인지 확인
	if (UPopupWidget* Popup = Cast<UPopupWidget>(View))
	{
		ClosePopup(Popup);
		return;
	}

	// 페이지인지 확인 — 스택 최상위만 닫기 가능
	if (UPageWidget* Page = Cast<UPageWidget>(View))
	{
		if (PageStack.Num() > 0 && PageStack.Last() == Page)
		{
			ClosePage();
		}
		else
		{
			UE_LOG(LogCardGameHUD, Warning, TEXT("RequestCloseView: Page %s is not top of stack"), *Page->GetClass()->GetName());
		}
	}
}

// ────────────────────────────────────────────────────────────
// Internal
// ────────────────────────────────────────────────────────────

void ACardGameHUD::InitRootWidget()
{
	if (!RootWidgetClass)
	{
		UE_LOG(LogCardGameHUD, Warning, TEXT("RootWidgetClass is not set. UI will not be created."));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	RootWidget = CreateWidget<UUserWidget>(PC, RootWidgetClass);
	if (!RootWidget) return;

	RootWidget->AddToViewport(0);

	// 루트 위젯 내의 PageLayer / PopupLayer (CanvasPanel) 참조 취득
	if (UWidgetTree* Tree = RootWidget->WidgetTree)
	{
		PageLayer = Cast<UPanelWidget>(Tree->FindWidget(FName("PageLayer")));
		PopupLayer = Cast<UPanelWidget>(Tree->FindWidget(FName("PopupLayer")));
	}

	if (!PageLayer)
	{
		UE_LOG(LogCardGameHUD, Error, TEXT("PageLayer not found in RootWidget. Ensure WBP has a panel named 'PageLayer'."));
	}
	if (!PopupLayer)
	{
		UE_LOG(LogCardGameHUD, Error, TEXT("PopupLayer not found in RootWidget. Ensure WBP has a panel named 'PopupLayer'."));
	}

	UE_LOG(LogCardGameHUD, Log, TEXT("RootWidget created: PageLayer=%s, PopupLayer=%s"),
		PageLayer ? TEXT("OK") : TEXT("MISSING"),
		PopupLayer ? TEXT("OK") : TEXT("MISSING"));
}

void ACardGameHUD::InitFloatingTextPool()
{
	if (!FloatingTextClass)
	{
		UE_LOG(LogCardGameHUD, Log, TEXT("FloatingTextClass is not set. Floating text disabled."));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	FloatingTextPool.Reserve(FloatingTextPoolSize);
	for (int32 i = 0; i < FloatingTextPoolSize; ++i)
	{
		UFloatingTextWidget* Widget = CreateWidget<UFloatingTextWidget>(PC, FloatingTextClass);
		if (Widget)
		{
			Widget->AddToViewport(100);  // 최상위 레이어
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			FloatingTextPool.Add(Widget);
		}
	}

	UE_LOG(LogCardGameHUD, Log, TEXT("FloatingText pool created: %d"), FloatingTextPool.Num());
}
