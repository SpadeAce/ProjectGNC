# PausePopup Abstract 에러 수정

- **일시**: 2026-04-09
- **분류**: 버그 수정 (UI 시스템)
- **범위**: StagePageWidget.h, StagePageWidget.cpp

---

## 배경

전투 중 일시정지 버튼을 누르면 다음 에러가 발생하여 PausePopup이 열리지 않았다.

```
PIE: Error: 추상(Abstract), 폐기(Deprecated), 대체(Replaced) 클래스는 유저 위젯 구성시 사용할 수 없습니다.
LogCardGameHUD: Error: OpenPopup: Failed to create widget PausePopupWidget
```

## 원인

`UPausePopupWidget`이 `UCLASS(Abstract, BlueprintType)`로 선언되어 있어 `CreateWidget`이 C++ 클래스를 직접 인스턴스화할 수 없다. `StagePageWidget`에서 `HUD->OpenPopup<UPausePopupWidget>()`으로 템플릿 호출하면 `UPausePopupWidget::StaticClass()`가 그대로 전달되어 Abstract 클래스 생성을 시도하게 된다.

BP 서브클래스(`WBP_PausePopup`)는 이미 존재했지만, 코드에서 참조하지 않고 있었다.

`BattleResultPopupWidget`도 동일한 패턴으로 잠재적 동일 버그가 있었다.

## 수정 내용

### StagePageWidget.h
- `TSubclassOf<UPausePopupWidget> PausePopupClass` 프로퍼티 추가
- `TSubclassOf<UBattleResultPopupWidget> BattleResultPopupClass` 프로퍼티 추가
- `UPausePopupWidget`, `UBattleResultPopupWidget` forward declaration 추가

### StagePageWidget.cpp
- `HUD->OpenPopup<UPausePopupWidget>()` → `HUD->OpenPopup(PausePopupClass)`
- `HUD->OpenPopup<UBattleResultPopupWidget>(Param)` → `HUD->OpenPopup(BattleResultPopupClass, Param)`

## 에디터 후속 작업

빌드 후 `WBP_StagePage` Blueprint에서:
- **PausePopupClass** → `WBP_PausePopup` 지정
- **BattleResultPopupClass** → `WBP_BattleResultPopup` 지정

## 비고

다른 위젯(LobbyPageWidget 등)은 이미 `TSubclassOf` 프로퍼티로 BP 클래스를 지정하는 패턴을 사용하고 있어, 이번 수정으로 프로젝트 전체의 위젯 생성 패턴이 통일되었다.
