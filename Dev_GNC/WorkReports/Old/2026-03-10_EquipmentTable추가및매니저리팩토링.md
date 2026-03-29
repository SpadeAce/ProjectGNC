# 작업 보고서: EquipmentTable 추가 및 매니저 리팩토링

**날짜**: 2026-03-10
**작업자**: Claude

## 요청 내용

`EquipmentDataTable`을 프로젝트에 추가하고, `DeckManager`에 혼재되어 있던 보유 Pawn 관리 기능을 `PawnManager`로 분리하여 각 매니저의 역할을 명확히 압축해달라는 요청.

## 수행한 작업

- `EquipmentDataTable.cs` 생성 — `EquipmentData` bytes 로드 및 id 기반 조회 제공
- `DataManager.cs`에 `Equipment` 프로퍼티 및 `LoadAll()` 등록 추가
- `PawnManager.cs` 재작성 — `_pawnList`, `Pawns`, `AddPawn()` 추가, 미사용 `_dicPawn` 제거
- `DeckManager.cs`에서 보유 Pawn 관련 코드(`_ownPawnList`, `OwnPawns`, `AddPawn()`) 제거
- `DeckManager.InitTestData()` 내 Pawn 추가 호출을 `PawnManager.Instance.AddPawn()`으로 변경
- `DeckSetting_Pawn.cs`의 `OwnPawns` 참조를 `PawnManager.Instance.Pawns`로 변경
- `RecruitItem.cs`의 `AddPawn` 호출을 `PawnManager.Instance.AddPawn()`으로 변경
- `PawnManager.OwnPawns` 프로퍼티명을 `Pawns`로 수정 (PawnManager 컨텍스트에서 Own 접두사 불필요)

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/TableData/EquipmentDataTable.cs` | 추가 | EquipmentData bytes 로드 및 id 조회 클래스 |
| `Assets/Scripts/Logic/Manager/DataManager.cs` | 수정 | `Equipment` 프로퍼티 및 `LoadAll()` 등록 추가 |
| `Assets/Scripts/Logic/Manager/PawnManager.cs` | 수정 | `_pawnList`, `Pawns`, `AddPawn()` 추가; `_dicPawn` 제거 |
| `Assets/Scripts/Logic/Manager/DeckManager.cs` | 수정 | 보유 Pawn 관련 필드·메서드 제거, `InitTestData()` 수정 |
| `Assets/Scripts/UI/DeckSetting/DeckSetting_Pawn.cs` | 수정 | `OwnPawns` 참조를 `PawnManager.Instance.Pawns`로 변경 |
| `Assets/Scripts/UI/Recruit/RecruitItem.cs` | 수정 | `AddPawn` 호출을 `PawnManager.Instance.AddPawn()`으로 변경 |

## 주요 결정 사항

- **`_pawnList` 필드명 채택**: PawnManager 자체가 보유 Pawn 컨텍스트이므로 `_ownPawnList`의 `own` 접두사 불필요. 마찬가지로 프로퍼티도 `OwnPawns` → `Pawns`로 단순화.
- **`ResetPawnStats()`는 DeckManager 유지**: 편성된 Pawn(`DeckPawns`)의 스탯을 리셋하는 기능으로, 편성 관련 책임에 해당하여 DeckManager에 존속.
- **`DeckManager` 역할 압축**: 편성 슬롯 관리 + 카드/금화/핸드 관리로 한정. 보유 Pawn 목록 관리는 PawnManager 단독 책임.

## 참고 사항

- `EquipmentDataTable` 사용법: `DataManager.Instance.Equipment.Get(id)`
- 영입 후 보유 Pawn 접근: `PawnManager.Instance.Pawns`
- 편성된 Pawn 접근: `DeckManager.Instance.DeckPawns`
