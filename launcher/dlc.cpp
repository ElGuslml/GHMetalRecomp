// Downloadable content: see dlc.h.

#include "dlc.h"

#include <windows.h>
#include <shlobj.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <fstream>

#include "game.h"

namespace launcher {

namespace {

// An STFS header's fields (Free60's layout; the SDK's StfsHeader): all big-endian.
constexpr size_t kContentTypeOffset = 0x344;
constexpr size_t kContentSizeOffset = 0x34C;
constexpr size_t kTitleIdOffset = 0x360;
constexpr size_t kDisplayNameOffset = 0x411;  // 18 languages of 0x80 bytes, UTF-16, English first
constexpr size_t kTitleNameOffset = 0x1691;   // 0x80 bytes, UTF-16
constexpr size_t kNameBytes = 0x80;
constexpr size_t kHeaderBytes = kTitleNameOffset + kNameBytes;
constexpr size_t kLanguages = 18;
// A folder is searched this deep, and this many entries at most (a drive chosen by mistake).
constexpr int kMaxDepth = 5;
constexpr int kMaxEntries = 20000;

uint32_t ReadU32(const uint8_t* p) { return uint32_t(p[0]) << 24 | uint32_t(p[1]) << 16 | uint32_t(p[2]) << 8 | p[3]; }

uint64_t ReadU64(const uint8_t* p) { return uint64_t(ReadU32(p)) << 32 | ReadU32(p + 4); }

std::string Utf8(const std::wstring& wide) {
  if (wide.empty()) return {};
  int length = WideCharToMultiByte(CP_UTF8, 0, wide.data(), int(wide.size()), nullptr, 0, nullptr, nullptr);
  std::string text(size_t(std::max(length, 0)), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wide.data(), int(wide.size()), text.data(), length, nullptr, nullptr);
  return text;
}

// A big-endian UTF-16 string of at most `bytes`, up to its terminator.
std::string Utf16BeToUtf8(const uint8_t* p, size_t bytes) {
  std::wstring wide;
  for (size_t i = 0; i + 1 < bytes; i += 2) {
    wchar_t c = wchar_t(p[i] << 8 | p[i + 1]);
    if (!c) break;
    wide.push_back(c);
  }
  // Names padded with spaces.
  while (!wide.empty() && wide.back() == L' ') wide.pop_back();
  return Utf8(wide);
}

bool ReadPackage(const std::filesystem::path& path, DlcPackage& out) {
  std::error_code ec;
  if (!std::filesystem::is_regular_file(path, ec) || std::filesystem::file_size(path, ec) < kHeaderBytes) {
    return false;
  }
  std::array<uint8_t, kHeaderBytes> header{};
  {
    std::ifstream file(path, std::ios::binary);
    if (!file.read(reinterpret_cast<char*>(header.data()), header.size())) {
      return false;
    }
  }
  if (std::memcmp(header.data(), "LIVE", 4) && std::memcmp(header.data(), "PIRS", 4) &&
      std::memcmp(header.data(), "CON ", 4)) {
    return false;
  }
  out.path = path;
  out.file_name = Utf8(path.filename().wstring());
  out.content_type = ReadU32(&header[kContentTypeOffset]);
  out.content_size = ReadU64(&header[kContentSizeOffset]);
  out.title_id = ReadU32(&header[kTitleIdOffset]);
  for (size_t i = 0; i < kLanguages && out.display_name.empty(); ++i) {
    out.display_name = Utf16BeToUtf8(&header[kDisplayNameOffset + i * kNameBytes], kNameBytes);
  }
  if (out.display_name.empty()) {
    out.display_name = out.file_name;
  }
  out.title_name = Utf16BeToUtf8(&header[kTitleNameOffset], kNameBytes);
  return true;
}

uint64_t FolderBytes(const std::filesystem::path& folder) {
  uint64_t bytes = 0;
  std::error_code ec;
  for (auto it = std::filesystem::recursive_directory_iterator(
           folder, std::filesystem::directory_options::skip_permission_denied, ec);
       !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
    std::error_code size_ec;
    if (it->is_regular_file(size_ec)) {
      bytes += it->file_size(size_ec);
    }
  }
  return bytes;
}

// The header the game writes next to an installed package, last (ContentManager::WriteContentHeaderFile):
// <title folder>\Headers\00000002\<package>.header, an XCONTENT_AGGREGATE_DATA.
std::filesystem::path HeaderPath(const std::filesystem::path& exe_dir, const std::wstring& folder) {
  return DlcFolder(exe_dir).parent_path() / L"Headers" / L"00000002" / (folder + L".header");
}

}  // namespace

std::filesystem::path GameUserDataFolder(const std::filesystem::path& exe_dir) {
  std::filesystem::path portable = exe_dir / L"userdata";
  std::error_code ec;
  std::filesystem::create_directories(portable, ec);
  std::filesystem::path probe = portable / L".write_test";
  bool writable = false;
  {
    std::ofstream file(probe, std::ios::binary);
    writable = bool(file);
  }
  std::filesystem::remove(probe, ec);
  if (writable) {
    return portable;
  }
  PWSTR documents = nullptr;
  std::filesystem::path result = portable;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &documents))) {
    result = std::filesystem::path(documents) / game::kDocumentsFolder;
  }
  CoTaskMemFree(documents);
  return result;
}

