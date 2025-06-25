#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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
      if (std::filesystem::exists(file)) {
        std::filesystem::remove(file);
      }
    }
  }

  bool downloadMedia(const std::string &url, const std::string &outputName, bool audioOnly = false) {
    std::string command = "yt-dlp -o " + outputName + " ";
    if (audioOnly) {
      command += "-x ";
    }
    command += "\"" + url + "\"";

    int result = executeCommand(command);
    if (result != 0) {
      std::cerr << "Error downloading " << (audioOnly ? "audio" : "video") << std::endl;
      return false;
    }
    return true;
  }

  bool trimMedia(const std::string &inputFile, const std::string &outputFile,
                 const std::string &startTime, const std::string &endTime, bool isAudio = false) {
    std::string command = "ffmpeg ";

    if (isAudio) {
      command += "-ss " + startTime + " -to " + endTime + " -i " + inputFile + " " + outputFile;
    } else {
      command += "-i " + inputFile + " -ss " + startTime + " -to " + endTime +
                 " -c:v copy -c:a copy " + outputFile;
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
              << "  trim-video <url> <start> <end> Download and trim video\n"
              << "  trim-audio <url> <start> <end> Download and trim audio\n"
              << "  help                          Show this help\n"
              << "Time format: HH:MM:SS or seconds (e.g., 90)\n";
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
      return downloadMedia(url, "output", false);
    }

    if (command == "audio" && argc >= 3) {
      std::string url = argv[2];
      return downloadMedia(url, "output", true);
    }

    if (command == "trim-video" && argc >= 5) {
      std::string url = argv[2];
      std::string startTime = argv[3];
      std::string endTime = argv[4];

      if (!downloadMedia(url, "temp_video", false)) {
        return false;
      }

      bool success = trimMedia("temp_video.webm", "output.webm", startTime, endTime, false);
      cleanupFiles({"temp_video.webm"});
      return success;
    }

    if (command == "trim-audio" && argc >= 5) {
      std::string url = argv[2];
      std::string startTime = argv[3];
      std::string endTime = argv[4];

      if (!downloadMedia(url, "temp_audio", true)) {
        return false;
      }

      bool success = trimMedia("temp_audio.opus", "output.opus", startTime, endTime, true);
      cleanupFiles({"temp_audio.opus"});
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
