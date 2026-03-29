# InitialPageClass를 GameMode로 이전

> **날짜**: 2026-03-27
> **작업 유형**: 리팩토링
> **관련 시스템**: UI 시스템, 씬 흐름

---

## 작업 요약

WBP_CardGameHUD를 모든 맵에서 공용으로 사용할 수 있도록, `InitialPageClass` 속성을 HUD에서 GameMode로 이전했다. 기존에는 HUD의 `EditDefaultsOnly` 속성이었기 때문에 맵별로 초기 페이지를 달리하려면 HUD BP를 복제해야 했다. 이제 맵별 GameMode BP에서 `InitialPageClass`만 설정하면 된다.

---

## 변경 파일 (3개)

### 1. CardGameMode.h
- `UPageWidget` 전방 선언 추가
- `InitialPageClass` (TSubclassOf\<UPageWidget\>) 속성 추가 (EditDefaultsOnly)

### 2. CardGameHUD.h
- `InitialPageClass` 속성 제거

### 3. CardGameHUD.cpp
- `CardGameMode.h` include 추가
- BeginPlay에서 `InitialPageClass` 직접 참조 → `GetWorld()->GetAuthGameMode()`를 통해 GameMode에서 가져오도록 변경

---

## 설계 근거

| 항목 | 변경 전 | 변경 후 |
|------|---------|---------|
| InitialPageClass 위치 | ACardGameHUD (EditDefaultsOnly) | ACardGameMode (EditDefaultsOnly) |
| WBP_CardGameHUD | 맵별로 자식 BP 필요 | 공용 1개로 충분 |
| 맵별 초기 페이지 설정 | HUD BP에서 설정 | GameMode BP에서 설정 |

---

## 에디터 설정 방법

1. 맵별 GameMode BP 생성 (예: `BP_TitleGameMode`, `BP_LobbyGameMode`, `BP_StageGameMode`)
2. 각 GameMode BP의 `InitialPageClass`에 해당 페이지 위젯 설정
   - Title → TitlePageWidget
   - Lobby → LobbyPageWidget
   - Stage → StagePageWidget
3. World Settings에서 맵마다 해당 GameMode BP 지정
4. WBP_CardGameHUD는 모든 맵에서 공용 사용
