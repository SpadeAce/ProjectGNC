# 작업 보고서: PawnStats 스탯 컴포넌트 구현

**날짜**: 2026-03-11
**작업자**: Claude

## 요청 내용

장비(`EquipmentData`)에 `statusType` / `statusValue` 스탯 필드가 추가됨에 따라 관련 기능을 구현한다. DPawn이 직접 스탯 합산 로직을 갖추면 클래스가 과도하게 커지므로, 스탯 관리용 서브 컴포넌트를 별도 파일로 분리한다. `_equips`가 변경될 경우 해당 리스트로 스탯을 자동 갱신한다.

## 수행한 작업

- `PawnStats` 클래스 신규 생성 (`Assets/Scripts/Data/DataObject/Components/` 폴더 신설)
- `PawnStats.Recalculate(PawnData, IReadOnlyList<DEquipment>)`에서 PawnData 기본값과 장비 `StatusType`/`StatusValue` 쌍을 합산하여 최종 기저 스탯 제공
- `DPawn.Stats` 프로퍼티 추가 및 생성자에서 `Stats.Recalculate()` 선행 호출
- `DPawn.Equip` / `Unequip` 수정 → 장착/해제 후 `RecalculateStats()` 자동 호출
- `DPawn.UpdateStatus()` 수정 → 기존 `Data.*` 직접 참조 대신 `Stats.*`에서 읽도록 변경
- `DPawn.MaxHandSize` 를 `public const` → `public int` 프로퍼티로 변경 (`BaseMaxHandSize + Stats.CardCap`)
- `05_캐릭터_시스템.md` 갱신 — 구현 상태, 핵심 파일, 스탯 합산 구조, 장비 스탯 테이블, 카드풀 섹션, 향후 작업 체크리스트

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Data/DataObject/Components/PawnStats.cs` | 추가 | Pawn 기저 스탯 합산 컴포넌트 신규 생성 |
| `Assets/Scripts/Data/DataObject/DPawn.cs` | 수정 | Stats 프로퍼티 추가, Equip/Unequip 갱신 트리거, UpdateStatus 위임, MaxHandSize 프로퍼티화 |
| `WorkReports/Design/05_캐릭터_시스템.md` | 수정 | PawnStats 구조 설명, StatusType 매핑 표, 체크리스트 갱신 |

## 주요 결정 사항

- **클래스명 `PawnStats`**: 장비뿐 아니라 레벨업, 특성 등 다른 소스로도 확장될 수 있으므로 `PawnEquipStats` 대신 범용 명칭 채택
- **경로 `DataObject/Components/`**: 서브 컴포넌트를 위한 별도 폴더 신설하여 DataObject 루트가 비대해지는 것을 방지
- **PawnData 기본값도 PawnStats에서 합산**: `UpdateStatus()`가 항상 `Stats.*`만 참조하도록 일원화하여, 향후 새로운 스탯 소스 추가 시 `Recalculate()` 내부만 수정하면 됨
- **`MaxHandSize` 프로퍼티화**: `StatCardCap` 장비 보너스를 반영하기 위해 `const`에서 인스턴스 프로퍼티로 변경

## 참고 사항

- `StatAccuracy`는 현재 DPawn에 대응 스탯이 없어 PawnStats에서 미처리 (예비 확장용으로 남겨둠)
- `Sight` / `Ammo`는 StatusType에 대응하는 장비 스탯이 없으므로 PawnData 값만 사용
- `ResetStats()`는 `_buffs.Clear()` 후 `UpdateStatus()`를 호출하므로, 장비 보너스는 Stats에 남아 버프만 제거되는 올바른 동작이 유지됨
