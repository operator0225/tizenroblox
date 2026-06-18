# KQ83SF95AEXKR 성능 분석 보고서
## Samsung OLED SF95 83인치 (2025) — TizenRoblox 프로젝트용

---

## 1. 모델 식별

| 항목 | 내용 |
|------|------|
| 모델명 | KQ83SF95AEXKR |
| 시리즈 | Samsung OLED SF95 (S95F) |
| 화면 크기 | 83인치 (209cm) |
| 출시 연도 | 2025 |
| 판매 지역 | 대한민국 (KR) |
| 패널 타입 | 4세대 QD-OLED (Samsung 5-스택 구조) |

---

## 2. 하드웨어 분석

### 2.1 프로세서 (SoC)

| 항목 | 사양 |
|------|------|
| 프로세서명 | NQ4 AI Gen3 (Neural Quantum 4K 3세대) |
| CPU | ARM Cortex 계열 (정확한 코어 비공개) |
| GPU | 이전 세대(Gen2) 대비 **4.2배** 향상 |
| NPU | 이전 세대 대비 **2.0배** 향상 |
| CPU 속도 | 이전 세대 대비 **67%** 향상 |
| AI 신경망 | 128개 (Gen2의 6배) |

> **비고**: S85F는 NQ4 AI Gen2, S95F는 Gen3 탑재. Gen3는 삼성 2025 OLED 최상위 프로세서.

### 2.2 메모리 / 저장공간

| 항목 | 사양 |
|------|------|
| RAM | 4–8GB (프리미엄 OLED 기준, 정확한 수치 미공개) |
| 내부 스토리지 | 16–32GB |
| 앱 가용 공간 | 약 8–16GB (시스템 점유 후) |

### 2.3 디스플레이

| 항목 | 사양 |
|------|------|
| 해상도 | 4K UHD (3840 × 2160) |
| 패널 | QD-OLED (Quantum Dot + OLED) |
| 최대 주사율 | 165Hz (Motion Xcelerator 165Hz) |
| 최대 밝기 | ~2000–4000 nits (HDR 피크) |
| 반사 방지 | OLED Glare Free 2.0 |
| HDR | OLED HDR Pro, Dolby Vision, HDR10+ |
| 색 영역 | DCI-P3 100% |

### 2.4 게이밍 입력 성능

| 항목 | 사양 |
|------|------|
| 입력 지연 (Input Lag) | **4.2ms** (4K@165Hz 기준) |
| 가변 주사율 | VRR, AMD FreeSync Premium Pro |
| ALLM | 지원 (Auto Low Latency Mode) |
| HDMI | 4포트 × HDMI 2.1a (4K@165Hz) |

### 2.5 연결성

| 항목 | 사양 |
|------|------|
| Wi-Fi | Wi-Fi 5 (802.11ac, 듀얼밴드) |
| Bluetooth | 5.3 |
| USB | USB-A 2포트 + USB-C 1포트 |
| 유선 LAN | 1Gbps Ethernet |
| 광출력 | 1포트 |

### 2.6 오디오

| 항목 | 사양 |
|------|------|
| 채널 구성 | 4.2.2 채널 |
| 총 출력 | 70W |
| 기술 | Dolby Atmos, Object Tracking Sound+, Q-Symphony |

---

## 3. 소프트웨어 분석

### 3.1 운영체제 (OS)

| 항목 | 내용 |
|------|------|
| OS | **Tizen 9.0** (One UI Tizen) |
| 커널 | Linux 기반 |
| 배포 방식 | OTA 업데이트 |
| 앱 포맷 | **TPK** (Tizen Package, `.tpk`) |
| APK 지원 | 미지원 (Android 앱 직접 설치 불가) |

### 3.2 Tizen 9.0 주요 특성

- **WebAssembly (WASM)** 런타임 지원
- **WebGL / WebGL2** 지원 (Web 앱 기반 그래픽)
- **NaCl(Native Client)** 미지원 → C/C++ 코드는 WASM으로 포팅 필요
- **OpenGL ES 2.0 / 3.0** 지원 (네이티브 앱 기준)
- **Vulkan** 일부 지원 (2025 프리미엄 모델)
- **JavaScript V8 엔진** 내장 (Web 런타임)
- 실시간 자막, 새 탐색 포맷 등 신규 API 추가

### 3.3 앱 설치 경로

| 방법 | 설명 |
|------|------|
| Samsung App Store | 공식 앱만 설치 가능, Roblox 미등록 |
| 개발자 모드 | Developer Mode 활성화 후 TPK 사이드로딩 |
| Tizen Studio | PC에서 빌드 후 Wi-Fi/USB로 배포 |

**개발자 모드 활성화 경로**:  
`설정 → 지원 → 정보 → IP Address 입력 후 개발자 모드 ON`

### 3.4 인증서 요구사항 (2025년 이후)

- 2025년 9월 이후 Samsung 인증서 정책 강화
- 배포용 TPK는 Samsung 발급 인증서로 서명 필요
- 개발/테스트용: Tizen Studio에서 생성한 임시 인증서 사용 가능

