# 작업 보고서: NamePresetTable 등록 및 add-table 커맨드 제작

**날짜**: 2026-03-06
**작업자**: Claude

## 요청 내용

새로 추가된 NamePresetDataTable을 프로젝트에 등록하는 작업을 요청했다.
이후 동일한 테이블 추가 작업이 반복될 것을 고려해 Claude 커맨드(`/add-table`)를 제작해달라는 추가 요청이 있었다.

## 수행한 작업

- `NamePresetTable.cs` 생성 — `Data/NamePresetData.bytes`를 로드, `Get(id)`로 조회
- `DataManager.cs` 수정 — `NamePreset` 프로퍼티 추가 및 `LoadAll()`에 등록
- `.claude/commands/add-table.md` 생성 — 테이블 추가 작업을 자동화하는 Claude 커맨드 제작

## 수정된 파일

| 파일 경로 | 변경 유형 | 변경 내용 요약 |
|-----------|-----------|----------------|
| `Assets/Scripts/Logic/Manager/TableData/NamePresetTable.cs` | 추가 | NamePresetDataTable 로드 및 Get(id) 조회 클래스 |
| `Assets/Scripts/Logic/Manager/DataManager.cs` | 수정 | NamePreset 프로퍼티 추가, LoadAll()에 로드 구문 추가 |
| `.claude/commands/add-table.md` | 추가 | /add-table 커맨드 정의 파일 |

## 주요 결정 사항

- **Claude 커맨드 방식 채택**: 쉘 스크립트 대신 Claude 커맨드(`.claude/commands/add-table.md`)를 선택. 파일 생성 외에 GameData.cs 클래스 존재 여부 확인, bytes 파일 존재 여부 확인, Id 필드 타입 판단 등 컨텍스트 기반 판단이 필요한 단계가 있어 AI가 수행하는 것이 적합하다고 판단.

## 참고 사항

**`/add-table` 커맨드 사용법:**
```
/add-table NamePreset
```
- 사전 검증: GameData.cs 클래스 존재 여부, bytes 파일 존재 여부, 중복 등록 여부 확인
- `XxxTable.cs` 자동 생성 + `DataManager.cs` 자동 수정
- Id 타입이 int가 아닌 경우 자동으로 타입 조정

**등록된 테이블 조회 방법:**
```csharp
DataManager.Instance.NamePreset.Get(id); // NamePresetData 반환
```
