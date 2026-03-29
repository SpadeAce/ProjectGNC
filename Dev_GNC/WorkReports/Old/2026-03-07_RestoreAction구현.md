# 작업 보고서: RestoreAction 구현

**날짜**: 2026-03-07
**작업자**: Claude

## 요청 내용

카드 효과 타입 `RestoreAction`을 구현해달라는 요청. 카드를 사용하면 대상 Pawn의 ActingPower를 EffectValue만큼 증가시키는 기능이며, 카드를 통한 증가는 최대치 초과를 허용하되 턴이 넘어갈 경우 초과분은 소멸시키는 방식으로 구현.

## 수행한 작업

- `DPawn.cs`에 `AddActingPower(int amount)` 메서드 추가 — cap 없이 ActingPower를 amount만큼 증가, `onStatsChanged` 이벤트 발생
- `StageManager.ApplyCardEffects()`의 switch에 `CardEffectType.RestoreAction` case 추가 — targetData가 DPawn이면 `rp.AddActingPower(value)` 호출
- `WorkReports/Design/03_카드_시스템.md` 기획서 갱신 — RestoreAction 구현 완료로 업데이트
- `WorkReports/Design/99_로드맵.md` 로드맵 갱신 — RestoreAction 항목 완료 처리

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Data/DataObject/DPawn.cs` | 수정 | `AddActingPower(int amount)` 메서드 추가 |
| `Assets/Scripts/Logic/Manager/StageManager.cs` | 수정 | `ApplyCardEffects()`에 `RestoreAction` case 추가 |
| `WorkReports/Design/03_카드_시스템.md` | 수정 | RestoreAction 구현 완료 표기 |
| `WorkReports/Design/99_로드맵.md` | 수정 | RestoreAction 완료 체크 |

## 주요 결정 사항

- **최대치 초과 허용**: 카드를 통한 ActingPower 증가는 `Data.ActingPower` 상한 없이 허용. `Mathf.Min` cap을 적용하지 않고 `ActingPower += amount`로 단순 증가.
- **초과분 소멸 처리는 기존 코드 활용**: `TurnManager.StartPawnTurn()`이 이미 `pawn.RestoreActingPower()` → `ActingPower = Data.ActingPower`를 호출하므로 별도 초과분 처리 코드 불필요.
- **기존 패턴 동일 적용**: Heal(`p.Heal(value)`)과 동일한 구조로 targetData가 DPawn이면 적용, Monster에는 적용하지 않음.

## 참고 사항

- 기존 `RestoreActingPower()`는 파라미터 없이 최대값으로 완전 회복하는 용도로 유지. `AddActingPower()`는 부분 회복/초과 회복 전용으로 별도 메서드로 분리.
- `onStatsChanged` 이벤트 발생으로 카드 disabled 상태(ActingPower 부족 시 카드 비활성화)가 자동 갱신됨.
- Self 타입 카드와 Ally 타입 카드 모두 targetActor가 DPawn이므로 동일 분기로 처리됨.
