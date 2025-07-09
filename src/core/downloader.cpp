#include "yt-snip/downloader.hpp"
#include <cstdlib>
#include <iostream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <algorithm>

namespace yt_snip {

YouTubeDownloader::YouTubeDownloader() : outputDir(".") {
    initializeDirectoryStructure();
}

void YouTubeDownloader::initializeDirectoryStructure() {
    // Create main directories
    ensureDirectoryExists(VIDEO_DIR);
    ensureDirectoryExists(AUDIO_DIR);

    // Create subdirectories
    ensureDirectoryExists(std::string(VIDEO_DIR) + "/" + SINGLES_DIR);
    ensureDirectoryExists(std::string(VIDEO_DIR) + "/" + PLAYLISTS_DIR);
    ensureDirectoryExists(std::string(AUDIO_DIR) + "/" + SINGLES_DIR);
    ensureDirectoryExists(std::string(AUDIO_DIR) + "/" + PLAYLISTS_DIR);
}

std::string YouTubeDownloader::getVideoOutputPath(const std::string& filename, bool isPlaylist) {
    std::string subdir = isPlaylist ? PLAYLISTS_DIR : SINGLES_DIR;
    return std::string(VIDEO_DIR) + "/" + subdir + "/" + filename;
}

std::string YouTubeDownloader::getAudioOutputPath(const std::string& filename, bool isPlaylist) {
    std::string subdir = isPlaylist ? PLAYLISTS_DIR : SINGLES_DIR;
    return std::string(AUDIO_DIR) + "/" + subdir + "/" + filename;
}

bool YouTubeDownloader::downloadVideo(const std::string& url) {
    std::string title = getVideoTitle(url);
    std::string outputPath = getVideoOutputPath(sanitizeFilename(title), false);
    return downloadMedia(url, outputPath, false, false);
}

bool YouTubeDownloader::downloadAudio(const std::string& url) {
    std::string title = getVideoTitle(url);
    std::string outputPath = getAudioOutputPath(sanitizeFilename(title), false);
    return downloadMedia(url, outputPath, true, false);
}

bool YouTubeDownloader::downloadPlaylistVideo(const std::string& url) {
    return downloadMedia(url, "", false, true);
}

bool YouTubeDownloader::downloadPlaylistAudio(const std::string& url) {
    return downloadMedia(url, "", true, true);
}

bool YouTubeDownloader::trimVideo(const std::string& url, const std::string& startTime, const std::string& endTime) {
    std::string title = getVideoTitle(url);
    std::string outputPath = getVideoOutputPath("trimmed_" + sanitizeFilename(title), false);
    return trimAndDownload(url, startTime, endTime, false);
}

bool YouTubeDownloader::trimAudio(const std::string& url, const std::string& startTime, const std::string& endTime) {
    std::string title = getVideoTitle(url);
    std::string outputPath = getAudioOutputPath("trimmed_" + sanitizeFilename(title), false);
    return trimAndDownload(url, startTime, endTime, true);
}

int YouTubeDownloader::executeCommand(const std::string& command) {
    return std::system(command.c_str());
}

std::string YouTubeDownloader::executeCommandWithOutput(const std::string& command) {
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";
    
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

void YouTubeDownloader::printProgressBar(int current, int total, const std::string& title, int barWidth) {
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(barWidth * progress);

    std::cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "█";
        else if (i == pos) std::cout << "▒";
        else std::cout << "░";
    }
    std::cout << "] " << std::setw(3) << static_cast<int>(progress * 100) << "% "
              << "(" << current << "/" << total << ") " << title;
    std::cout.flush();
}

int YouTubeDownloader::getPlaylistCount(const std::string& url) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    std::string command = "yt-dlp --quiet --no-warnings --flat-playlist --dump-json \"" + processedUrl + "\" 2>/dev/null | wc -l";
    std::string output = executeCommandWithOutput(command);
    
    try {
        return std::stoi(output);
    } catch (...) {
        return 0;
    }
}

std::vector<std::string> YouTubeDownloader::getPlaylistUrls(const std::string& url) {
    std::vector<std::string> urls;
    std::string processedUrl = convertYouTubeMusicURL(url);
    std::string command = "yt-dlp --quiet --no-warnings --flat-playlist --get-url \"" + processedUrl + "\" 2>/dev/null";
    std::string output = executeCommandWithOutput(command);
    
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            urls.push_back(line);
        }
    }
    return urls;
}

std::string YouTubeDownloader::getVideoTitle(const std::string& url) {
    std::string command = "yt-dlp --quiet --no-warnings --get-title \"" + url + "\" 2>/dev/null";
    std::string title = executeCommandWithOutput(command);
    if (!title.empty() && title.back() == '\n') {
        title.pop_back();
    }
    if (title.length() > 60) {
        title = title.substr(0, 57) + "...";
    }
    return title.empty() ? "Unknown Title" : title;
}

std::string YouTubeDownloader::getPlaylistTitle(const std::string& url) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    std::string command = "yt-dlp --quiet --no-warnings --print playlist_title \"" + processedUrl + "\" 2>/dev/null | head -1";
    std::string title = executeCommandWithOutput(command);
    
    title.erase(std::remove(title.begin(), title.end(), '\n'), title.end());
    title.erase(std::remove(title.begin(), title.end(), '\r'), title.end());
    
    if (!title.empty()) {
        std::regex invalid_chars("[<>:\"/\\|?*]");
        title = std::regex_replace(title, invalid_chars, "_");
        
        std::regex multiple_spaces("\\s+");
        title = std::regex_replace(title, multiple_spaces, " ");
        
        title.erase(0, title.find_first_not_of(" \t"));
        title.erase(title.find_last_not_of(" \t") + 1);
        
        if (title.length() > 50) {
            title = title.substr(0, 47) + "...";
        }
    }
    
    return title.empty() ? "Unknown_Playlist" : title;
}

