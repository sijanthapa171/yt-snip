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
              << "  yt-snip trim-audio \"https://youtube.com/watch?v=VIDEO_ID\" \"90\" \"180\"\n"
              << "  yt-snip playlist-audio \"https://music.youtube.com/playlist?list=PLAYLIST_ID\"\n";
}

bool isPlaylistUrl(const std::string& url) {
    return url.find("playlist?") != std::string::npos;
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

        // Check if URL is a playlist and correct the command if needed
        std::string urlStr(url);
        bool isPlaylist = isPlaylistUrl(urlStr);

        if (command == "video") {
            if (isPlaylist) {
                std::cout << "URL is a playlist, using playlist-video command instead\n";
                success = downloader.downloadPlaylistVideo(urlStr);
            } else {
                success = downloader.downloadVideo(urlStr);
            }
        }
        else if (command == "audio") {
            if (isPlaylist) {
                std::cout << "URL is a playlist, using playlist-audio command instead\n";
                success = downloader.downloadPlaylistAudio(urlStr);
            } else {
                success = downloader.downloadAudio(urlStr);
            }
        }
        else if (command == "playlist-video") {
            if (!isPlaylist) {
                std::cerr << "Error: Not a playlist URL\n";
                return 1;
            }
            success = downloader.downloadPlaylistVideo(urlStr);
        }
        else if (command == "playlist-audio") {
            if (!isPlaylist) {
                std::cerr << "Error: Not a playlist URL\n";
                return 1;
            }
            success = downloader.downloadPlaylistAudio(urlStr);
        }
        else if (command == "trim-video" && argc == 5) {
            if (isPlaylist) {
                std::cerr << "Error: Cannot trim a playlist URL\n";
                return 1;
            }
            success = downloader.trimVideo(urlStr, argv[3], argv[4]);
        }
        else if (command == "trim-audio" && argc == 5) {
            if (isPlaylist) {
                std::cerr << "Error: Cannot trim a playlist URL\n";
                return 1;
            }
            success = downloader.trimAudio(urlStr, argv[3], argv[4]);
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