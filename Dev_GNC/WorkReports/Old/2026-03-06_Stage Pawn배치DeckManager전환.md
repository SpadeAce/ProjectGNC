# 작업 보고서: Stage Pawn 배치 DeckManager 전환

**날짜**: 2026-03-06
**작업자**: Claude

## 요청 내용

스테이지에 배치할 Pawn을 기존 TileMapPreset의 `spawnId` 기반에서 `DeckManager`가 보유한 Pawn 목록 기반으로 전환해달라는 요청.
스폰 포인트는 배치 위치만 정의하고 실제 Pawn은 덱에서 가져오도록 변경하며, 스테이지별 최대 배치 Pawn 수 제한 기능도 추가.

## 수행한 작업

- `TileMapPreset`에 `maxPawnCount` 필드 추가 (스테이지별 배치 Pawn 상한 설정)
- `DeckManager`에 `DeckPawns` 공개 프로퍼티 추가 (`IReadOnlyList<DPawn>`)
- `StageManager.LoadStageFromPreset()` Player Spawn 로직을 DeckManager 기반으로 교체
- 배치 수 결정 공식 구현: `min(maxPawnCount, 스폰포인트 수, 덱 Pawn 수)`
- Pawn 배치 시스템 기획서 작성 (`WorkReports/PawnDeployment기획서.md`)

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Data/TileMapPreset.cs` | 수정 | `maxPawnCount` 필드 추가 |
| `Assets/Scripts/Logic/Manager/DeckManager.cs` | 수정 | `DeckPawns` 프로퍼티 추가 |
| `Assets/Scripts/Logic/Manager/StageManager.cs` | 수정 | Player Spawn 로직을 DeckManager 기반으로 교체 |
| `WorkReports/PawnDeployment기획서.md` | 추가 | Pawn 배치 시스템 기획서 신규 작성 |

## 주요 결정 사항

- **Player 스폰 포인트의 `spawnId` 무시**: 이제 배치 위치 마킹 용도만 담당. Enemy 스폰 포인트는 기존대로 `spawnId`(MonsterId)를 사용.
- **`maxPawnCount = 0` → 제한 없음**: 0일 때 스폰 포인트 수를 상한으로 사용해 기존 동작과 호환성 유지.
- **`IReadOnlyList` 노출**: DeckManager 내부 리스트를 외부에서 수정할 수 없도록 읽기 전용 인터페이스로 공개.

## 참고 사항

**후속 작업 예정 (기획서에 기록됨):**
- Pre-battle Deployment 시퀀스: 스테이지 진입 후 플레이어가 직접 Pawn을 배치 포인트에 선택 배치하는 UI/흐름 구현
- 현재는 자동 배치(DeckPawns 순서대로) → 추후 플레이어 선택 배치로 교체 예정
