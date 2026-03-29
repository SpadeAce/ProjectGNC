# 작업 보고서: StagePage 카드 로직 StageManager 이전

**날짜**: 2026-03-06
**작업자**: Claude

## 요청 내용

`StagePage`에 카드 사용 관련 게임 로직이 과도하게 집중되어 있어, UI 제어 책임만 남기고 카드 범위 계산, 타겟 유효성 검사, 카드 효과 적용 등의 기능적 로직을 `StageManager`로 이전해달라는 요청. UI 갱신이 필요한 경우 StageManager 이벤트를 구독하는 방식으로 처리.

## 수행한 작업

- `StageManager`에 카드 사용 범위 계산 메서드 `GetCardUseRange()` 추가
- `StageManager`에 카드 범위 타일 하이라이트 관리 메서드 `ShowCardRange()` / `ClearCardRange()` 추가 (전용 필드 `_cardRangeTiles` 포함)
- `StageManager`에 Self 타겟 카드 사용 메서드 `UseCardSelf()` 추가
- `StageManager`에 타겟 타일 카드 사용 시도 메서드 `TryUseCard()` 추가 — 범위 검사, 타겟 타입 검사, `DeckManager.UseCard()` 호출 포함
- `StageManager`에 카드 효과 적용 로직 `ApplyCardEffects()` (private) 추가 — StagePage.UseCard() 내부 로직 이전
- `StagePage`에서 게임 로직 메서드 일체 제거: `GetUseRange()`, `HighlightRange()`, `ClearHighlights()`, `UseCard()`
- `StagePage`의 `_highlightedTiles` 필드 제거
- `StagePage.OnCardDragBegin()` — `StageManager.ShowCardRange()` 호출로 교체
- `StagePage.OnCardDragEnd()` — `StageManager.ClearCardRange()`, `UseCardSelf()`, `TryUseCard()` 호출로 교체. Raycast 및 드래그 제스처 판정(CancelRadius, SelfConfirmDelta)은 UI 책임으로 유지

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/StageManager.cs` | 수정 | 카드 범위/사용/효과 API 추가 (`_cardRangeTiles`, `GetCardUseRange`, `ShowCardRange`, `ClearCardRange`, `UseCardSelf`, `TryUseCard`, `ApplyCardEffects`) |
| `Assets/Scripts/UI/Stage/StagePage.cs` | 수정 | 게임 로직 메서드 및 필드 제거, 드래그 핸들러를 StageManager 호출로 교체 |

## 주요 결정 사항

- **카드 범위 하이라이트를 StageManager로 이전**: `_highlightedTiles`를 `_cardRangeTiles`로 이름 변경하여 이동. 이동 범위 하이라이트(`_reachableTiles`)와 별도로 관리하여 두 상태가 독립적으로 작동.
- **Raycast와 드래그 제스처 판정은 StagePage에 유지**: 스크린 좌표 처리와 UI 제스처 기준(CancelRadius, SelfConfirmDelta)은 UI 레이어의 책임. StageManager는 타일 단위의 게임 로직만 담당.
- **UI 갱신에 신규 이벤트 불필요**: 카드 사용 → `DeckManager.onHandChanged` → `StagePage.SetCardList()`, 몬스터 사망 → `onBattleEnd` → `StagePage.OnBattleEnd()` 로 기존 이벤트 체계로 모두 커버 가능.
- **`TryUseCard()` 반환값(bool) 활용 보류**: 현재 실패 시 별도 피드백이 없으므로 반환값은 무시. 향후 UI 피드백(카드 흔들림 등) 추가 시 활용 가능하도록 bool 반환 유지.

## 참고 사항

- 이번 리팩토링으로 StagePage는 드래그 UI 상태 관리, 카드 아이콘 목록 갱신, 턴/전투 결과 이벤트 구독만 담당하는 순수 UI 클래스가 됨.
- 향후 카드 효과 종류가 늘어날 경우 `StageManager.ApplyCardEffects()`만 수정하면 되는 구조.
