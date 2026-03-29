# 작업 보고서: LobbyManager 분리 및 PawnIcon 구현

**날짜**: 2026-03-08
**작업자**: Claude

## 요청 내용

ShopPage/RecruitPage가 목록 데이터를 직접 보유하여 페이지 전환 시 목록이 초기화되는 문제를 근본적으로 해결하기 위해 `LobbyManager`를 신설하고 데이터를 이전 요청.
추가로 `PawnIcon.SetData()` 구현과 `DPawn`에 아이콘 경로 추가 요청.

## 수행한 작업

- `PawnIcon.SetData(DPawn)` 구현 — `CodeName`, `ClassType`, `_rawIcon.texture` 로드
- `DPawn`에 `IconPath` 프로퍼티 추가 — 두 생성자에서 `"Textures/Icon/Pawn/{Data.Name}"` 임시 경로로 초기화
- `ShopPage`/`RecruitPage` `OnOpened()` 가드 조건 개선 — `RefreshItems()` 호출 조건 분리 (중간 수정)
- `LobbyManager` 신규 생성 — ShopCards/ShopLevel 및 RecruitPawns 상태 관리, MonoSingleton으로 페이지 수명과 독립
- `ShopPage` 리팩토링 — `_shopCards`, `_shopLevel`, `GenerateCardList()` 제거, `LobbyManager` 참조로 교체
- `RecruitPage` 리팩토링 — `_recruitPawns`, `GenerateRecruitList()`, `RecruitSlotCount` 제거, `LobbyManager` 참조로 교체
- 로드맵(`99_로드맵.md`) 정리 — 완료 항목 제거, 향후 작업(영입 고도화, 자원/HUD, 상점 고도화 등) 신규 추가
- 기획서 갱신 — `06_UI_시스템.md`, `07_영입_시스템.md`, `09_상점_시스템.md`

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/LobbyManager.cs` | 추가 | 상점(ShopCards/Level) + 영입(RecruitPawns) 상태 관리 신규 생성 |
| `Assets/Scripts/UI/Shop/ShopPage.cs` | 수정 | 데이터 필드/메서드 제거, LobbyManager 참조로 전면 교체 |
| `Assets/Scripts/UI/Recruit/RecruitPage.cs` | 수정 | 데이터 필드/메서드 제거, LobbyManager 참조로 전면 교체 |
| `Assets/Scripts/UI/Icon/PawnIcon.cs` | 수정 | `SetData()` 구현 — CodeName, ClassType, 아이콘 텍스처 표시 |
| `Assets/Scripts/Data/DataObject/DPawn.cs` | 수정 | `IconPath` 프로퍼티 추가, 두 생성자에서 임시 경로 초기화 |
| `WorkReports/Design/99_로드맵.md` | 수정 | 완료 항목 제거, 신규 일감 추가 |
| `WorkReports/Design/06_UI_시스템.md` | 수정 | PawnIcon 구현 상태 및 ItemIcon 설명 분리 갱신 |
| `WorkReports/Design/07_영입_시스템.md` | 수정 | 구현 상태 갱신, LobbyManager 기반 구조 반영 |
| `WorkReports/Design/09_상점_시스템.md` | 수정 | 핵심 파일 LobbyManager 추가, 동작 흐름/진열 유지 정책 갱신 |

## 주요 결정 사항

- **LobbyManager 신설 (DeckManager 아님)**: DeckManager가 골드·덱 구성(카드/Pawn)에 집중하도록 역할을 분리. 상점/영입은 로비 전용 진행 상태이므로 별도 매니저가 적합.
- **DPawn.IconPath를 인스턴스 필드로 추가**: PawnData에 IconPath가 없고, 개체마다 다른 이미지를 가질 수 있으므로 DPawn 인스턴스에 경로 보관. 현재는 `PawnData.Name` 기반 컨벤션 경로(`Textures/Icon/Pawn/{Name}`)를 임시값으로 사용.

## 참고 사항

- `PawnIcon._rawIcon`에 로드되는 텍스처는 아직 리소스가 없음 — `Assets/Resources/Textures/Icon/Pawn/` 경로에 `PawnData.Name`과 일치하는 파일 추가 시 자동 적용.
- LobbyManager.cs 신규 생성 직후 IDE 언어 서버가 타입을 즉시 인식하지 못해 ShopPage/RecruitPage에 일시적 에러 표시 — Unity 재컴파일 후 정상 해소.