std::filesystem::path DlcFolder(const std::filesystem::path& exe_dir) {
  wchar_t title[16];
  std::swprintf(title, std::size(title), L"%08X", game::kTitleId);
  return GameUserDataFolder(exe_dir) / L"0000000000000000" / title / L"00000002";
}

std::vector<DlcPackage> FindDlcPackages(const std::filesystem::path& path, const std::filesystem::path& exe_dir) {
  std::vector<DlcPackage> result;
  std::error_code ec;
  DlcPackage package;
  if (std::filesystem::is_regular_file(path, ec)) {
    if (ReadPackage(path, package)) {
      result.push_back(package);
    }
  } else {
    int entries = 0;
    for (auto it = std::filesystem::recursive_directory_iterator(
             path, std::filesystem::directory_options::skip_permission_denied, ec);
         !ec && it != std::filesystem::recursive_directory_iterator() && entries < kMaxEntries; it.increment(ec)) {
      ++entries;
      if (it.depth() >= kMaxDepth) {
        it.disable_recursion_pending();
      }
      if (ReadPackage(it->path(), package)) {
        result.push_back(package);
        package = {};
      }
    }
  }
  std::filesystem::path installed = DlcFolder(exe_dir);
  for (DlcPackage& p : result) {
    p.installed = std::filesystem::is_directory(installed / p.path.filename(), ec);
  }
  std::sort(result.begin(), result.end(),
            [](const DlcPackage& a, const DlcPackage& b) { return a.display_name < b.display_name; });
  return result;
}

std::vector<InstalledDlc> ListInstalledDlc(const std::filesystem::path& exe_dir) {
  std::vector<InstalledDlc> result;
  std::error_code ec;
  for (const auto& entry : std::filesystem::directory_iterator(DlcFolder(exe_dir), ec)) {
    std::error_code dir_ec;
    if (!entry.is_directory(dir_ec)) {
      continue;
    }
    InstalledDlc dlc;
    std::wstring folder = entry.path().filename().wstring();
    dlc.folder = Utf8(folder);
    dlc.display_name = dlc.folder;
    // The header: an XCONTENT_DATA (device ID, content type, then the display name, 128 UTF-16 characters).
    std::array<uint8_t, 8 + 256> header{};
    std::ifstream file(HeaderPath(exe_dir, folder), std::ios::binary);
    if (file.read(reinterpret_cast<char*>(header.data()), header.size())) {
      std::string name = Utf16BeToUtf8(&header[8], 256);
      if (!name.empty()) {
        dlc.display_name = name;
      }
    }
    dlc.bytes = FolderBytes(entry.path());
    result.push_back(std::move(dlc));
  }
  std::sort(result.begin(), result.end(),
            [](const InstalledDlc& a, const InstalledDlc& b) { return a.display_name < b.display_name; });
  return result;
}

