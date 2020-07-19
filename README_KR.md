# EdiZon
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/icon.jpg"><br />
      <a href="https://github.com/WerWolv/EdiZon/releases/latest"><img src="https://img.shields.io/github/downloads/WerWolv/EdiZon/total.svg" alt="Latest Release" /></a>
    <a href="https://discord.gg/qyA38T8"><img src="https://discordapp.com/api/guilds/465980502206054400/embed.png" alt="Discord Server" /></a>
    <a href="https://travis-ci.com/WerWolv/EdiZon"><img src="https://travis-ci.com/WerWolv/EdiZon.svg?branch=master" alt="Build Status" /></a>
  </p>

홈브류는 닌텐도 스위치의 OS 인 Horizon의 파일 저장 장치 인젝터 및 콘솔 편집기를 저장합니다.

# 개요
  EdiZon은 3 가지 주요 기능으로 구성되어 있습니다.
  - **저장 파일 관리**
    - 게임 저장 추출.
    - 추출된 게임 저장 주입 (여러분과 여러분의 친구들은 파일을 저장합니다).
    - savefiles를 https://anonfile.com 에 직접 업로드.
    - 시스템에있는 모든 게임의 모든 저장 파일을 일괄적으로 추출.
  - **저장 파일 수정**
    - 사용하기 쉽고 스크립팅 가능하며 콘솔에서 쉽게 확장할 수 있음.
      - 루아와 파이썬 스크립트 지원.
    - 내장된 저장 편집기 업데이트 프로그램.
  - **On-the-fly 메모리 편집**
    - RAM 편집과 같은 치트 엔진.
    - Atmosphère의 치트 모듈을 통해 RAM에 값 고정.
    - Atmosphère 차투를 로딩, 관리, 업데이트하기 위한 인터페이스.

  모두 사용하기 쉽고 홈브류를 쉽게 설치할 수 있습니다.

# 이미지
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/main_menu.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/save_editor_1.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/save_editor_2.jpg"></p>
  <p align="center"><img src="https://raw.githubusercontent.com/WerWolv/EdiZon/master/assets/ram_editor.jpg"></p>

# 편집기 구성 및 스크립트 파일 저장

  작업 편집기 설정 및 편집기 스크립트 파일을 다운로드하려면 [이 저장소](https://github.com/WerWolv/EdiZon_ConfigsAndScripts/tree/master) 를 방문하세요.

  에디터 구성 및 에디터 스크립트 파일을 직접 작성하는 방법에 대한 자세한 내용은 [위키 페이지](https://github.com/WerWolv/EdiZon/wiki) 를 확인하세요.

# 설치하는 방법

  1. [GitHub 릴리즈 페이지](https://github.com/WerWolv/EdiZon/releases/latest). 에서 최신 릴리스를 다운로드하세요.
  2. 다운로드 한 zip 파일의 압축을 풀고 파일을 닌텐도 스위치의 SD 카드에 넣고 폴더를 병합시킵니다.
  3. [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere) 와 같은 무료 오픈 소스 CFW를 사용하여 hbmenu를 실행하고 거기에서 EdiZon을 시작하세요.
     1. 치트 관리자를 사용하고 싶다면 반드시 [Atmosphère](https://github.com/Atmosphere-NX/Atmosphere) 를 사용해야 만합니다. 왜냐하면 치트만 지원되기 때문입니다.
     2. 가장 좋은 경험을 위해서 `/atmosphere/system_settings.ini` 파일을 열고 `dmnt_cheats_enabled_by_default = u8!0x1` 를 `dmnt_cheats_enabled_by_default = u8!0x0` 로 변경하십시오..


# 컴파일하는 방법

  1. `git clone https://github.com/WerWolv/EdiZon` 을 사용하여 EdiZon repo를 컴퓨터에 복제하세요.
  2. devkitA64를 다운로드하여 설치하세요. [devkitPro](https://devkitpro.org) 툴 체인과 함께 번들로 제공됩니다.
  3. devkitPro와 함께 제공되는 팩맨 패키지 관리자를 사용하여 libNX, portlibs (`switch-portlibs`) 및 freetype2 (`switch-freetype`)를 다운로드하고 설치하세요.
  4. 나머지 컴파일은 `make` 명령을 사용하여 작동합니다.

# Discord

  EdiZon의 사용 또는 저장 편집기 구성, 스크립트 작성을 지원하려면 Disord의 EdiZon 서버에 자유롭게 가입하세요: https://discord.gg/qyA38T8

# 크레딧

  고맙습니다...

  - 그들의 놀라운 툴체인, [devkitPro](https://devkitpro.org) !
  - [덤프/주입 저장](https://github.com/3096/nut) , [3096](https://github.com/3096) .
  - [Checkpoint](https://github.com/BernardoGiordano/Checkpoint) 일부 코드, [Bernardo Giordano](https://github.com/BernardoGiordano) .
  - [홈브류 런처](https://github.com/switchbrew/nx-hbmenu) GUI 및 공유 글꼴 코드, [SwitchBrew](https://switchbrew.org/) .
  - 백업 및 복원 코드, 업데이터 스크립트 대부분 저장을 위한 [thomasnet-mc](https://github.com/thomasnet-mc/) .
  - 배치 백업 및 편집 전용 모드를 지원, [trueicecold](https://github.com/trueicecold) .
  - edizon 디버거와 구현을 검토하는 많은 사람들을 위한 [onepiecefreak](https://github.com/onepiecefreak3) .
  - Travis CI 구성 및 구성 생성 프로그램, [Jojo](https://github.com/drdrjojo) .
  - 서버 측 업데이트 스크립트 및 EdiZon 저장 웹 사이트에 대한 도움, [Ac_K](https://github.com/AcK77) .
  - RAM 편집 및 영감으로 사용된 sys-netcheat 구현에 대한 커다란 도움, [jakibaki](https://github.com/jakibaki) .
  - aarch64 하드웨어 가속화 SHA256 코드, Atmosphère 치트 엔진 구현, 개발 지원, [SciresM](https://github.com/SciresM) .
  - 아름다운 현재 아이콘, **kardch**.
  - 아름다운 오래된 아이콘, **bernv3**.
  - 이 프로젝트에 생명을 불어 넣기 위한 **모든 설정 제작자**!

  <br>

  - 위대한 json 라이브러리, [nlohmann](https://github.com/nlohmann) .
  - nanojpeg JPEG 디코딩 라이브러리, [Martin J. Fiedler](https://svn.emphy.de/nanojpeg/trunk/nanojpeg/nanojpeg.c) .
  - 스크립팅 언어, [Lua](https://www.lua.org/) .
  - 스크립팅 언어를 각각 파이썬 포트를 스위치에 연결하는, [Python](https://www.python.org/) and [nx-python](https://github.com/nx-python) .


  <br>
  <p align="center"><img src="https://www.lua.org/images/logo.gif">
  <img src="https://upload.wikimedia.org/wikipedia/commons/c/c3/Python-logo-notext.svg"><p>
