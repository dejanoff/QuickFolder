#include "Common.h"
#include "PathUtils.h"
#include "SelectionHelper.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <stdexcept>

using namespace QuickFolder;

static int g_passCount = 0;
static int g_failCount = 0;

#define RUN_TEST(fn) \
    do { \
        std::cout << "Running " #fn "... " << std::flush; \
        try { \
            fn(); \
            std::cout << "[PASS]\n"; \
            g_passCount++; \
        } catch (const std::exception& ex) { \
            std::cout << "[FAIL]: " << ex.what() << "\n"; \
            g_failCount++; \
        } catch (...) { \
            std::cout << "[FAIL]: Unknown exception\n"; \
            g_failCount++; \
        } \
    } while (0)

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        throw std::runtime_error("Assertion failed: " #cond); \
    }

#define ASSERT_EQUAL(a, b) \
    if (!((a) == (b))) { \
        throw std::runtime_error("Assertion failed: " #a " == " #b); \
    }

void TestValidFolderName() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photos") == PathUtils::ValidationResult::Valid);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Birthday 2026") == PathUtils::ValidationResult::Valid);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"My-Documents_Archive") == PathUtils::ValidationResult::Valid);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Summer (Vacation)") == PathUtils::ValidationResult::Valid);
}

void TestInvalidCharacters() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo/Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo\\Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo:Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo*Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo?Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo\"Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo<Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo>Album") == PathUtils::ValidationResult::InvalidCharacters);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photo|Album") == PathUtils::ValidationResult::InvalidCharacters);

    // Control characters (0x01 to 0x1F)
    std::wstring withCtrl = L"Bad\x07Name";
    ASSERT_TRUE(PathUtils::ValidateFolderName(withCtrl) == PathUtils::ValidationResult::InvalidCharacters);
}

void TestReservedDeviceName() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"CON") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"con") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"PRN") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"aux") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"NUL") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"COM1") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"com9") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"LPT1") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"lpt9") == PathUtils::ValidationResult::ReservedName);

    // Reserved names with extension (Windows forbids these as directory names)
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"CON.txt") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"aux.backup") == PathUtils::ValidationResult::ReservedName);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"prn.pdf") == PathUtils::ValidationResult::ReservedName);
}

void TestTrailingDot() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"MyFolder.") == PathUtils::ValidationResult::TrailingDotOrSpace);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Test...") == PathUtils::ValidationResult::TrailingDotOrSpace);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L".") == PathUtils::ValidationResult::DotOrDotDot);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"..") == PathUtils::ValidationResult::DotOrDotDot);
}

void TestTrailingSpace() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"MyFolder ") == PathUtils::ValidationResult::TrailingDotOrSpace);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Photos   ") == PathUtils::ValidationResult::TrailingDotOrSpace);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"   ") == PathUtils::ValidationResult::Empty);
}

void TestUnicode() {
    // Russian
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"День рождения") == PathUtils::ValidationResult::Valid);
    // Ukrainian
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"День народження") == PathUtils::ValidationResult::Valid);
    // German
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Geburtstagsfotos für Jürgen") == PathUtils::ValidationResult::Valid);
    // Polish
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"Łódź wycieczka") == PathUtils::ValidationResult::Valid);
    // Japanese
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"写真フォルダ") == PathUtils::ValidationResult::Valid);
    // Chinese
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"生日照片") == PathUtils::ValidationResult::Valid);
    // Arabic
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"صور الرحلة") == PathUtils::ValidationResult::Valid);
}

void TestCombiningUnicode() {
    // Precomposed vs Combining marks (e.g. e + combining acute accent U+0301)
    std::wstring combining = L"e\u0301cole";
    ASSERT_TRUE(PathUtils::ValidateFolderName(combining) == PathUtils::ValidationResult::Valid);
}

void TestEmoji() {
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"🎂 Birthday") == PathUtils::ValidationResult::Valid);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"📸 Vacation") == PathUtils::ValidationResult::Valid);
    ASSERT_TRUE(PathUtils::ValidateFolderName(L"🚀 Project") == PathUtils::ValidationResult::Valid);
}

void TestCommonParent() {
    std::wstring p1 = L"D:\\Photos\\IMG_001.jpg";
    std::wstring p2 = L"D:\\Photos\\IMG_002.jpg";
    std::wstring p3 = L"D:\\Photos\\SubFolder";

    std::wstring parent1 = PathUtils::GetParentDirectory(p1);
    std::wstring parent2 = PathUtils::GetParentDirectory(p2);
    std::wstring parent3 = PathUtils::GetParentDirectory(p3);

    ASSERT_TRUE(PathUtils::ArePathsEqual(parent1, L"D:\\Photos"));
    ASSERT_TRUE(PathUtils::ArePathsEqual(parent2, L"D:\\Photos"));
    ASSERT_TRUE(PathUtils::ArePathsEqual(parent3, L"D:\\Photos"));
}

