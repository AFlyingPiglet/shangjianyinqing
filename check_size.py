"""检查仓库体积：总大小、最大的文件、超限文件。上传 GitHub/Gitee 前跑一次。

用法：python check_size.py [目录]（默认当前目录）
"""

import os
import sys
from collections import defaultdict

SKIP_DIRS = {".git", "node_modules", "__pycache__"}
LIMIT_WARN = 20 * 1024 * 1024   # 20 MB 提醒
LIMIT_BLOCK = 100 * 1024 * 1024  # 100 MB 是 GitHub 的硬上限


def human(size):
    for unit in ("B", "KB", "MB", "GB"):
        if size < 1024 or unit == "GB":
            return "%.1f %s" % (size, unit) if unit != "B" else "%d B" % size
        size /= 1024.0


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else "."
    root = os.path.abspath(root)

    files = []
    by_ext = defaultdict(lambda: [0, 0])
    total = 0
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]
        for name in filenames:
            path = os.path.join(dirpath, name)
            try:
                size = os.path.getsize(path)
            except OSError:
                continue
            rel = os.path.relpath(path, root)
            files.append((size, rel))
            total += size
            ext = os.path.splitext(name)[1].lower() or "(无扩展名)"
            by_ext[ext][0] += 1
            by_ext[ext][1] += size

    print("目录：", root)
    print("文件数：%d　总大小：%s" % (len(files), human(total)))
    print()

    print("按类型统计：")
    for ext, (count, size) in sorted(by_ext.items(), key=lambda kv: -kv[1][1])[:10]:
        print("  %-12s %4d 个　%s" % (ext, count, human(size)))
    print()

    print("最大的 10 个文件：")
    for size, rel in sorted(files, reverse=True)[:10]:
        print("  %-10s %s" % (human(size), rel))
    print()

    big = [(s, r) for s, r in files if s >= LIMIT_WARN]
    if not big:
        print("✔ 没有超过 20 MB 的文件，可以直接上传")
    else:
        print("⚠ 以下文件偏大，建议不要提交（放进 .gitignore 或改用网盘链接）：")
        for size, rel in sorted(big, reverse=True):
            flag = "【超过 100 MB，GitHub 会直接拒绝】" if size >= LIMIT_BLOCK else ""
            print("  %-10s %s %s" % (human(size), rel, flag))

    print()
    if total > 1024 * 1024 * 1024:
        print("⚠ 总体积超过 1 GB，建议精简后再上传")
    else:
        print("✔ 总体积 %.1f MB，远小于平台上限，放心上传" % (total / 1024 / 1024))


if __name__ == "__main__":
    main()
