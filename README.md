# yt-snip

Fast YouTube video/audio downloader and trimmer - a yt-dlp wrapper.

## Install

**From AUR:**
```bash
yay -S yt-snip
```

**From source:**
```bash
git clone https://github.com/melqtx/yt-snip.git
cd yt-snip
make
sudo make install
```

## Usage

```bash
# Download full video
yt-snip video "https://youtube.com/watch?v=VIDEO_ID"

# Download audio only
yt-snip audio "https://youtube.com/watch?v=VIDEO_ID"

# Download and trim video (1:30 to 3:45)
yt-snip trim-video "https://youtube.com/watch?v=VIDEO_ID" "00:01:30" "00:03:45"

# Download and trim audio (90s to 180s)
yt-snip trim-audio "https://youtube.com/watch?v=VIDEO_ID" "90" "180"
```

## Dependencies

- `yt-dlp`
- `ffmpeg`

## Output

- Videos: `output.webm`
- Audio: `output.opus`
