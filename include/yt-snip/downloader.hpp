#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace yt_snip {

class YouTubeDownloader {
public:
    YouTubeDownloader();
    ~YouTubeDownloader() = default;

    YouTubeDownloader(const YouTubeDownloader&) = delete;
    YouTubeDownloader& operator=(const YouTubeDownloader&) = delete;

    bool downloadVideo(const std::string& url);
    bool downloadAudio(const std::string& url);
    bool downloadPlaylistVideo(const std::string& url);
    bool downloadPlaylistAudio(const std::string& url);
    bool trimVideo(const std::string& url, const std::string& startTime, const std::string& endTime);
    bool trimAudio(const std::string& url, const std::string& startTime, const std::string& endTime);

private:
    std::string outputDir;

    int executeCommand(const std::string& command);
    std::string executeCommandWithOutput(const std::string& command);
    void printProgressBar(int current, int total, const std::string& title, int barWidth = 50);
    int getPlaylistCount(const std::string& url);
    std::vector<std::string> getPlaylistUrls(const std::string& url);
    std::string getVideoTitle(const std::string& url);
    std::string getPlaylistTitle(const std::string& url);
    std::string sanitizeFilename(const std::string& filename);
    void cleanupFiles(const std::vector<std::string>& files);
    std::string convertYouTubeMusicURL(const std::string& url);
    void ensureDirectoryExists(const std::string& dir);
    std::string getOutputPath(const std::string& filename);
    bool downloadMedia(const std::string& url, const std::string& outputName, bool audioOnly = false, bool isPlaylist = false);
    bool downloadPlaylist(const std::string& url, bool audioOnly);
    bool trimAndDownload(const std::string& url, const std::string& startTime, const std::string& endTime, bool isAudio);
};

} 