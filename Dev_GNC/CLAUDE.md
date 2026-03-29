# Dev_GNC 프로젝트 지시사항

## 프로젝트 개요

Unity C# 턴제 카드 게임(CardGame)을 UE5 C++로 포팅하는 프로젝트.
원본 프로젝트: `c:\UnityProjects\CardGame`

## 작업 보고서 자동 작성 규칙

비자명한 작업(코드 수정, 기능 구현, 버그 수정, 설계 변경, 리팩토링 등)을 완료한 후에는
반드시 사용자에게 다음과 같이 질문한다:

> "보고서를 작성할까요? (WorkReports/ 폴더에 저장됩니다)"

사용자가 동의하면 `/work-report` 명령을 실행한다.

**트리거 조건 (다음 작업 완료 후 질문):**
- 코드 파일 1개 이상 수정
- 새 기능 구현
- 버그 수정
- 아키텍처 또는 설계 결정
- 리팩토링

**비트리거 조건 (다음의 경우 질문하지 않음):**
- 단순 질문/답변
- 코드 설명/분석만 한 경우
- 파일을 읽기만 한 경우

## UE5 코딩 컨벤션

- C++ 주도 개발, Blueprint는 UI 레이아웃/애니메이션에만 사용
- UPROPERTY, UFUNCTION 매크로 반드시 사용
- 헤더 파일에 GENERATED_BODY() 포함
- CardGame 코드는 `Source/Dev_GNC/CardGame/` 하위에 배치
- Content 에셋은 `Content/CardGame/` 하위에 배치