---

## 4. TizenRoblox 프로젝트 관점 성능 평가

### 4.1 하드웨어 충분성

| 요소 | 평가 | 비고 |
|------|------|------|
| CPU 처리 능력 | ✅ 충분 | Gen3, 67% 향상 |
| GPU 렌더링 | ✅ 우수 | 4.2배 향상, 165Hz 지원 |
| RAM | ✅ 양호 | 4–8GB, 게임 실행에 적합 |
| 입력 지연 | ✅ 우수 | 4.2ms (게임용 최적) |
| 저장 공간 | ⚠️ 주의 | 가용 8–16GB, Roblox 크기 고려 필요 |
| 해상도 부담 | ⚠️ 주의 | 4K 렌더링 시 GPU 부하 큼, 동적 해상도 권장 |

### 4.2 소프트웨어 호환성

| 요소 | 평가 | 비고 |
|------|------|------|
| Tizen 9.0 대응 | ✅ 최신 | 2025 최신 OS |
| TPK 패키징 | 필수 | Roblox를 TPK로 변환 필요 |
| OpenGL ES | ✅ 지원 | Roblox 렌더러 호환 가능 |
| Vulkan | 🔶 부분 | 안정성 검증 필요 |
| 인증서 서명 | 필수 | Samsung 인증서 필요 |
| 입력 장치 | ⚠️ 주의 | 리모컨/게임패드 매핑 별도 구현 필요 |

### 4.3 핵심 도전 과제

1. **렌더러 포팅**: Roblox의 Vulkan/D3D 렌더러를 OpenGL ES 3.0으로 전환 또는 Angle 레이어 적용
2. **4K 해상도 최적화**: 기본 렌더링 해상도를 1080p/1440p로 설정 후 TV의 AI 업스케일링 활용
3. **입력 시스템**: 삼성 스마트 리모컨 + 외부 게임패드(USB/BT) 지원 구현
4. **메모리 관리**: 8GB 이하 환경에서 Roblox 메모리 사용량 최적화
5. **TPK 인증서**: 배포 전 Samsung 공식 인증서 취득 필요

---

## 5. 성능 최적화 권장사항

### 렌더링
```
- 기본 해상도: 1920×1080 또는 2560×1440
- TV 내장 AI 업스케일 활용 (4K AI Upscaling Pro)
- VSync 대신 VRR 활용으로 165Hz에서 부드러운 프레임 유지
- LOD(Level of Detail) 거리 축소 적용
```

### 메모리
```
- 텍스처 스트리밍 활성화
- 최대 메모리 사용량: 3GB 이하 목표
- 저해상도 텍스처 옵션 제공
```

### 입력
```
- Samsung Smart Remote: D-pad → 캐릭터 이동
- 외부 게임패드: USB HID 또는 Bluetooth HID 프로파일
- 터치패드 지원 (삼성 리모컨 내장)
```

### 네트워크
```
- 유선 Ethernet 우선 사용 권장
- Wi-Fi 5 GHz 대역 사용 시 낮은 지연
- Roblox 서버 지역: 한국 데이터센터(ap-northeast) 우선
```

---

## 6. 결론

**KQ83SF95AEXKR은 TizenRoblox 구현에 있어 하드웨어적으로 충분한 사양을 갖추고 있습니다.**

- NQ4 AI Gen3의 4.2배 향상된 GPU와 4.2ms 초저지연으로 게이밍 성능 우수
- 165Hz QD-OLED는 부드러운 게임 경험 제공
- 소프트웨어 측면에서 Tizen 9.0은 OpenGL ES 3.0 기반 네이티브 앱 실행 지원

주요 과제는 하드웨어 성능 부족이 아닌 **소프트웨어 포팅 복잡도** (렌더러 전환, 인증서, 입력 시스템)이며, 이를 단계적으로 해결하면 고품질 Roblox 경험을 TV에서 구현할 수 있습니다.

---

## 참고 자료

- [Samsung KQ83SF95AEXKR 공식 페이지](https://www.samsung.com/sec/tvs/oled-83sf95-d2c/KQ83SF95AEXKR/)
- [Samsung S95F 사양 (displayspecifications.com)](https://www.displayspecifications.com/en/news/5941dae)
- [Samsung Developer — Tizen 일반 사양](https://developer.samsung.com/smarttv/develop/specifications/general-specifications.html)
- [Samsung Developer — TV 모델 그룹](https://developer.samsung.com/smarttv/develop/specifications/tv-model-groups.html)
- [S95F OLED 리뷰 (RTINGS.com)](https://www.rtings.com/tv/reviews/samsung/s95f-oled)
- [Samsung 2025 TV 라인업 (AVForums)](https://www.avforums.com/news/samsung-2025-tv-line-up-oled-neo-qled-and-more-all-you-need-to-know.22579/)
- [Tizen OS (Wikipedia)](https://en.wikipedia.org/wiki/Tizen)
- [Tizen 9.0 문서](https://docs.tizen.org/platform/what-is-tizen/versions/tizen-9-0-m2/)