std::string YouTubeDownloader::sanitizeFilename(const std::string& filename) {
    std::regex invalid_chars("[<>:\"/\\|?*]");
    return std::regex_replace(filename, invalid_chars, "_");
}

void YouTubeDownloader::cleanupFiles(const std::vector<std::string>& files) {
    for (const auto& file : files) {
        if (std::filesystem::exists(file)) {
            std::filesystem::remove(file);
        }
    }
}

std::string YouTubeDownloader::convertYouTubeMusicURL(const std::string& url) {
    std::regex music_pattern("music\\.youtube\\.com");
    return std::regex_replace(url, music_pattern, "www.youtube.com");
}

void YouTubeDownloader::ensureDirectoryExists(const std::string& dir) {
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
}

std::string YouTubeDownloader::getOutputPath(const std::string& filename) {
    return filename;
}

bool YouTubeDownloader::downloadMedia(const std::string& url, const std::string& outputName, bool audioOnly, bool isPlaylist) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    
    if (isPlaylist) {
        return downloadPlaylist(processedUrl, audioOnly);
    }
    
    std::string outputPath = outputName + ".%(ext)s";
    std::string command = "yt-dlp --quiet --no-warnings -o \"" + outputPath + "\" ";
    if (audioOnly) {
        command += "-x --audio-format opus ";
    }
    command += "--no-playlist \"" + processedUrl + "\"";

    std::cout << "Downloading " << (audioOnly ? "audio" : "video") << "..." << std::endl;
    int result = executeCommand(command);
    if (result != 0) {
        std::cerr << "Error downloading " << (audioOnly ? "audio" : "video") << std::endl;
        return false;
    }
    std::cout << "Download complete!" << std::endl;
    return true;
}

bool YouTubeDownloader::downloadPlaylist(const std::string& url, bool audioOnly) {
    std::cout << "Analyzing playlist..." << std::endl;
    
    std::string playlistTitle = getPlaylistTitle(url);
    std::cout << "Playlist: " << playlistTitle << std::endl;
    
    // Create playlist directory inside the appropriate category
    std::string playlistDir = audioOnly ? 
        getAudioOutputPath(playlistTitle, true) : 
        getVideoOutputPath(playlistTitle, true);
    ensureDirectoryExists(playlistDir);
    
    int totalCount = getPlaylistCount(url);
    if (totalCount == 0) {
        std::cerr << "Error: Could not get playlist information or playlist is empty" << std::endl;
        return false;
    }
    
    std::cout << "Found " << totalCount << " items in playlist" << std::endl;
    std::cout << "Starting download...\n" << std::endl;
    
    std::vector<std::string> urls = getPlaylistUrls(url);
    if (static_cast<int>(urls.size()) != totalCount) {
        std::cout << "Warning: Expected " << totalCount << " items but found " << urls.size() << " URLs" << std::endl;
        totalCount = static_cast<int>(urls.size());
    }
    
    int successCount = 0;
    for (int i = 0; i < totalCount; i++) {
        std::string title = getVideoTitle(urls[i]);
        printProgressBar(i + 1, totalCount, title);
        
        std::string indexStr = std::to_string(i + 1);
        if (indexStr.length() == 1) indexStr = "0" + indexStr;
        
        std::string outputName = playlistDir + "/" + indexStr + "_" + sanitizeFilename(title);
        if (downloadMedia(urls[i], outputName, audioOnly, false)) {
            successCount++;
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nPlaylist download complete! Successfully downloaded " << successCount << "/" << totalCount << " items" << std::endl;
    return successCount > 0;
}

bool YouTubeDownloader::trimAndDownload(const std::string& url, const std::string& startTime, const std::string& endTime, bool isAudio) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    
    std::string title = getVideoTitle(processedUrl);
    title = sanitizeFilename(title);
    std::string outputFile = title + (isAudio ? ".opus" : ".mp4");
    
    std::cout << "Downloading and trimming " << (isAudio ? "audio" : "video") << "..." << std::endl;
    
    std::string command;
    if (isAudio) {
        command = "yt-dlp --quiet --no-warnings -f 'bestaudio' --no-playlist \"" + processedUrl + "\" -o - 2>/dev/null | ";
        command += "ffmpeg -hide_banner -loglevel error -i pipe: -ss " + startTime + " -to " + endTime + " ";
        command += "-c:a libopus \"" + outputFile + "\" 2>/dev/null";
    } else {
        command = "yt-dlp --quiet --no-warnings -f 'best' --no-playlist \"" + processedUrl + "\" -o - 2>/dev/null | ";
        command += "ffmpeg -hide_banner -loglevel error -i pipe: -ss " + startTime + " -to " + endTime + " ";
        command += "-c:v copy -c:a copy \"" + outputFile + "\" 2>/dev/null";
    }
    
    int result = executeCommand(command);
    if (result != 0) {
        std::cerr << "Error trimming " << (isAudio ? "audio" : "video") << std::endl;
        return false;
    }
    
    std::cout << "Trimming complete! Saved as: " << outputFile << std::endl;
    return true;
}

} 