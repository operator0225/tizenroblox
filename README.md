# TizenRoblox

Samsung Tizen 9.0 TV(KQ83SF95AEXKR, OLED SF95 83")에서 Roblox를 네이티브로 구동하는 프로젝트.
실제 [Sober](https://github.com/vinegarhq/sober) ARM64 런타임 + 얇은 Tizen 호환 레이어로 구성.

- 삼성 TV 리모컨 → 가상 게임패드 매핑 지원
- 실제 물리 게임패드(USB/블루투스) 패스스루 지원
- 핵심 라이브러리(OpenSSL, curl, libxml2, glib, freetype, fontconfig) 실제 바이너리 정적 번들
- GPU/컴포지터 종속 라이브러리(EGL, GLES, Wayland)는 시스템 것 사용

작업 이력은 [`PROGRESS.md`](PROGRESS.md), TV 하드웨어 분석은
[`KQ83SF95AEXKR_PERFORMANCE_ANALYSIS.md`](KQ83SF95AEXKR_PERFORMANCE_ANALYSIS.md) 참고.

---

## 아키텍처

```
[Samsung KQ83SF95AEXKR TV — Tizen 9.0 / Enlightenment Wayland]
        │
        ├── tizen/launch.sh ─────────────────── 메인 런처
        │       │
        │       ├── input_mapper ─────────────── Samsung 리모컨 → uinput 가상 게임패드
        │       │       └── /dev/uinput → /dev/input/eventX (Sober가 읽음)
        │       │
        │       └── sober (실제 Sober ARM64 바이너리)
        │               ├── libloader.so ──────── 안드로이드 ELF 로더 (Sober 번들)
        │               ├── libbadcpu.so ───────── CPU 검증 (Sober 번들)
        │               ├── libmimalloc.so.3 ──── 메모리 할당자 (Sober 번들)
        │               │
        │               ├── [번들] libcrypto.so.3 ─────── 실제 OpenSSL 3.0.13 (정적 링크)
        │               ├── [번들] libcurl.so.4 ────────── 실제 curl 8.10.1
        │               ├── [번들] libxml2.so.16 ───────── 실제 libxml2 2.12.9
        │               ├── [번들] libglib-2.0.so.0 ────── 실제 GLib 2.82.2
        │               ├── [번들] libgobject-2.0.so.0 ── 실제 GObject 2.82.2
        │               ├── [번들] libfreetype.so.6 ───── 실제 FreeType 2.13.3
        │               ├── [번들] libfontconfig.so.1 ─── 실제 fontconfig 2.15.0
        │               │
        │               ├── [위임] libgstreamer/app/video ─ 시스템 GStreamer (HW 코덱 필수)
        │               ├── [위임] libdbus-1.so.3 ────────── 시스템 D-Bus (없으면 no-op)
        │               ├── [위임] libdecor-0.so.0 ───────── 시스템 or fullscreen 폴백
        │               ├── [자체구현] libsecret-1.so.0 ──── 파일 기반 자격증명 저장
        │               │
        │               └── [시스템 필수] libEGL, libGLESv2,
        │                       libwayland-client/egl/cursor ── 삼성 GPU 드라이버 직결
        │
        └── 첫 실행 시 Roblox 안드로이드 엔진(Sober가 관리)을
              공식 CDN에서 다운로드 — 이 저장소엔 포함 안 됨
```

**왜 "번들 vs 위임"으로 나뉘는가**: OpenSSL/curl/libxml2/glib/freetype/fontconfig는
하드웨어에 접근하지 않는 순수 라이브러리라 실제 바이너리를 크로스컴파일해서 통째로
넣을 수 있음 (Android APK가 `.so`를 번들하는 것과 동일한 방식). 반면 EGL/GLES/Wayland는
삼성 TV의 실제 GPU 드라이버·컴포지터와 직접 통신해야 하므로 범용 구현을 넣으면 하드웨어
가속이 깨짐 — 반드시 시스템 것을 써야 함. GStreamer는 하드웨어 비디오 코덱이 시스템의
plugin registry에만 있어서 시스템 전체(core+plugins)를 위임.

---

## 디렉터리 구조

```
tizenroblox/
├── stub/                  라이브러리 호환 레이어 (C 소스 + CMakeLists.txt)
├── tizen/                 input_mapper.c(리모컨→게임패드), launch.sh(메인 런처)
├── scripts/
│   ├── build_thirdparty.sh   OpenSSL/curl/libxml2/glib/freetype/fontconfig 크로스컴파일
│   ├── extract_sober.sh      Sober Flatpak에서 ARM64 바이너리 추출
│   ├── build.sh               전체 크로스컴파일 + dist/ 조립
│   ├── package_tpk.sh          .tpk 패키징 (서명/비서명)
│   ├── deploy.sh / deploy_sdb.sh  TV로 배포 (SSH / SDB)
│   ├── diagnose.sh             TV 런타임 진단 (라이브러리·장치·glibc 버전 등)
│   └── setup.sh                 Roblox 자격증명 최초 설정
├── toolchain/aarch64-tizen.cmake  CMake aarch64 크로스컴파일 툴체인
├── thirdparty/
│   ├── env.sh, cross-aarch64.ini   빌드 환경 (크로스컴파일러, Meson cross-file)
│   ├── install/                     크로스컴파일된 정적 라이브러리 (git에 포함됨)
│   └── src/                         소스 tarball + 빌드 중간 파일 (gitignore, 재생성됨)
├── sober_bundle/            추출된 실제 Sober ARM64 바이너리 (git에 포함됨)
├── dist/                     최종 조립된 배포판 (bin/ + lib/, git에 포함됨)
├── tizenroblox.tpk            패키징된 사이드로드용 tpk (git에 포함됨)
└── build/                     CMake 빌드 중간 산출물 (gitignore, 매번 재생성)
```

---

## 빌드 & 배포

### 최초 1회 (또는 라이브러리 버전 바꿀 때만)
```bash
bash scripts/build_thirdparty.sh   # OpenSSL/curl/libxml2/glib/freetype/fontconfig 크로스컴파일 (~15-20분)
```
이미 `thirdparty/install/`이 git에 포함돼 있으므로 보통 생략 가능.

### Sober 바이너리 추출 (최초 1회, 또는 Sober 버전 업데이트 시)
```bash
bash scripts/extract_sober.sh <path/to/org.vinegarhq.Sober.flatpak>
```
이미 `sober_bundle/`이 git에 포함돼 있으므로 보통 생략 가능.

### 매번 빌드
```bash
bash scripts/build.sh              # dist/ 조립 (수 초)
bash scripts/package_tpk.sh        # tizenroblox.tpk 생성
```

### TV로 배포
```bash
# 방법 1: SDB (개발자 모드 켠 TV)
sdb connect <TV_IP>:26101
sdb install tizenroblox.tpk

# 방법 2: SSH (root 접근 가능한 경우)
bash scripts/deploy.sh <TV_IP>
```

### 진단 & 실행
```bash
ssh root@<TV_IP> bash /opt/tizenroblox/scripts/diagnose.sh   # 또는 tpk 설치 후 자동 실행 경로
ssh root@<TV_IP> /opt/tizenroblox/bin/launch.sh
tail -f /opt/tizenroblox/logs/tizenroblox.log
```

Enlightenment 컴포지터가 xdg_shell을 지원 안 하면:
```bash
export TIZENROBLOX_FORCE_LIBDECOR_STUB=1
```

---

## 게임패드

- **삼성 리모컨**: `input_mapper`가 `/dev/input/eventX`의 리모컨 입력을 읽어 `/dev/uinput`으로
  가상 Xbox 호환 게임패드를 생성 (방향키→DPAD, OK→A, 컬러버튼→X/Y/LB/RB 등)
- **실제 게임패드**: USB/블루투스로 연결하면 Sober가 `/dev/input/eventX`에서 직접 읽음
  (별도 매핑 불필요, `input_mapper`는 신규 연결만 감지·로그)

---

## 알려진 제약사항

- **glibc 버전**: 번들 라이브러리는 Ubuntu 24.04 크로스툴체인으로 빌드했고, isoc23 심볼
  하향 패치로 최소 요구사항을 **GLIBC 2.34**까지 낮췄음 (2021년 기준, 매우 안전한 하한선).
  Tizen 9.0의 정확한 glibc 버전은 실기 미확인 — `scripts/diagnose.sh`가 체크함.
- **첫 실행 다운로드**: 이 저장소는 Roblox 게임 엔진 자체를 포함하지 않음. Sober가 처음
  실행될 때 로블록스 공식 서버에서 안드로이드 엔진 바이너리를 다운로드(수백 MB대) —
  TV 저장공간과 네트워크 필요.
- **4K 고주사율**: TV 패널이 4K 165Hz 입력을 지원해도, NQ4 AI Gen3 SoC의 내장 GPU가
  Roblox 콘텐츠를 4K에서 165fps로 실시간 렌더링하는 건 비현실적. 30-60fps가 현실적 기대치.
- **실기 미검증**: Wayland EGL 초기화, 오디오(ALSA/PulseAudio), GStreamer 비디오 파이프라인은
  실제 TV에서 아직 테스트 안 됨 — `PROGRESS.md`의 "남은 작업" 참고.

---

## 트러블슈팅

`scripts/diagnose.sh`가 다음을 자동 점검:
아키텍처/커널, glibc 버전, `/dev/uinput` 권한, ELF interpreter, Sober 바이너리+번들 라이브러리,
Wayland 소켓, GPU/EGL, 오디오, 번들 라이브러리 7종 + 위임 스텁 6종, D-Bus, 입력 장치, 자격증명 파일.

로그 위치: `<INSTALL_DIR>/logs/tizenroblox.log` (10MB 초과 시 `.old`로 자동 로테이션)

크래시 시 자동 복구: 60초 내 최대 3회까지 자동 재시작 (`launch.sh`)

---

## 참고 링크

- Sober 소스: https://github.com/vinegarhq/sober
- Sober ARM64 이슈: https://github.com/vinegarhq/sober/issues/1221
- Tizen Developer Docs: https://developer.tizen.org
