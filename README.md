# yt-snip

Fast YouTube video/audio downloader and trimmer - a yt-dlp wrapper.

## Features

- Download videos and audio from YouTube and YouTube Music
- Download entire playlists with automatic indexing
- Extract audio in opus format
- Trim videos and audio using timestamps
- Handles errors gracefully during playlist downloads
- Organized file storage in dedicated folders

## Dependencies

- `yt-dlp`
- `ffmpeg`
- CMake (>= 3.14)
- C++17 compatible compiler

## Build & Install

**From AUR:**
```bash
yay -S yt-snip
```

**From source:**
```bash
# Clone the repository
git clone https://github.com/melqtx/yt-snip.git
cd yt-snip

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Install (optional)
sudo make install
```

## Usage

```bash
# Download full video
yt-snip video "https://youtube.com/watch?v=VIDEO_ID"

# Download audio only
yt-snip audio "https://youtube.com/watch?v=VIDEO_ID"

# Download all videos from a playlist
yt-snip playlist-video "https://youtube.com/playlist?list=PLAYLIST_ID"

# Download all audio from a playlist (works with YouTube Music)
yt-snip playlist-audio "https://music.youtube.com/playlist?list=PLAYLIST_ID"

# Download and trim video (1:30 to 3:45)
yt-snip trim-video "https://youtube.com/watch?v=VIDEO_ID" "00:01:30" "00:03:45"

# Download and trim audio (90s to 180s)
yt-snip trim-audio "https://youtube.com/watch?v=VIDEO_ID" "90" "180"
```

## Output Organization

Downloads are automatically organized in the following directory structure:

```
.
├── video/
│   ├── singles/     # Individual video downloads
│   └── playlists/   # Playlist video downloads
└── audio/
    ├── singles/     # Individual audio downloads
    └── playlists/   # Playlist audio downloads
```

### File Naming

- Individual videos: `./video/singles/[Title].webm`
- Individual audio: `./audio/singles/[Title].opus`
- Playlist videos: `./video/playlists/[Title][Playlist_Index].webm`
- Playlist audio: `./audio/playlists/[Title][Playlist_Index].opus`
- Trimmed videos: `./video/singles/trimmed_[Title].webm`
- Trimmed audio: `./audio/singles/trimmed_[Title].opus`

## Supported URLs

- YouTube (youtube.com, youtu.be)
- YouTube Music (music.youtube.com)


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
