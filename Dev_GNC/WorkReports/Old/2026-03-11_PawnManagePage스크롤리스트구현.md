# 작업 보고서: PawnManagePage 스크롤 리스트 구현

**날짜**: 2026-03-11
**작업자**: Claude

## 요청 내용

PawnManagePage의 두 스크롤 리스트를 구현한다. `_scrollPawnList`에는 보유 중인 Pawn 목록을 `PawnManageItem`으로, `_scrollEquipList`에는 보유 중인 장비 목록을 `PawnManageEquipItem`으로 표시한다.

## 수행한 작업

- `ItemManager`에 `Equips` public getter 추가하여 외부에서 장비 목록 접근 가능하게 함
- `PawnManageItem`에 `Spawn_Icon` Linker 필드 및 `SetData(DPawn)` 메서드 구현 (DeckPawnItem 패턴 적용)
- `PawnManageEquipItem`에 `SetData(DEquipment)` 최소 구현 (프리팹 미완성으로 데이터 저장만)
- `PawnManagePage`에 `PreOpen`, `OnOpened`, `RefreshPawnList`, `RefreshEquipList` 구현

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/ItemManager.cs` | 수정 | `IReadOnlyList<DEquipment> Equips` getter 추가 |
| `Assets/Scripts/UI/PawnManage/PawnManageItem.cs` | 수정 | `Spawn _spawnIcon` Linker 필드 + `SetData(DPawn)` 구현 |
| `Assets/Scripts/UI/PawnManage/PawnManageEquipItem.cs` | 수정 | `SetData(DEquipment)` 최소 구현 |
| `Assets/Scripts/UI/PawnManage/PawnManagePage.cs` | 수정 | 두 스크롤 리스트 채우기 전체 구현 |

## 주요 결정 사항

- **ShopPage/DeckSetting_Pawn 패턴 채택**: `PrefabLoader.Load<T>()` + `List<T> _activeItems` + 수동 `anchoredPosition` 지정 방식을 그대로 따름
- **PawnManageEquipItem 최소 구현**: 프리팹 UI 요소가 미완성이므로 Linker 연결 없이 데이터 저장(`_equip`)만 구현. 프리팹 완성 후 UI 연결 추가 필요
- **content sizeDelta 수동 설정**: ScrollRect가 올바르게 스크롤되도록 content 높이를 아이템 수 × itemHeight로 설정
- **아이템 높이 기본값**: PawnManageItem 310f, PawnManageEquipItem 120f (실제 프리팹 크기에 맞게 조정 필요)

## 참고 사항

- `PawnManageEquipItem`의 `_equip` 필드는 현재 미사용 힌트가 발생하나, 프리팹 완성 후 UI 표시 로직 추가 시 사용됨
- 아이템 높이(itemHeight) 상수는 실제 프리팹 RectTransform 크기에 맞게 수정해야 스크롤이 정확히 동작함
- `PawnManageEquipItem` 프리팹에 UI 요소 추가 후, Linker 필드와 SetData 내 표시 로직을 채워야 함
