# 작업 보고서: ALLY 타겟 및 Radius 기능 구현

**날짜**: 2026-03-07
**작업자**: Claude

## 요청 내용

CardData에 추가된 `Radius` 필드와 `TargetType.Ally`를 실제 게임 로직에 연결해달라는 요청. ALLY는 아군 Pawn만 대상으로 하는 타입이며, Radius는 ENEMY/GROUND/ALLY 카드 사용 시 주변 동일 대상에게도 효과를 적용하는 AoE 기능이다. 추가로 카드 드래그 중 마우스 위치 기준으로 효과 범위를 미리보기로 표시하는 기능도 구현.

## 수행한 작업

- `SquareTile.cs`에 반경 미리보기용 `_goRadius` 직렬화 필드 및 `SetRadius(bool)` 메서드 추가, `Clear()`에 `SetRadius(false)` 연동
- `StageManager.cs`에 `_radiusPreviewTiles` HashSet 추가
- `StageManager.TryUseCard()`에 `TargetType.Ally` 분기 추가 (DPawn 대상 검증 후 비용 소모 및 카드 사용 처리)
- 기존 ENEMY/GROUND 분기의 단일 타겟 `ApplyCardEffects()` 호출을 `ApplyCardEffectsWithRadius()` 호출로 교체
- `ApplyCardEffectsWithRadius()` 신규 메서드 추가 — 중심 타일 기준 Radius 범위 내 대상 전체에 효과 적용
- `GetRadiusTargets()` 신규 메서드 추가 — 타입별 다이아몬드 범위 탐색 (Enemy→DMonster, Ally→DPawn, Ground→walkable 타일 전체)
- `ShowRadiusPreview()` / `ClearRadiusPreview()` / `IsInCardRange()` 메서드 추가
- `StagePage.StartCardDrag()`에 ALLY 타입을 카드 사거리 표시 조건에 추가
- `StagePage.OnCardDragging()`에 레이캐스트 기반 호버 타일 감지 및 `ShowRadiusPreview()` 호출 로직 추가 (사거리 밖이면 미리보기 클리어)
- `StagePage.OnCardDragEnd()`에 ALLY 타입을 타겟팅 분기에 추가하고 `ClearRadiusPreview()` 호출 추가
- `WorkReports/Design/03_카드_시스템.md` 기획서 갱신 (Radius 필드, Ally 타입, 반경 미리보기 흐름 추가)
- `WorkReports/Design/07_미구현_기능_로드맵.md` 갱신 (완료 항목 체크)

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Tile/SquareTile.cs` | 수정 | `_goRadius` 필드, `SetRadius(bool)` 메서드 추가 |
| `Assets/Scripts/Logic/Manager/StageManager.cs` | 수정 | ALLY 분기, `ApplyCardEffectsWithRadius`, `GetRadiusTargets`, 반경 미리보기 메서드 추가 |
| `Assets/Scripts/UI/Stage/StagePage.cs` | 수정 | ALLY 드래그 처리, `OnCardDragging` 반경 미리보기 호버 로직 추가 |
| `WorkReports/Design/03_카드_시스템.md` | 수정 | Radius/Ally/반경 미리보기 내용 반영 |
| `WorkReports/Design/07_미구현_기능_로드맵.md` | 수정 | 완료 항목 3개 체크 |

## 주요 결정 사항

- **Radius=0 단일 타겟도 `GetRadiusTargets()` 통과**: 루프 범위가 `dx=0, dy=0`만 순회하므로 기존 단일 타겟 동작과 동일. 분기 없이 통일된 코드 유지.
- **GROUND의 null actor 허용**: GROUND 타입은 빈 타일에도 효과가 발동해야 하므로 `GetRadiusTargets()`에서 actor가 null이어도 리스트에 추가. `ApplyCardEffects()`는 이미 null actor를 처리함.
- **반경 미리보기에 새 시각 요소 추가**: 기존 `_goTarget`(사거리)과 구분하기 위해 `_goRadius` 별도 오버레이 사용. SquareTile 프리팹에서 색상 차별화 가능.
- **호버 미리보기 매 프레임 레이캐스트**: `OnCardDragging`은 이미 매 프레임 호출되므로 별도 Update 없이 자연스럽게 동작.

## 참고 사항

- SquareTile 프리팹에 `_goRadius` 자식 GameObject를 추가하고 Inspector에서 연결해야 미리보기가 시각적으로 동작함 (사용자가 프리팹 작업 완료함).
- ALLY 카드는 시전자 본인도 DPawn이므로 자기 자신 타일을 대상으로 지정하면 효과 적용됨 (Self 타입과 차이: Self는 위로 드래그, Ally는 타일 지정).
- AoE 공격 애니메이션이 다수 대상에 대해 동시에 발생할 수 있음 — 향후 순차 재생이 필요하면 별도 코루틴 처리 검토 필요.