void TestDifferentParents() {
    std::wstring p1 = L"D:\\Photos\\IMG_001.jpg";
    std::wstring p2 = L"C:\\Users\\Dejan\\Documents\\file.txt";

    std::wstring parent1 = PathUtils::GetParentDirectory(p1);
    std::wstring parent2 = PathUtils::GetParentDirectory(p2);

    ASSERT_TRUE(!PathUtils::ArePathsEqual(parent1, parent2));
}

void TestDestinationInsideSelection() {
    std::wstring selectedFolder = L"D:\\Photos\\Birthday";
    std::wstring insideFolder = L"D:\\Photos\\Birthday\\SubFolder";
    std::wstring sameFolder = L"D:\\Photos\\Birthday";
    std::wstring siblingFolder = L"D:\\Photos\\Birthday2";

    ASSERT_TRUE(PathUtils::IsSubdirectoryOf(insideFolder, selectedFolder));
    ASSERT_TRUE(PathUtils::IsSubdirectoryOf(sameFolder, selectedFolder));
    ASSERT_TRUE(!PathUtils::IsSubdirectoryOf(siblingFolder, selectedFolder));
}

void TestExistingDestination() {
    wchar_t winDir[MAX_PATH];
    GetWindowsDirectoryW(winDir, MAX_PATH);

    ASSERT_TRUE(PathUtils::PathExists(winDir));
    ASSERT_TRUE(!PathUtils::PathExists(L"C:\\NonExistentDirectory_123456789_QuickFolder"));
}

void TestAbsolutePathConstruction() {
    std::wstring combined = PathUtils::CombinePath(L"D:\\Photos", L"Birthday 2026");
    ASSERT_TRUE(PathUtils::ArePathsEqual(combined, L"D:\\Photos\\Birthday 2026"));

    std::wstring rootCombined = PathUtils::CombinePath(L"C:\\", L"Test");
    ASSERT_TRUE(PathUtils::ArePathsEqual(rootCombined, L"C:\\Test"));

    // Suggestion logic: single file vs single folder
    std::wstring fileSuggest = PathUtils::SuggestFolderNameForSingleItem(L"D:\\Photos\\IMG_1234.jpg", false);
    ASSERT_TRUE(fileSuggest == L"IMG_1234");

    std::wstring dirSuggest = PathUtils::SuggestFolderNameForSingleItem(L"D:\\Photos\\Holiday Photos", true);
    ASSERT_TRUE(dirSuggest == L"Holiday Photos - Folder");
}

void TestLongPath() {
    std::wstring longBase = L"D:\\A_Very_Long_Directory_Name_For_Testing_Path_Limits_In_QuickFolder_2026";
    while (longBase.length() < 300) {
        longBase += L"\\Nested_Level_Component";
    }

    std::wstring prefixed = PathUtils::EnsureLongPathPrefix(longBase);
    ASSERT_TRUE(prefixed.length() > 300);
    ASSERT_TRUE(prefixed.substr(0, 4) == L"\\\\?\\");

    // Verify UNC long path
    std::wstring unc = L"\\\\server\\share\\longfolder";
    std::wstring uncPrefixed = PathUtils::EnsureLongPathPrefix(unc);
    ASSERT_TRUE(uncPrefixed.substr(0, 8) == L"\\\\?\\UNC\\");
}

void TestShortcutTreatedAsFilesystemObject() {
    std::wstring lnkPath = L"C:\\Users\\Public\\Desktop\\Application.lnk";
    std::wstring urlPath = L"C:\\Users\\Public\\Desktop\\Website.url";

    ASSERT_TRUE(PathUtils::GetFileName(lnkPath) == L"Application.lnk");
    ASSERT_TRUE(PathUtils::GetFileName(urlPath) == L"Website.url");

    std::wstring lnkSuggest = PathUtils::SuggestFolderNameForSingleItem(lnkPath, false);
    ASSERT_TRUE(lnkSuggest == L"Application");
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "=========================================\n";
    std::cout << " QuickFolder Automated Unit Test Suite\n";
    std::cout << "=========================================\n";

    RUN_TEST(TestValidFolderName);
    RUN_TEST(TestInvalidCharacters);
    RUN_TEST(TestReservedDeviceName);
    RUN_TEST(TestTrailingDot);
    RUN_TEST(TestTrailingSpace);
    RUN_TEST(TestUnicode);
    RUN_TEST(TestCombiningUnicode);
    RUN_TEST(TestEmoji);
    RUN_TEST(TestCommonParent);
    RUN_TEST(TestDifferentParents);
    RUN_TEST(TestDestinationInsideSelection);
    RUN_TEST(TestExistingDestination);
    RUN_TEST(TestAbsolutePathConstruction);
    RUN_TEST(TestLongPath);
    RUN_TEST(TestShortcutTreatedAsFilesystemObject);

    std::cout << "=========================================\n";
    std::cout << " Results: " << g_passCount << " Passed, " << g_failCount << " Failed\n";
    std::cout << "=========================================\n";

    return (g_failCount == 0) ? 0 : 1;
}
