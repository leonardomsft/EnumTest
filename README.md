# EnumTest

EnumTest is a directory enumeration test utility. It helps diagnose performance issues in directories with a very large number of files.

Windows API's FindFirstFile and FindNextFile are commonly used by applications to enumerate directory content. However, some applications also call other API's to obtain extended file information, which can cause poor enumeration performance.

For example, in addition to FindFirstFile/FindNextFile, SAP transaction AL11 also calls these API's against every returned file:

GetFileAttributes
GetFileType
GetFileSize
GetFileSecurity

If the enumeration takes 30 minutes or more, the operation times out and displays an error. Additionally, there is no progress bar or partial results, so the user never knows if it's going to be a successful or failed run.

EnumTest enables support engineers to quickly diagnose these types of issues by producing the same sequence of calls and outputing partial results.

## Usage

EnumTest.exe "directory name" "refresh rate in seconds" "test type"

Test Types:
  1 - Enumeration only
  2 - Enumeration + Atributes

## Example

![Alt text](screenshot1.png?raw=true "Image1")