bool RemoveDlc(const std::filesystem::path& exe_dir, const std::string& folder) {
  int length = MultiByteToWideChar(CP_UTF8, 0, folder.c_str(), -1, nullptr, 0);
  std::wstring name(size_t(std::max(length, 1)), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, folder.c_str(), -1, name.data(), length);
  name.resize(wcslen(name.c_str()));
  // Only a folder directly in the DLC folder.
  if (name.empty() || name == L"." || name == L".." || name.find_first_of(L"\\/:") != std::wstring::npos) {
    return false;
  }
  std::error_code ec;
  std::filesystem::remove_all(DlcFolder(exe_dir) / name, ec);
  bool removed = !ec;
  std::filesystem::remove(HeaderPath(exe_dir, name), ec);
  return removed;
}

DlcInstall::~DlcInstall() {
  if (worker_.joinable()) {
    worker_.join();
  }
}

void DlcInstall::Start(const std::filesystem::path& exe_dir, std::vector<DlcPackage> packages) {
  if (worker_.joinable()) {
    worker_.join();
  }
  {
    std::lock_guard lock(mutex_);
    progress_ = {};
    progress_.state = State::kRunning;
    progress_.count = packages.size();
    for (const DlcPackage& p : packages) {
      progress_.total += p.content_size;
    }
  }
  worker_ = std::thread(&DlcInstall::Run, this, exe_dir, std::move(packages));
}

DlcInstall::Progress DlcInstall::Get() const {
  std::lock_guard lock(mutex_);
  return progress_;
}

void DlcInstall::Reset() {
  std::lock_guard lock(mutex_);
  if (progress_.state == State::kDone) {
    progress_ = {};
  }
}

void DlcInstall::Run(std::filesystem::path exe_dir, std::vector<DlcPackage> packages) {
  std::filesystem::path game = exe_dir / game::kRecompExecutable;
  std::filesystem::path dlc_folder = DlcFolder(exe_dir);
  uint64_t base = 0;
  for (size_t i = 0; i < packages.size(); ++i) {
    const DlcPackage& package = packages[i];
    {
      std::lock_guard lock(mutex_);
      progress_.index = i;
      progress_.current = package.display_name;
      progress_.done = base;
    }
    std::wstring folder = package.path.filename().wstring();
    std::filesystem::path header = HeaderPath(exe_dir, folder);
    std::error_code ec;
    std::filesystem::remove(header, ec);  // written last: its presence afterwards is the success
    std::wstring command_line =
        L"\"" + game.wstring() + L"\" \"--install_content=" + package.path.wstring() + L"\"";
    STARTUPINFOW startup = {sizeof(startup)};
    PROCESS_INFORMATION process = {};
    bool ok = false;
    if (CreateProcessW(game.c_str(), command_line.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                       exe_dir.c_str(), &startup, &process)) {
      CloseHandle(process.hThread);
      // The game extracts the package: its folder grows towards the package's content size. A package of a few GB
      // takes a minute or two on a hard disk; one that has not finished in 30 minutes has failed.
      auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(30);
      while (WaitForSingleObject(process.hProcess, 250) == WAIT_TIMEOUT) {
        if (std::chrono::steady_clock::now() > deadline) {
          TerminateProcess(process.hProcess, 1);
          WaitForSingleObject(process.hProcess, 5000);
          break;
        }
        uint64_t bytes = std::min(FolderBytes(dlc_folder / folder), package.content_size);
        std::lock_guard lock(mutex_);
        progress_.done = base + bytes;
      }
      DWORD exit_code = 1;
      GetExitCodeProcess(process.hProcess, &exit_code);
      CloseHandle(process.hProcess);
      ok = exit_code == 0 && std::filesystem::exists(header, ec);
    }
    base += package.content_size;
    std::lock_guard lock(mutex_);
    progress_.done = base;
    if (ok) {
      ++progress_.installed;
    } else {
      progress_.failed.push_back(package.display_name);
    }
  }
  std::lock_guard lock(mutex_);
  progress_.state = State::kDone;
}

}  // namespace launcher
