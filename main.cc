#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <algorithm>

namespace fs = std::filesystem;

/*
#TODO
- video download
- video trimming
- audio trimming
*/
class YouTubeDownloader {
private:
  std::string outputDir = ".";

  int executeCommand(const std::string &command) {
    return std::system(command.c_str());
  }

  std::string executeCommandWithOutput(const std::string &command) {
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

  void printProgressBar(int current, int total, const std::string &title, int barWidth = 50) {
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

  int getPlaylistCount(const std::string &url) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    std::string command = "yt-dlp --quiet --no-warnings --flat-playlist --dump-json \"" + processedUrl + "\" 2>/dev/null | wc -l";
    std::string output = executeCommandWithOutput(command);
    
    try {
      return std::stoi(output);
    } catch (...) {
      return 0;
    }
  }

  std::vector<std::string> getPlaylistUrls(const std::string &url) {
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

  std::string getVideoTitle(const std::string &url) {
    std::string command = "yt-dlp --quiet --no-warnings --get-title \"" + url + "\" 2>/dev/null";
    std::string title = executeCommandWithOutput(command);
    // Remove newline and limit length for display
    if (!title.empty() && title.back() == '\n') {
      title.pop_back();
    }
    if (title.length() > 60) {
      title = title.substr(0, 57) + "...";
    }
    return title.empty() ? "Unknown Title" : title;
  }

  std::string getPlaylistTitle(const std::string &url) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    std::string command = "yt-dlp --quiet --no-warnings --print playlist_title \"" + processedUrl + "\" 2>/dev/null | head -1";
    std::string title = executeCommandWithOutput(command);
    
    // Remove all newlines and carriage returns
    title.erase(std::remove(title.begin(), title.end(), '\n'), title.end());
    title.erase(std::remove(title.begin(), title.end(), '\r'), title.end());
    
    // Clean up title for use as folder name
    if (!title.empty()) {
      // Replace invalid characters for folder names
      std::regex invalid_chars("[<>:\"/\\|?*]");
      title = std::regex_replace(title, invalid_chars, "_");
      
      // Remove multiple spaces and trim
      std::regex multiple_spaces("\\s+");
      title = std::regex_replace(title, multiple_spaces, " ");
      
      // Trim leading and trailing spaces
      title.erase(0, title.find_first_not_of(" \t"));
      title.erase(title.find_last_not_of(" \t") + 1);
      
      // Limit length
      if (title.length() > 50) {
        title = title.substr(0, 47) + "...";
      }
    }
    
    return title.empty() ? "Unknown_Playlist" : title;
  }

  std::string sanitizeFilename(const std::string &filename) {
    std::regex invalid_chars("[<>:\"/\\|?*]");
    return std::regex_replace(filename, invalid_chars, "_");
  }

  void cleanupFiles(const std::vector<std::string> &files) {
    for (const auto &file : files) {
      if (fs::exists(file)) {
        fs::remove(file);
      }
    }
  }

  std::string convertYouTubeMusicURL(const std::string &url) {
    std::regex music_pattern("music\\.youtube\\.com");
    return std::regex_replace(url, music_pattern, "www.youtube.com");
  }

  void ensureDirectoryExists(const std::string &dir) {
    if (!fs::exists(dir)) {
      fs::create_directories(dir);
    }
  }

  std::string getOutputPath(const std::string &filename) {
    return filename;
  }

  bool downloadMedia(const std::string &url, const std::string &outputName, bool audioOnly = false, bool isPlaylist = false) {
    // Convert YouTube Music URLs to regular YouTube URLs
    std::string processedUrl = convertYouTubeMusicURL(url);
    
    if (isPlaylist) {
      return downloadPlaylist(processedUrl, audioOnly);
    }
    
    // Single video/audio download - save directly in current directory
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

  bool trimAndDownload(const std::string &url, const std::string &startTime, const std::string &endTime, bool isAudio) {
    std::string processedUrl = convertYouTubeMusicURL(url);
    
    // Get video title for output filename
    std::string title = getVideoTitle(processedUrl);
    title = sanitizeFilename(title);
    std::string outputFile = title + (isAudio ? ".opus" : ".mp4");
    
    std::cout << "Downloading and trimming " << (isAudio ? "audio" : "video") << "..." << std::endl;
    
    std::string command;
    if (isAudio) {
      // For audio: pipe yt-dlp audio to ffmpeg for trimming
      command = "yt-dlp --quiet --no-warnings -f 'bestaudio' --no-playlist \"" + processedUrl + "\" -o - 2>/dev/null | ";
      command += "ffmpeg -hide_banner -loglevel error -i pipe: -ss " + startTime + " -to " + endTime + " ";
      command += "-c:a libopus \"" + outputFile + "\" 2>/dev/null";
    } else {
      // For video: pipe yt-dlp video to ffmpeg for trimming
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

  bool downloadPlaylist(const std::string &url, bool audioOnly) {
    std::cout << "Analyzing playlist..." << std::endl;
    
    // Get playlist title for folder name
    std::string playlistTitle = getPlaylistTitle(url);
    std::cout << "Playlist: " << playlistTitle << std::endl;
    
    // Create playlist folder
    ensureDirectoryExists(playlistTitle);
    
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
    int failedCount = 0;
    
    for (int i = 0; i < totalCount; i++) {
      std::string title = getVideoTitle(urls[i]);
      printProgressBar(i + 1, totalCount, title);
      
      // Download individual item into playlist folder with manual indexing
      std::string indexStr = std::to_string(i + 1);
      if (indexStr.length() == 1) indexStr = "0" + indexStr; // Pad single digits
      
      std::string outputPath = playlistTitle + "/" + indexStr + " - %(title)s.%(ext)s";
      std::string command = "yt-dlp --quiet --no-warnings -o \"" + outputPath + "\" ";
      if (audioOnly) {
        command += "-x --audio-format opus ";
      }
      command += "--no-playlist \"" + urls[i] + "\" >/dev/null 2>&1";
      
      int result = executeCommand(command);
      if (result == 0) {
        successCount++;
      } else {
        failedCount++;
        std::cout << "\nFailed to download: " << title << std::endl;
      }
    }
    
    std::cout << "\n\nPlaylist download complete!" << std::endl;
    std::cout << "Results: " << successCount << " successful, " << failedCount << " failed" << std::endl;
    std::cout << "Files saved in: ./" << playlistTitle << "/" << std::endl;
    
    return failedCount == 0;
  }

  bool trimMedia(const std::string &inputFile, const std::string &outputFile,
                 const std::string &startTime, const std::string &endTime, bool isAudio = false) {
    std::string command = "ffmpeg -hide_banner -loglevel error ";

    if (isAudio) {
      command += "-ss " + startTime + " -to " + endTime + " -i " + inputFile + " \"" + outputFile + "\"";
    } else {
      command += "-i " + inputFile + " -ss " + startTime + " -to " + endTime +
                 " -c:v copy -c:a copy \"" + outputFile + "\"";
    }

    std::cout << "Trimming " << (isAudio ? "audio" : "video") << "..." << std::endl;
    int result = executeCommand(command);
    if (result != 0) {
      std::cerr << "Error trimming " << (isAudio ? "audio" : "video") << std::endl;
      return false;
    }
    std::cout << "Trimming complete!" << std::endl;
    return true;
  }

public:
  void printUsage() {
    std::cout << "Usage: yt-snip <command> [options]\n"
              << "Commands:\n"
              << "  video <url>                    Download full video\n"
              << "  audio <url>                    Download audio only\n"
              << "  playlist-video <url>           Download all videos from playlist\n"
              << "  playlist-audio <url>           Download all audio from playlist\n"
              << "  trim-video <url> <start> <end> Download and trim video\n"
              << "  trim-audio <url> <start> <end> Download and trim audio\n"
              << "  help                          Show this help\n"
              << "Time format: HH:MM:SS or seconds (e.g., 90)\n"
              << "\nSupported URLs:\n"
              << "  - YouTube (youtube.com, youtu.be)\n"
              << "  - YouTube Music (music.youtube.com)\n"
              << "\nOutput Organization:\n"
              << "  - Single files: saved in current directory\n"
              << "  - Playlists: ./[Playlist Name]/\n"
              << "  - Trimmed files: saved as [Video Title].mp4/.opus\n"
              << "\nPlaylist Features:\n"
              << "  - Creates folder named after playlist\n"
              << "  - Shows total item count before downloading\n"
              << "  - Real-time progress bar for each item\n"
              << "  - Clean output with download summary\n";
  }

  bool run(int argc, char *argv[]) {
    if (argc < 2) {
      printUsage();
      return false;
    }

    std::string command = argv[1];

    if (command == "help" || command == "-h" || command == "--help") {
      printUsage();
      return true;
    }

    if (command == "video" && argc >= 3) {
      std::string url = argv[2];
      return downloadMedia(url, "%(title)s", false);
    }

    if (command == "audio" && argc >= 3) {
      std::string url = argv[2];
      return downloadMedia(url, "%(title)s", true);
    }

    if (command == "playlist-video" && argc >= 3) {
      std::string url = argv[2];
      return downloadMedia(url, "%(title)s", false, true);
    }

    if (command == "playlist-audio" && argc >= 3) {
      std::string url = argv[2];
      return downloadMedia(url, "%(title)s", true, true);
    }

    if (command == "trim-video" && argc >= 5) {
      std::string url = argv[2];
      std::string startTime = argv[3];
      std::string endTime = argv[4];

      return trimAndDownload(url, startTime, endTime, false);
    }

    if (command == "trim-audio" && argc >= 5) {
      std::string url = argv[2];
      std::string startTime = argv[3];
      std::string endTime = argv[4];

      return trimAndDownload(url, startTime, endTime, true);
    }

    std::cerr << "Invalid command or missing arguments\n";
    printUsage();
    return false;
  }
};

int main(int argc, char *argv[]) {
  YouTubeDownloader downloader;
  return downloader.run(argc, argv) ? 0 : 1;
}
