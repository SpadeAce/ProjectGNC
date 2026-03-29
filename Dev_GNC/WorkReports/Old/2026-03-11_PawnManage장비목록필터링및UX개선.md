# 작업 보고서: PawnManage 장비 목록 필터링 및 UX 개선

**날짜**: 2026-03-11
**작업자**: Claude

## 요청 내용

PawnManagePage에서 세 가지 UX 개선을 수행한다.
① 드래그 중 스크롤 마스크로 인해 오브젝트가 클리핑되는 문제 수정,
② 캐릭터 미선택 상태에서 장비 슬롯 영역을 숨기는 기능,
③ 이미 장착 중인 장비는 장비 목록에 표시하지 않는 필터링 기능.

## 수행한 작업

- `PawnManageEquipItem`에 드래그 시작 시 Canvas 루트로 reparent하고 종료 시 원래 계층으로 복귀하는 로직 추가 (스크롤 마스크 클리핑 해결)
- `PawnManagePage`에 `_goEquipSlot` 참조 추가, 페이지 열릴 때 비활성화하고 Pawn 선택 시 활성화하도록 수정
- `DEquipment`에 `EquippedPawnId`, `IsEquipped`, `SetEquippedPawn`, `ClearEquippedPawn` 추가
- `PawnManagePage.RefreshEquipList`에서 `IsEquipped == false` 장비만 표시하도록 필터링
- `EquipToSlot`에서 장착 시 `SetEquippedPawn`, 기존 슬롯 교체 시 `ClearEquippedPawn` 호출 + `RefreshEquipList` 호출
- `OnSlotRightClick`에서 해제 시 `ClearEquippedPawn` + `RefreshEquipList` 호출
- `05_캐릭터_시스템.md` 기획서에 장착 상태 추적 섹션 추가

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/UI/PawnManage/PawnManageEquipItem.cs` | 수정 | 드래그 시 `_canvas.transform`으로 reparent, 종료 시 원래 부모 복귀 |
| `Assets/Scripts/UI/PawnManage/PawnManagePage.cs` | 수정 | `_goEquipSlot` 선택 여부에 따라 활성화, 장비 목록 필터링, 장착/해제 시 EquippedPawnId 갱신 |
| `Assets/Scripts/Data/DataObject/DEquipment.cs` | 수정 | `EquippedPawnId`, `IsEquipped`, `SetEquippedPawn`, `ClearEquippedPawn` 추가 |
| `WorkReports/Design/05_캐릭터_시스템.md` | 수정 | 장착 상태 추적 섹션 추가 |

## 주요 결정 사항

- **Canvas 루트로 reparent**: 드래그 중 스크롤 뷰포트 마스크 바깥으로 이탈하는 가장 단순한 방법. `worldPositionStays = true`로 reparent해 화면상 위치 유지. `OnPointerDown` 시점에 `GetComponentInParent<Canvas>()`로 캐시 (reparent 전이라 올바른 Canvas를 찾음)
- **InstanceId 기반 장착 추적**: `DObject.InstanceId`(long)를 활용해 DEquipment가 어느 Pawn에 장착됐는지 자체적으로 추적. UI 레이어와 데이터 레이어 모두에서 장착 여부를 확인 가능
- **장착/해제 직후 RefreshEquipList 호출**: 목록을 즉시 갱신해 장착된 아이템은 사라지고 해제된 아이템은 복원되도록 함

## 참고 사항

- 다른 Pawn의 슬롯을 열어봤을 때 해당 Pawn의 장착 장비는 슬롯에 표시되지만 목록에는 나타나지 않는 것이 의도된 동작
- 장비 목록은 `IsEquipped == false`만 표시하므로 여러 Pawn에 동일 장비를 중복 장착하는 것이 자연스럽게 방지됨
