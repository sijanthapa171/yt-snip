#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <regex>

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

  std::string getOutputPath(bool audioOnly, bool isPlaylist, const std::string &filename) {
    std::string baseDir = audioOnly ? "audio" : "video";
    std::string subDir = isPlaylist ? "playlists" : "singles";
    std::string fullPath = baseDir + "/" + subDir;
    
    ensureDirectoryExists(fullPath);
    return fullPath + "/" + filename;
  }

  bool downloadMedia(const std::string &url, const std::string &outputName, bool audioOnly = false, bool isPlaylist = false) {
    // Convert YouTube Music URLs to regular YouTube URLs
    std::string processedUrl = convertYouTubeMusicURL(url);
    
    // Prepare output path
    std::string outputPath;
    if (isPlaylist) {
      outputPath = getOutputPath(audioOnly, true, outputName + "%(playlist_index)s.%(ext)s");
    } else {
      outputPath = getOutputPath(audioOnly, false, outputName + ".%(ext)s");
    }

    std::string command = "yt-dlp -o \"" + outputPath + "\" ";
    if (audioOnly) {
      command += "-x --audio-format opus ";
    }
    if (isPlaylist) {
      // Add playlist options to ensure proper handling
      command += "--yes-playlist --ignore-errors --no-abort-on-error ";
    } else {
      // For single videos/songs, disable playlist handling
      command += "--no-playlist ";
    }
    command += "\"" + processedUrl + "\"";

    int result = executeCommand(command);
    if (result != 0) {
      std::cerr << "Error downloading " << (audioOnly ? "audio" : "video") << std::endl;
      return false;
    }
    return true;
  }

  bool trimMedia(const std::string &inputFile, const std::string &outputFile,
                 const std::string &startTime, const std::string &endTime, bool isAudio = false) {
    std::string outputPath = getOutputPath(isAudio, false, outputFile);
    std::string command = "ffmpeg ";

    if (isAudio) {
      command += "-ss " + startTime + " -to " + endTime + " -i " + inputFile + " \"" + outputPath + "\"";
    } else {
      command += "-i " + inputFile + " -ss " + startTime + " -to " + endTime +
                 " -c:v copy -c:a copy \"" + outputPath + "\"";
    }

    int result = executeCommand(command);
    if (result != 0) {
      std::cerr << "Error trimming " << (isAudio ? "audio" : "video") << std::endl;
      return false;
    }
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
              << "  - Single videos: ./video/singles/\n"
              << "  - Video playlists: ./video/playlists/\n"
              << "  - Single audio: ./audio/singles/\n"
              << "  - Audio playlists: ./audio/playlists/\n";
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

      std::string tempFile = "temp_video.webm";
      if (!downloadMedia(url, "temp_video", false)) {
        return false;
      }

      bool success = trimMedia(tempFile, "trimmed_%(title)s.webm", startTime, endTime, false);
      cleanupFiles({tempFile});
      return success;
    }

    if (command == "trim-audio" && argc >= 5) {
      std::string url = argv[2];
      std::string startTime = argv[3];
      std::string endTime = argv[4];

      std::string tempFile = "temp_audio.opus";
      if (!downloadMedia(url, "temp_audio", true)) {
        return false;
      }

      bool success = trimMedia(tempFile, "trimmed_%(title)s.opus", startTime, endTime, true);
      cleanupFiles({tempFile});
      return success;
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
