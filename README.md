# YouTube Downloader

Simple C++ tool to download and trim YouTube videos/audio.
in other words just a yt dlp wrapper

## What you need
- `yt-dlp` 
- `ffmpeg`
- C++17 compiler

## Build & Run
```bash
g++ -std=c++17 -o youtube_downloader main.cc
./youtube_downloader
```

## Options
1. Download full video
2. Download audio only  
3. Trim video
4. Trim audio

Use time format like `00:01:30` or `90` for trimming.

Output files: `output.webm`, `output.opus`
