# 작업 보고서: CardIcon BattleCard 분리

**날짜**: 2026-03-07
**작업자**: Claude

## 요청 내용

기존 `CardIcon`이 전투 전용 드래그 기능을 과하게 포함하고 있어 상점/로비 등 비전투 화면에서 재사용하기 어려웠다. 전투 기능을 `BattleCard`로 분리하고, `CardIcon`은 순수 표시 전용으로 단순화해달라는 요청. `BattleCard`는 `CardIcon`을 상속하지 않고 `MonoBehaviourEx`를 직접 상속하여 전투 표시 구조를 향후 독립적으로 변경할 수 있도록 완전히 분리.

## 수행한 작업

- `CardIcon.cs`에서 드래그 인터페이스(`IPointerDownHandler`, `IDragHandler`, `IPointerUpHandler`), `_imageDimmed`, `_isDisabled`, drag 이벤트, `SetDisabled()` 제거 — 표시 전용으로 단순화
- `BattleCard.cs` 플레이스홀더 교체 — `MonoBehaviourEx` 직접 상속, 모든 UI 필드(`_textCost`, `_textName`, `_textDesc`, `_rawIcon`, `_imageDimmed`) 독립 정의, `SetData()`, `SetDisabled()`, drag 이벤트 구현
- `StagePage.cs`의 `List<CardIcon>` → `List<BattleCard>` 교체, 이벤트 시그니처 `Action<CardIcon, ...>` → `Action<BattleCard, ...>` 교체, 프리팹 경로 `"Prefabs/UI/Icon/CardIcon"` → `"Prefabs/UI/HUD/BattleCard"` 변경
- `WorkReports/Design/06_UI_시스템.md` 갱신 — CardIcon과 BattleCard 분리 반영
- `WorkReports/Design/03_카드_시스템.md` 갱신 — BattleCard 참조로 업데이트

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/UI/Icon/CardIcon.cs` | 수정 | 드래그 로직 전부 제거, 표시 전용으로 단순화 |
| `Assets/Scripts/UI/HUD/BattleCard.cs` | 수정 | 플레이스홀더 → MonoBehaviourEx 상속, 전투 드래그 기능 구현 |
| `Assets/Scripts/UI/Stage/StagePage.cs` | 수정 | List<BattleCard>, 이벤트 시그니처, 프리팹 경로 교체 |
| `WorkReports/Design/06_UI_시스템.md` | 수정 | BattleCard 독립 항목 추가, StagePage 설명 갱신 |
| `WorkReports/Design/03_카드_시스템.md` | 수정 | 핵심 파일 표, 사용 흐름, 사용불가 처리 섹션 BattleCard로 갱신 |

## 주요 결정 사항

- **BattleCard가 CardIcon을 상속하지 않음**: 향후 전투 화면 표시를 단순화할 때 CardIcon과 독립적으로 변경할 수 있도록 `MonoBehaviourEx` 직접 상속 선택. 현재는 동일한 UI 필드를 가지나 이후 별도 방향으로 변경 가능.
- **BattleCard 위치**: `Assets/Scripts/UI/HUD/` — 사용자가 미리 생성해둔 파일과 프리팹(`Prefabs/UI/HUD/BattleCard`) 활용.

## 참고 사항

- `BattleCard` 프리팹(`Resources/Prefabs/UI/HUD/BattleCard.prefab`)은 이미 존재하므로 Unity 에디터에서 컴포넌트 연결만 확인하면 됨.
- `CardIcon` 프리팹은 표시 전용으로 유지 — `Image_Dimmed` 자식 오브젝트는 제거해도 무방.
