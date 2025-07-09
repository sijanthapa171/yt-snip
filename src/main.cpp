#include "yt-snip/downloader.hpp"
#include <iostream>
#include <string>
#include <string_view>

void printUsage() {
    std::cout << "Usage:\n"
              << "  yt-snip video <url>                     # Download full video\n"
              << "  yt-snip audio <url>                     # Download audio only\n"
              << "  yt-snip playlist-video <url>            # Download all videos from a playlist\n"
              << "  yt-snip playlist-audio <url>            # Download all audio from a playlist\n"
              << "  yt-snip trim-video <url> <start> <end>  # Download and trim video\n"
              << "  yt-snip trim-audio <url> <start> <end>  # Download and trim audio\n"
              << "\nExamples:\n"
              << "  yt-snip video \"https://youtube.com/watch?v=VIDEO_ID\"\n"
              << "  yt-snip trim-video \"https://youtube.com/watch?v=VIDEO_ID\" \"00:01:30\" \"00:03:45\"\n"
              << "  yt-snip trim-audio \"https://youtube.com/watch?v=VIDEO_ID\" \"90\" \"180\"\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    std::string_view command = argv[1];
    std::string_view url = argv[2];
    
    yt_snip::YouTubeDownloader downloader;

    try {
        bool success = false;

        if (command == "video") {
            success = downloader.downloadVideo(std::string(url));
        }
        else if (command == "audio") {
            success = downloader.downloadAudio(std::string(url));
        }
        else if (command == "playlist-video") {
            success = downloader.downloadPlaylistVideo(std::string(url));
        }
        else if (command == "playlist-audio") {
            success = downloader.downloadPlaylistAudio(std::string(url));
        }
        else if (command == "trim-video" && argc == 5) {
            success = downloader.trimVideo(std::string(url), argv[3], argv[4]);
        }
        else if (command == "trim-audio" && argc == 5) {
            success = downloader.trimAudio(std::string(url), argv[3], argv[4]);
        }
        else {
            printUsage();
            return 1;
        }

        return success ? 0 : 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 