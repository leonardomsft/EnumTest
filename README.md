# EnumTest

EnumTest is a directory enumeration test utility. It helps diagnose performance issues in directories with a very large number of files/folders.

Windows API's FindFirstFile and FindNextFile are commonly used by applications to enumerate directory content. However, some applications also call other API's to obtain extended file information, which can cause poor enumeration performance. As an example, SAP transaction AL11 calls GetFileAttributes, GetFileType, GetFileSize, and GetFileSecurity against every file returned by FindFirstFile and FindNextFile. If the enumeration takes 30 minutes or more, the operation times out and displays an error. There is no progress bar or partial results, so the user never knows if it's going to succeed or fail.

EnumTest enables support engineers to quickly diagnose these types of issues by producing the same sequence of calls and outputing partial results, so it can identify exactly where the problem is.

## Usage

EnumTest.exe &lt;path&gt; &lt;options&gt;

Examples:

EnumTest.exe \\SERVER01\Docs\

EnumTest.exe C:\Temp\Docs -recurse

EnumTest.exe C:\Temp\Docs -recurse -extended

EnumTest.exe C:\Temp\Docs -recurse -extended -refresh 10


## Example

![Alt text](screenshot1.png?raw=true "Image1")



