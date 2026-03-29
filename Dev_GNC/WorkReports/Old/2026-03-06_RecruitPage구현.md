# 작업 보고서: RecruitPage 구현

**날짜**: 2026-03-06
**작업자**: Claude

## 요청 내용

Pawn을 영입하는 RecruitPage를 구현해달라는 요청.
3개의 랜덤 후보를 표시하고, 개별 영입 시 DeckManager에 추가되며, 후보 목록은 Stage 복귀나 Reroll 시에만 갱신된다.
현재 재화 기능이 없으므로 무료로 동작.

## 수행한 작업

- `PawnTable`에 `GetAll()` 메서드 추가 (전체 PawnData 목록 반환)
- `DeckManager`에 `AddPawn(DPawn)` 메서드 추가
- `RecruitItem.SetData()` 완성 (CodeName, ClassType, Price 표시)
- `RecruitItem.OnClickRecruit()` 구현 + `OnRecruited` 이벤트 추가
- `RecruitPage` 전체 로직 구현 (랜덤 후보 생성, 아이템 동적 생성, 영입 처리, Reroll)
- 영입 시스템 기획서 작성 (`WorkReports/Design/09_영입_시스템.md`)

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/TableData/PawnTable.cs` | 수정 | `GetAll()` 메서드 추가 |
| `Assets/Scripts/Logic/Manager/DeckManager.cs` | 수정 | `AddPawn(DPawn)` 메서드 추가 |
| `Assets/Scripts/UI/Recruit/RecruitItem.cs` | 수정 | `SetData()` 완성, `OnClickRecruit()` 구현, `OnRecruited` 이벤트 추가 |
| `Assets/Scripts/UI/Recruit/RecruitPage.cs` | 수정 | 전체 로직 구현 |
| `WorkReports/Design/09_영입_시스템.md` | 추가 | 영입 시스템 기획서 신규 작성 |

## 주요 결정 사항

- **후보 목록 유지 방식**: `_recruitPawns`를 RecruitPage 필드에 보관. 페이지를 닫고 재오픈해도 유지되며 Reroll 또는 `Refresh()` 호출 시에만 교체.
- **event 시그니처**: `Action<RecruitItem, DPawn>` — 영입된 아이템과 Pawn 양쪽을 전달해 RecruitPage에서 리스트 정리를 깔끔하게 처리.
- **Stage 복귀 갱신**: `Refresh()` 공개 메서드로 외부 호출 인터페이스를 마련해 둠. 연결은 추후 구현.

## 참고 사항

- 재화 시스템 미구현으로 Price는 "FREE" 하드코딩, 실제 차감 로직 없음.
- Stage 복귀 시 `RecruitPage.Refresh()` 호출 연결이 아직 없음 (StageScript 또는 씬 전환 흐름에서 추가 필요).
- `_textClass.text = pawn.Data.ClassType.ToString()` — 영문 enum 그대로 표시, 추후 한글화 또는 아이콘 처리 예정.
