# Variant 경로 제거 — CardGame 리네임

- **일시**: 2026-03-29
- **분류**: 리팩토링 (프로젝트 구조)
- **범위**: Source, Config, 기획서, 가이드 전반의 `Variant_CardGame` → `CardGame` 경로 변경

---

## 배경

프로젝트 초기에는 Variant_Strategy, Variant_TwinStick 등 여러 게임 타입이 공존하는 구조였기 때문에 `Variant_CardGame`이라는 접두사를 사용했다. 이후 다른 Variant를 모두 제거하여 CardGame이 베이스 프로젝트가 되었으므로, 경로에서 `Variant_` 접두사를 제거하고 `CardGame`으로 단순화했다.

---

## 변경 내용

### 1. 소스 폴더 이동

| 변경 전 | 변경 후 |
|---------|---------|
| `Source/Dev_GNC/Variant_CardGame/` | `Source/Dev_GNC/CardGame/` |
| `Source/Dev_GNCEditor/Variant_CardGame/` | `Source/Dev_GNCEditor/CardGame/` |

### 2. 소스 코드 및 설정 파일 수정 (9개 파일)

| 파일 | 수정 내용 |
|------|-----------|
| `Source/Dev_GNC/Dev_GNC.Build.cs` | PublicIncludePaths 6곳 |
| `Source/Dev_GNCEditor/Dev_GNCEditor.Build.cs` | PublicIncludePaths 1곳 |
| `Config/DefaultEngine.ini` | `/Game/` 에셋 참조 경로 3곳 |
| `Source/Dev_GNC/CardGame/UI/CardGameHUD.cpp` | `#include` 경로 1곳 |
| `Source/Dev_GNC/CardGame/Subsystem/GridSubsystem.cpp` | 에셋 참조 경로 1곳 |
| `Source/Dev_GNC/CardGame/Subsystem/DataSubsystem.cpp` | 에셋 참조 경로 11곳 |
| `Source/Dev_GNC/CardGame/Subsystem/TextSubsystem.cpp` | 에셋 참조 경로 1곳 |
| `Source/Dev_GNCEditor/CardGame/TileMapEditor/STileMapEditorWindow.cpp` | 에셋 참조 경로 2곳 |
| `CLAUDE.md` | 코딩 컨벤션 경로 2곳 |

### 3. 기획서 수정 (2개 파일)

| 파일 | 수정 개소 |
|------|-----------|
| `WorkReports/Design/00_프로젝트_개요.md` | 2곳 |
| `WorkReports/Design/04_타일_맵_시스템.md` | 2곳 |

### 4. 가이드 수정 (4개 파일)

| 파일 | 수정 개소 |
|------|-----------|
| `WorkReports/Guide/DataTable_가이드.md` | 2곳 |
| `WorkReports/Guide/TileMap_에디터_사용_가이드.md` | 1곳 |
| `WorkReports/Guide/UE5_에디터_설정_가이드.md` | 12곳 |
| `WorkReports/Guide/UE5_포팅_가이드.md` | 2곳 |

---

## 미수정 항목 (수동 처리 필요)

### Content 폴더

`Content/Variant_CardGame/` → `Content/CardGame/` 이동은 UE5 에디터에서 수동 처리 필요. 에디터의 콘텐츠 브라우저에서 폴더를 이동해야 내부 에셋 참조(리디렉터)가 정상 갱신된다.

### 작업 보고서

`WorkReports/` 루트의 날짜별 보고서(.md)에도 `Variant_CardGame` 참조가 남아있으나, 작업 이력 기록이므로 원본 그대로 보존했다.

---

## 최종 경로 구조

```
Source/Dev_GNC/CardGame/
├── Data/
├── Entity/
├── Grid/
├── Subsystem/
└── UI/

Source/Dev_GNCEditor/CardGame/
└── TileMapEditor/

Content/CardGame/          ← 에디터에서 수동 이동 필요
├── Blueprints/
├── DataTables/
├── Input/
├── Maps/
├── Materials/
├── Mesh/
└── Textures/
```
