# version-download-tool-cpp

一个基于 **C++ / Qt6** 的版本下载工具，支持扫描 HTTP/HTTPS 目录索引并批量下载文件。

## 功能特性

- 递归扫描远程目录，自动发现子目录文件。
- 过滤无效链接（如 `../`、排序链接、空链接、外部链接）。
- 表格展示序号、相对路径、文件大小、状态和下载进度。
- 支持跳过已存在且大小一致的文件。
- 下载时保持远程目录结构。
- 支持停止下载、失败状态标记和日志输出。
- 使用 QSettings 保存远程地址与本地目录。

## 构建方法

### 依赖
- CMake >= 3.21
- Qt6（Widgets、Network）
- C++17 编译器

### 本地构建

```bash
cmake -B build -S .
cmake --build build --config Release
```

## 使用方法

1. 输入远程版本目录 URL（HTTP/HTTPS）。
2. 选择本地保存目录。
3. 点击“扫描”，确认文件列表。
4. 点击“开始下载”。
5. 需要中止时点击“停止下载”。

## 测试与验收

详见：
- [docs/TESTING.md](docs/TESTING.md)

## 发布方法（Windows EXE）

详见：
- [docs/RELEASE.md](docs/RELEASE.md)

仓库已提供 GitHub Actions 工作流，在推送 `v*` tag 后自动构建并发布 `VersionDownloadTool-win64.zip`。
