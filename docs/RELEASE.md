# RELEASE

## 前置条件
- 已在默认分支合并完成。
- `.github/workflows/release.yml` 保持可用。

## 打 tag 触发 Windows EXE 发布

```bash
git checkout main
git pull

git tag v0.1.0
git push origin v0.1.0
```

> 约定：tag 需匹配 `v*` 才会触发发布流水线。

## GitHub Actions 行为
`Build and Release Windows Package` 会自动执行：
1. 安装 Qt6（MSVC 工具链）。
2. CMake 配置与 Release 编译。
3. `windeployqt` 收集运行时。
4. 校验 `platforms/qwindows.dll` 等关键文件。
5. 打包 `VersionDownloadTool-win64.zip` 并上传到 Release 资产。

## 发布检查清单
- Action 成功。
- Release 附件存在 `VersionDownloadTool-win64.zip`。
- zip 内含 `platforms/qwindows.dll`。
