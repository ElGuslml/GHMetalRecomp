// The DLC page (dlc.h): a card to install packages from a folder the player chooses (or drops on the window), the
// packages found there, and the content installed, each with a button to remove it.

#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <string>
#include <vector>

#include <SDL3/SDL_dialog.h>

#include "app.h"
#include "game.h"
#include "pages.h"

namespace launcher {

using namespace ui;

namespace {

std::wstring Wide(const std::string& utf8) {
  int length = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
  std::wstring wide(size_t(std::max(length, 1)), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide.data(), length);
  wide.resize(wcslen(wide.c_str()));
  return wide;
}

// "1,4 GB", or "350 MB" below a gigabyte.
std::string SizeText(uint64_t bytes, const char* decimal_separator) {
  char text[32];
  if (bytes >= 1024ull * 1024 * 1024) {
    std::snprintf(text, sizeof(text), "%.1f GB", double(bytes) / (1024.0 * 1024.0 * 1024.0));
  } else {
    std::snprintf(text, sizeof(text), "%.0f MB", std::max(1.0, double(bytes) / (1024.0 * 1024.0)));
  }
  std::string out = text;
  std::replace(out.begin(), out.end(), '.', decimal_separator[0]);
  return out;
}

constexpr Type kCardTitle{Font::kSemiBold, 18};
constexpr Type kText{Font::kRegular, 14};
constexpr Type kName{Font::kSemiBold, 15};
constexpr Type kDetail{Font::kRegular, 13};

}  // namespace

void App::BrowseDlc() {
  // As BrowseIso: the dialog answers on a thread of its own, PollDlc takes the folder on the UI thread.
  auto* holder = new std::shared_ptr<DialogResult>(dlc_dialog_);
  SDL_ShowOpenFolderDialog(
      [](void* userdata, const char* const* folders, int) {
        auto* result = static_cast<std::shared_ptr<DialogResult>*>(userdata);
        if (folders && folders[0]) {
          std::lock_guard lock((*result)->mutex);
          (*result)->path = folders[0];
        }
        delete result;
      },
      holder, window_, nullptr, false);
}

void App::ScanDlc(const std::string& path) {
  if (dlc_scan_.valid() || dlc_install_.Get().state == DlcInstall::State::kRunning) {
    return;
  }
  dlc_scanned_ = path;
  dlc_found_.reset();
  dlc_install_.Reset();
  dlc_scan_ = std::async(std::launch::async, FindDlcPackages, std::filesystem::path(Wide(path)), exe_dir_);
}

void App::ListDlc() {
  dlc_listed_ = true;
  if (!dlc_listing_.valid()) {
    dlc_listing_ = std::async(std::launch::async, ListInstalledDlc, exe_dir_);
  }
}

void App::PollDlc() {
  {
    std::optional<std::string> chosen;
    {
      std::lock_guard lock(dlc_dialog_->mutex);
      chosen.swap(dlc_dialog_->path);
    }
    if (chosen) {
      ScanDlc(*chosen);
    }
  }
  if (dlc_scan_.valid() && dlc_scan_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    dlc_found_ = dlc_scan_.get();
  }
  if (dlc_listing_.valid() && dlc_listing_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    dlc_installed_ = dlc_listing_.get();
  }
  // An installation ended: what is installed now, and the packages found marked again.
  if (dlc_refresh_after_install_ && dlc_install_.Get().state == DlcInstall::State::kDone) {
    dlc_refresh_after_install_ = false;
    ListDlc();
    if (dlc_found_) {
      std::filesystem::path folder = DlcFolder(exe_dir_);
      std::error_code ec;
      for (DlcPackage& package : *dlc_found_) {
        package.installed = std::filesystem::is_directory(folder / package.path.filename(), ec);
      }
    }
  }
}

void App::DlcPage(const ImRect& rect) {
  if (!dlc_listed_) {
    ListDlc();
  }
  ImRect content = PageContent(rect);
  float header_bottom = Header(content.Min, nullptr, t_.dlc_title, t_.dlc_subtitle);
  ImRect area(ImVec2(content.Min.x, header_bottom + Px(20.0f)), ImVec2(rect.Max.x - Px(24.0f), content.Max.y));
  BeginScroll("dlc", area);
  ImVec2 origin = ImGui::GetCursorScreenPos();
  float width = std::min(content.GetWidth(), ImGui::GetContentRegionAvail().x - Px(16.0f));
  float y = origin.y;
  DlcInstall::Progress progress = dlc_install_.Get();
  bool running = progress.state == DlcInstall::State::kRunning;

  // Installing: the folder to search, and the DLC folder for copying by hand.
  {
    float x = origin.x + Px(28.0f);
    float text_x = x + Px(64.0f) + Px(24.0f);
    float text_width = origin.x + width - Px(28.0f) - text_x;
    float paragraph = DrawParagraph(kText, ImVec2(0, 0), text_width, 1.5f, 0, t_.dlc_install_text, false);
    float height = Px(28.0f) * 2.0f + LineHeight(kCardTitle) + Px(8.0f) + paragraph + Px(20.0f) + Px(44.0f);
    ImRect card(ImVec2(origin.x, y), ImVec2(origin.x + width, y + height));
    FillRect(card, col::kCard, 12.0f);
    ImRect icon(ImVec2(x, card.Min.y + Px(28.0f)), ImVec2(x + Px(64.0f), card.Min.y + Px(92.0f)));
    FillRect(icon, col::kAccentTint, 12.0f);
    DrawIcon(Icon::kPackage, ImVec2(icon.Min.x + Px(17.0f), icon.Min.y + Px(17.0f)), 30.0f, col::kAccent, 1.8f);
    float top = card.Min.y + Px(28.0f);
    DrawLabel(kCardTitle, ImVec2(text_x, top), col::kText, t_.dlc_install_title);
    top += LineHeight(kCardTitle) + Px(8.0f);
    DrawParagraph(kText, ImVec2(text_x, top), text_width, 1.5f, col::kTextMuted, t_.dlc_install_text);
    top += paragraph + Px(20.0f);
    ButtonSpec choose{ButtonKind::kPrimary, Type{Font::kBold, 15}, 44, 20, 8, Icon::kFolder, 18, 2};
    if (Button("dlc_choose", ImVec2(text_x, top), choose, t_.dlc_choose_folder, !running && !dlc_scan_.valid())) {
      BrowseDlc();
    }
    ButtonSpec open{ButtonKind::kOutline, Type{Font::kSemiBold, 15}, 44, 20, 8, Icon::kArrowRight, 18, 2};
    open.icon_after = true;
    if (Button("dlc_open", ImVec2(text_x + ButtonWidth(choose, t_.dlc_choose_folder) + Px(12.0f), top), open,
               t_.dlc_open_folder)) {
      std::filesystem::path folder = DlcFolder(exe_dir_);
      std::error_code ec;
      std::filesystem::create_directories(folder, ec);
      OpenFolder(folder);
    }
    y += height + Px(20.0f);
  }

  // The packages found in the folder chosen, and installing them.
  if (dlc_scan_.valid() || dlc_found_) {
    float x = origin.x + Px(28.0f);
    float inner = width - Px(56.0f);
    const std::vector<DlcPackage> none;
    const std::vector<DlcPackage>& found = dlc_found_ ? *dlc_found_ : none;
    std::vector<DlcPackage> to_install;
    for (const DlcPackage& package : found) {
      if (package.IsDlc() && !package.installed) {
        to_install.push_back(package);
      }
    }
    float row = Px(62.0f);
    bool done = progress.state == DlcInstall::State::kDone;
    // Below the rows: the progress while installing, else Install (or "everything is installed"), and after an
    // installation its result.
    float footer = 0;
    if (running) {
      footer = Px(16.0f) + LineHeight(kDetail) + Px(10.0f) + Px(8.0f);
    } else if (!found.empty()) {
      footer = Px(16.0f) + Px(44.0f);
    }
    int result_lines = done ? (progress.installed ? 1 : 0) + (progress.failed.empty() ? 0 : 1) : 0;
    if (result_lines) {
      footer += Px(12.0f) + LineHeight(kDetail) * float(result_lines);
    }
    float rows = found.empty() ? LineHeight(kText) : float(found.size()) * row;
    float height = Px(24.0f) * 2.0f + LineHeight(kCardTitle) + Px(12.0f) + rows + footer;
    ImRect card(ImVec2(origin.x, y), ImVec2(origin.x + width, y + height));
    FillRect(card, col::kCard, 12.0f);
    float top = card.Min.y + Px(24.0f);
    char title[1024];
    std::snprintf(title, sizeof(title), dlc_scan_.valid() ? t_.dlc_searching : t_.dlc_found, dlc_scanned_.c_str());
    DrawLabel(kCardTitle, ImVec2(x, top), col::kText, Ellipsize(kCardTitle, title, inner));
    top += LineHeight(kCardTitle) + Px(12.0f);
    if (dlc_found_ && found.empty()) {
      char text[1024];
      std::snprintf(text, sizeof(text), t_.dlc_none_found, dlc_scanned_.c_str());
      DrawLabel(kText, ImVec2(x, top), col::kTextMuted, Ellipsize(kText, text, inner));
    }
    for (const DlcPackage& package : found) {
      ImRect line(ImVec2(x, top), ImVec2(x + inner, top + row));
      FillRect(ImRect(ImVec2(line.Min.x, line.Max.y - Snap(Px(1.0f))), line.Max), col::kSidebarBorder);
      // Its state on the right: installed, ready, or not downloadable content.
      const char* state = !package.IsDlc() ? t_.dlc_not_dlc : package.installed ? t_.dlc_already_installed : t_.dlc_ready;
      float state_width = 0;
      if (package.IsDlc()) {
        ImVec2 size = BadgeSize(state, 11);
        state_width = size.x;
        Badge(ImVec2(line.Max.x - size.x, line.Min.y + (row - size.y) * 0.5f), state,
              package.installed ? col::kSuccessTint : col::kAccentTint,
              package.installed ? col::kSuccess : col::kAccentLight, 11);
      } else {
        state_width = std::min(MeasureText(kDetail, state).x, inner * 0.4f);
        DrawLabel(kDetail, ImVec2(line.Max.x - state_width, line.Min.y), col::kTextDim, Ellipsize(kDetail, state, state_width),
                  row);
      }
      float text_width = inner - state_width - Px(16.0f);
      float name_y = line.Min.y + (row - LineHeight(kName) - Px(4.0f) - LineHeight(kDetail)) * 0.5f;
      DrawLabel(kName, ImVec2(x, name_y), package.IsDlc() ? col::kText : col::kTextDim,
                Ellipsize(kName, package.display_name, text_width));
      std::string detail = SizeText(package.content_size, t_.decimal_separator);
      if (package.title_id != game::kTitleId && !package.title_name.empty()) {
        char made_for[256];
        std::snprintf(made_for, sizeof(made_for), t_.dlc_made_for, package.title_name.c_str());
        detail = std::string(made_for) + "  ·  " + detail;
      }
      DrawLabel(kDetail, ImVec2(x, name_y + LineHeight(kName) + Px(4.0f)), col::kTextMuted,
                Ellipsize(kDetail, detail, text_width));
      top += row;
    }
    if (found.empty()) {
      top += LineHeight(kText);
    } else {
      top += Px(16.0f);
    }
    if (running) {
      char text[512];
      std::snprintf(text, sizeof(text), t_.dlc_installing, progress.current.c_str(), int(progress.index + 1),
                    int(progress.count));
      DrawLabel(kDetail, ImVec2(x, top), col::kTextMuted, Ellipsize(kDetail, text, inner));
      top += LineHeight(kDetail) + Px(10.0f);
      Progress(ImRect(ImVec2(x, top), ImVec2(x + inner, top + Px(8.0f))),
               progress.total ? float(double(progress.done) / double(progress.total)) : 0.0f);
    } else if (dlc_found_ && !found.empty()) {
      if (to_install.empty()) {
        DrawLabel(kText, ImVec2(x, top), col::kTextMuted, t_.dlc_nothing_new, Px(44.0f));
      } else {
        char label[64];
        std::snprintf(label, sizeof(label), t_.dlc_install, int(to_install.size()));
        ButtonSpec install{ButtonKind::kPrimary, Type{Font::kBold, 15}, 44, 22, 8, Icon::kArrowDown, 18, 2};
        if (Button("dlc_install", ImVec2(x, top), install, label)) {
          dlc_install_.Start(exe_dir_, to_install);
          dlc_refresh_after_install_ = true;
        }
      }
    }
    if (result_lines) {
      top += Px(44.0f) + Px(12.0f);
      if (progress.installed) {
        char text[256];
        std::snprintf(text, sizeof(text), t_.dlc_done, int(progress.installed));
        DrawLabel(kDetail, ImVec2(x, top), col::kSuccess, Ellipsize(kDetail, text, inner));
        top += LineHeight(kDetail);
      }
      if (!progress.failed.empty()) {
        std::string names;
        for (const std::string& name : progress.failed) {
          names += (names.empty() ? "" : ", ") + name;
        }
        char text[1024];
        std::snprintf(text, sizeof(text), t_.dlc_failed, names.c_str());
        DrawLabel(kDetail, ImVec2(x, top), col::kError, Ellipsize(kDetail, text, inner));
      }
    }
    y += height + Px(20.0f);
  }

  InstalledDlcCard(ImVec2(origin.x, y), width);
  y = ImGui::GetCursorScreenPos().y;
  ImGui::SetCursorScreenPos(origin);
  ImGui::Dummy(ImVec2(width, y - origin.y));
  EndScroll();
}

// What is installed, each with Remove (pressed twice: the second confirms). Leaves the cursor below it.
void App::InstalledDlcCard(ImVec2 pos, float width) {
  float x = pos.x + Px(28.0f);
  float inner = width - Px(56.0f);
  const std::vector<InstalledDlc> none;
  const std::vector<InstalledDlc>& installed = dlc_installed_ ? *dlc_installed_ : none;
  const char* where = game::GameTextsFor(language_).dlc_where;
  bool show_where = where && !installed.empty();
  float row = Px(62.0f);
  float height = Px(24.0f) * 2.0f + LineHeight(kCardTitle) + Px(12.0f) +
                 (show_where ? LineHeight(kDetail) + Px(8.0f) : 0.0f) +
                 (installed.empty() ? LineHeight(kText) : float(installed.size()) * row);
  ImRect card(pos, ImVec2(pos.x + width, pos.y + height));
  FillRect(card, col::kCard, 12.0f);
  float top = card.Min.y + Px(24.0f);
  DrawLabel(kCardTitle, ImVec2(x, top), col::kText, t_.dlc_installed_title);
  top += LineHeight(kCardTitle) + Px(12.0f);
  if (show_where) {
    DrawLabel(kDetail, ImVec2(x, top - Px(4.0f)), col::kTextMuted, Ellipsize(kDetail, where, inner));
    top += LineHeight(kDetail) + Px(8.0f);
  }
  if (installed.empty()) {
    DrawLabel(kText, ImVec2(x, top), col::kTextMuted, dlc_listing_.valid() ? "…" : t_.dlc_installed_none);
  }
  bool running = dlc_install_.Get().state == DlcInstall::State::kRunning;
  std::string removed;
  for (size_t i = 0; i < installed.size(); ++i) {
    const InstalledDlc& dlc = installed[i];
    ImRect line(ImVec2(x, top), ImVec2(x + inner, top + row));
    if (i + 1 < installed.size()) {
      FillRect(ImRect(ImVec2(line.Min.x, line.Max.y - Snap(Px(1.0f))), line.Max), col::kSidebarBorder);
    }
    bool confirming = dlc_confirm_ == dlc.folder;
    ButtonSpec remove{confirming ? ButtonKind::kPrimary : ButtonKind::kOutline, Type{Font::kSemiBold, 14}, 38, 16, 8,
                      Icon::kTrash, 16, 2};
    const char* label = confirming ? t_.dlc_remove_confirm : t_.dlc_remove;
    float button_width = ButtonWidth(remove, label);
    ImGui::PushID(int(i));
    if (Button("dlc_remove", ImVec2(line.Max.x - button_width, line.Min.y + (row - Px(38.0f)) * 0.5f), remove, label,
               !running)) {
      if (confirming) {
        removed = dlc.folder;
      } else {
        dlc_confirm_ = dlc.folder;
      }
    }
    ImGui::PopID();
    float text_width = inner - button_width - Px(16.0f);
    float name_y = line.Min.y + (row - LineHeight(kName) - Px(4.0f) - LineHeight(kDetail)) * 0.5f;
    DrawLabel(kName, ImVec2(x, name_y), col::kText, Ellipsize(kName, dlc.display_name, text_width));
    DrawLabel(kDetail, ImVec2(x, name_y + LineHeight(kName) + Px(4.0f)), col::kTextMuted,
              SizeText(dlc.bytes, t_.decimal_separator));
    top += row;
  }
  if (!removed.empty()) {
    RemoveDlc(exe_dir_, removed);
    dlc_confirm_.clear();
    ListDlc();
    if (dlc_found_) {
      for (DlcPackage& package : *dlc_found_) {
        if (package.file_name == removed) {
          package.installed = false;
        }
      }
    }
  }
  ImGui::SetCursorScreenPos(ImVec2(pos.x, card.Max.y));
}

}  // namespace launcher
