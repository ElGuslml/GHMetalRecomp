// Downloadable content (the DLC page, page_dlc.cpp): Xbox 360 content packages found in a folder the player chooses,
// installed by the game itself (its --install_content tool mode extracts a package into the user data's content
// folder, where the game's system software lists it), the ones installed, and removing one.

#pragma once

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace launcher {

// An Xbox 360 content package (an STFS file: LIVE, PIRS or CON), from its header.
struct DlcPackage {
  std::filesystem::path path;
  std::string file_name;     // the package's file name: the folder it is installed in
  std::string display_name;  // UTF-8; the file name when the package has none
  std::string title_name;    // the game it was made for, UTF-8 (may be empty)
  uint32_t title_id = 0;
  uint32_t content_type = 0;
  uint64_t content_size = 0;  // the data inside, about the installed size
  bool installed = false;     // a folder of its name is in the DLC folder already
  // Downloadable content (Marketplace content, type 2): the only kind the page installs.
  bool IsDlc() const { return content_type == 2; }
};

// Content installed for the game: a folder in the DLC folder, named after its package.
struct InstalledDlc {
  std::string folder;        // its name, UTF-8
  std::string display_name;  // from the header the game wrote next to it, else the folder's name
  uint64_t bytes = 0;
};

// The game's user data folder: userdata\ next to the game when it can be written there, else
// Documents\<game::kDocumentsFolder> (src/startup.cpp, ConfigureUserData).
std::filesystem::path GameUserDataFolder(const std::filesystem::path& exe_dir);
// Where the game's DLC is installed: <user data>\0000000000000000\<title ID>\00000002 (the SDK's ContentManager).
std::filesystem::path DlcFolder(const std::filesystem::path& exe_dir);

// The packages in `path` (a package itself, or a folder and its subfolders, a few levels deep), with `installed` set
// against the DLC folder. Reads a few KB of each file: run it off the UI thread.
std::vector<DlcPackage> FindDlcPackages(const std::filesystem::path& path, const std::filesystem::path& exe_dir);
// What is installed, by name. Sums the folders' sizes: run it off the UI thread.
std::vector<InstalledDlc> ListInstalledDlc(const std::filesystem::path& exe_dir);
// Removes an installed package's folder and its header.
bool RemoveDlc(const std::filesystem::path& exe_dir, const std::string& folder);

// Installing packages, one after another, on a thread of its own: the game in its --install_content tool mode for
// each (no window), its progress the installed folder's size against the package's content size.
class DlcInstall {
 public:
  enum class State { kIdle, kRunning, kDone };
  struct Progress {
    State state = State::kIdle;
    size_t index = 0;  // the package being installed
    size_t count = 0;
    std::string current;  // its name, UTF-8
    uint64_t done = 0;    // bytes of the packages so far
    uint64_t total = 0;
    size_t installed = 0;
    std::vector<std::string> failed;  // the names of those not installed
  };

  ~DlcInstall();
  void Start(const std::filesystem::path& exe_dir, std::vector<DlcPackage> packages);
  Progress Get() const;
  // Back to kIdle once its result has been shown.
  void Reset();

 private:
  void Run(std::filesystem::path exe_dir, std::vector<DlcPackage> packages);

  std::thread worker_;
  mutable std::mutex mutex_;
  Progress progress_;
};

}  // namespace launcher
